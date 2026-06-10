#include "arglib.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/errno.h>

#define PORT_MAX 65535
#define PORT_MIN 1024
#define BUFFER_SIZE 512
static int PORT = 5050;
char *id = NULL;
int is_quiet = 0;

#define error(...) if(!is_quiet) fprintf(stderr, __VA_ARGS__)

#undef printf
#define printf(...) if(!is_quiet) fprintf(stdout, __VA_ARGS__)

void *handle_client_conn(void *arg);

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


		POSANY(ARGLAST, "<id>", "Must be the last argument"){
			id = strdup(ARGVAL);
			ARGNEXT;
		}
	}

	return 0;
}

int main(int argc, char *argv[]){
	if (argparse(argc, argv) != 0)
		return 1;

	printf ("port: %d\n", PORT);

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
		switch (EXIT_FAILURE){
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

		pthread_t thread_id;
		pthread_create(&thread_id, NULL, handle_client_conn, (void *)client_fd);
		pthread_detach(thread_id);
	}

	return 0;
}

void *handle_client_conn(void *arg){
	int client_fd = *((int *)arg);
	char *buffer = (char *)malloc(BUFFER_SIZE * sizeof(char));
	ssize_t rec = recv(client_fd, buffer, BUFFER_SIZE, 0);
	

	if (rec > 0){
		// data was given.
	}

	char *resp = "Hello World!";
	send (client_fd, resp, strlen(resp), 0);

	printf ("%s\n", buffer);


	close(client_fd);
	free (arg);
	free(buffer);
	return NULL;
}
