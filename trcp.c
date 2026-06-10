#include "src/arglib.h"
#include "src/db.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/errno.h>
#include <regex.h>

#include "src/config.h"

#ifndef VERSION
#define VERSION "No version"
#endif

static int PORT = DEFAULT_PORT;
char *id = NULL;
static int is_quiet = 0;

#define error(...) if(!is_quiet) fprintf(stderr, __VA_ARGS__)

#undef printf
#define printf(...) if(!is_quiet) fprintf(stdout, __VA_ARGS__)

void *handle_client_conn(void *arg);
char *url_decode(const char *src);

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

	printf ("trcp-%s started on port %d\n", VERSION, PORT);

	// Print the censored channel ID (only the first and last char)
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

// Generates a HTTP header
void generate_response(int client_fd, int status, char* status_msg,
		char *content){
	char *header = (char *)malloc(BUFFER_SIZE * sizeof(char));

	// Generate header
	size_t response_len =
		snprintf(header, BUFFER_SIZE,
			"HTTP/1.1 %d %s\r\n"
			"Content-Type: text/plain\r\n"
			"\r\n"
			"%s\r\n",
			status, status_msg, content);

	// Send the final result
	send(client_fd, header, response_len, 0);

	free(header);
	return;
}

void *handle_client_conn(void *arg){
	int client_fd = *((int *)arg);
	char *buffer = (char *)malloc(BUFFER_SIZE * sizeof(char));
	ssize_t rec = recv(client_fd, buffer, BUFFER_SIZE, 0);

	// No data given
	if (rec == 0){
		generate_response(client_fd,
			202, "OK", "Invalid usage");
	
	// Determine method
	} else {
		regex_t regex;
		regcomp(&regex, "^GET /([^ ]*) HTTP/1", REG_EXTENDED);
		regmatch_t matches[2];
		if (regexec(&regex, buffer, 2, matches, 0) == 0){
			buffer[matches[1].rm_eo] = '\0';
			const char *url_encoded_file_name = buffer + matches[1].rm_so;
			char *url_path  = url_decode(url_encoded_file_name);

			// Offload to the DB
			db_manage(url_path);
			generate_response(client_fd,
				200, "OK", url_path);

			free(url_path);
		}
	}

	// Close the connection
	close(client_fd);
	free (arg);
	free(buffer);
	return NULL;
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
