#include "src/arglib.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/errno.h>
#include <sys/epoll.h>
#include <regex.h>

#ifndef VERSION
#define VERSION "No version"
#endif

#include "src/config.h"

static int PORT = DEFAULT_PORT;
char *id = NULL;
static int is_quiet = 0;

#define error(...) if(!is_quiet) fprintf(stderr, __VA_ARGS__)
#undef printf
#define printf(...) if(!is_quiet) fprintf(stdout, __VA_ARGS__)

// Very useful snprintf macro
#define snprintfm(dest, size_var, ...)\
	size_var = snprintf(NULL, 0, __VA_ARGS__)+1; \
	dest = (char*)malloc(size_var * sizeof(char)); \
	snprintf(dest, size_var, __VA_ARGS__)

/* function declarations */
void *handle_client_conn(void *arg);
char *url_decode(const char *src);
void generate_header(int client_fd, int status, char* status_msg, char **out, size_t *out_len);
void generate_stream(int client_fd, char **out, size_t *out_len);
void handle_url_path(int client_fd, char *endpoint);
int stream_send(int client_fd, char *content);
void backlog_append(char *content);

/* backlog */
static char *backlog[BACKLOG_SIZE];
int backlog_idx;
int total_messages;

// Argument processor function
int argparse(int argc, char *argv[]){
	FORARGS {
		ARG ("--help", "Show this basic help menu"){
			HELP(argparse);
			return 1;
		}

		ARG ("--port", "Set the port in use"){
			if((PORT = atoi(ARGVAL)) == 0 ){
				error("port: %s is invalid.\n", ARGVAL);
				return 1;
			}

			if (PORT < PORT_MIN){
				error("port: %d is too small, must be higher than %d\n",
						PORT, PORT_MIN);
				return 1;
			}

			if (PORT > PORT_MAX){
				error("port: %d is too large, must be less than %d\n",
						PORT, PORT_MAX);
				return 1;
			}
		}

		POSANY(ARGLAST, "<id>", "Must be the last argument, this is a unique ID for validating incoming requests"){
			id = strdup(ARGVAL);
			ARGNEXT;
		}
	}

	return 0;
}

int main(int argc, char *argv[]){
	if (argparse(argc, argv) != 0)
		return 1;

	// Print server information
	printf ("trcp-%s started on port %d\n", VERSION, PORT);
	if (id != NULL){
		fputs("GId: ", stdout);
		putchar(id[0]);
		for(int i = 0; i < strlen(id) - 2; i++)
			putchar('*');
		putchar(id[strlen(id)-1]);
		putchar('\n');
	}

	fflush(stdout);

	int server_fd;
	struct sockaddr_in addr;

	// Create the server socket
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		error("Socket faliure\n");
		return EXIT_FAILURE;
	}

	// Socket config
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons (PORT);

	// Bind the server to confirm validity
	if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0){
		switch (errno){
			case EADDRINUSE:
				error("bind failed, port is already in use\n");
				break;

			default:
				error("bind failed with code: %d\n", EXIT_FAILURE);
				break;
		}

		return EXIT_FAILURE;
	}

	// Listen for connections
	if (listen(server_fd, 10) < 0){
		error("Listen failed\n");
		return EXIT_FAILURE;
	}

	// Move onto the connection loop
	for (;;){
		struct sockaddr_in client_addr;
		socklen_t client_addr_len = sizeof(client_addr);
		int *client_fd = malloc(sizeof(int));

		// Accept the connection
		if((*client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len)) < 0){
			error("Failed to accept\n");
			continue;
		}

		char client_ip[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
		printf("Connection from: %s\n", client_ip);

		// Create a new thread for each client
		pthread_t thread_id;
		pthread_create(&thread_id, NULL, handle_client_conn, (void *)client_fd);
		pthread_detach(thread_id);
	}

	return 0;
}

// Generates a return header
void generate_header(int client_fd, int status, char* status_msg, char **out, size_t *out_len){
	free(*out);
	snprintfm(*out, *out_len,
		"HTTP/1.1 %d %s\r\n"
		"Content-Type: text/plain\r\n"
		"\r\n",
		status, status_msg);
	return;
}

// Generates the stream header
void generate_stream(int client_fd, char **out, size_t *out_len){
	free(*out);
	snprintfm(*out, *out_len,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/event-stream\r\n"
		"Cache-Control: no-cache\r\n"
		"Connection: keep-alive\r\n"
		"\r\n");
	return;
}

// Sends text content to the stream
// This will work with any connection through
// If this returns 1 then the client is no longer connected
int stream_send(int client_fd, char *content){
	const int content_size = strlen(content);
	if (send(client_fd, content, content_size, MSG_NOSIGNAL) <= 0)
		return 1;
	return 0;
}

// Client thread function, run for every client
void *handle_client_conn(void *arg){
	int client_fd = *((int *)arg);
	char *buffer = (char *)malloc(BUFFER_SIZE * sizeof(char));
	ssize_t rec = recv(client_fd, buffer, BUFFER_SIZE, 0);
	char *header = NULL;
	size_t header_size;

	// No data given, this should not be achievable
	if (rec == 0){
		generate_header(client_fd, 204, "No Content", &header, &header_size);
		send (client_fd, header, header_size, 0);
		stream_send(client_fd, "No Content");
	
	// Filter the request and get the endpoint requested
	} else {
		regex_t regex;
		regcomp(&regex, "^GET /([^ ]*) HTTP/1", REG_EXTENDED);
		regmatch_t matches[2];
		if (regexec(&regex, buffer, 2, matches, 0) == 0){
			buffer[matches[1].rm_eo] = '\0';

			// Offload the endpoint to the manager
			const char *url_encoded_path= buffer + matches[1].rm_so;
			char *url_path  = url_decode(url_encoded_path);
			handle_url_path(client_fd, url_path);
			free(url_path);
		}
	}

	// Close the connection
	free (arg);
	free(buffer);
	free(header);
	close(client_fd);

	printf("Thread exiting\n");
	return NULL;
}

// This function contains all endpoint management
void handle_url_path(int client_fd, char* endpoint){
	char *header = NULL;
	size_t header_size;

	// iterate through endpoints
	char *method   = strtok(endpoint, "/");
	char *given_id = strtok(NULL, "/");
	char *message  = strtok(NULL, "/");

	// No method OR id OR message was given.
	if (method == NULL || given_id == NULL || (!strcmp(method, "post") && message == NULL)){
		generate_header(client_fd, 400, "Bad Request", &header, &header_size);
		send(client_fd, header, header_size, 0);
		goto exit;
	}

	// ID is invalid
	if (strcmp(given_id, id)){
		generate_header(client_fd, 401, "Unauthorized", &header, &header_size);
		send(client_fd, header, header_size, 0);
		goto exit;
	}

	// Start stream
	if (strcmp(method, "sock") == 0){
		generate_stream(client_fd, &header, &header_size);
		send (client_fd, header, header_size, 0);

		// Hold the user while they are connected
		while(1){

			// TODO implement ticket behavior here
			if (stream_send(client_fd, "messages in backlog:\n"))
				break;
			for (int i = 0; i > backlog_idx; ++i)
				if (stream_send(client_fd, backlog[i]))
					break;

			sleep(1);
		}

	// Get the given message
	} else {
		generate_header(client_fd, 200, "OK", &header, &header_size);
		send(client_fd, header, header_size, 0);

		// Append to backlog
		stream_send(client_fd, "Message sent");
		backlog_append(message);
	}

exit:
	// Close connection
	free(header);
	return;
}

// Stolen..
char *url_decode(const char *src) {
	size_t src_len = strlen(src);
	char *decoded = malloc(src_len + 1);
	size_t decoded_len = 0;
	
	// decode %2x to hex
	for (size_t i = 0; i < src_len; i++) {
		if (src[i] == '%' && i + 2 < src_len) {
			int hex_val;
			sscanf(src + i + 1, "%2x", &hex_val);
			decoded[decoded_len++] = hex_val;
			i += 2;
		} else {
			decoded[decoded_len++] = src[i];
		}
	}
	
	// add null terminator
	decoded[decoded_len] = '\0';
	return decoded;
}


void backlog_append(char *content){
	if (content == NULL)
		return;

	// Backlog is full so we shuffle
	if (backlog_idx == BACKLOG_SIZE){
		free(backlog[0]);
		for (int i = 0; i < BACKLOG_SIZE-1; ++i){
			backlog[i] = (char *)malloc(strlen(backlog[i+1])*sizeof(char));
			strcpy(backlog[i], backlog[i+1]);
			free(backlog[i+1]);
			backlog[i+1] = NULL;
		}
	}
	
	backlog[backlog_idx] = (char *)malloc(strlen(content)*sizeof(char));
	strcpy(backlog[backlog_idx], content);
	backlog_idx++;
	return;
}
