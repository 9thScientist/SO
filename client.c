#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <stdio.h>

#define BUFFER_SIZE 512
#define SERVER_FIFO_PATH "sobuserver_fifo"

int main(int argc, char* argv[]) {
	char message[BUFFER_SIZE];
	int i, num_messages, server_fifo;
	pid_t pid;

	// Verifica se os argumentos são válidos
	if (argc == 1) {
		fprintf(stderr, "No command given\n");
		return -1;
	}

	if (strcmp(argv[1], "backup") && strcmp(argv[1], "restore")){
		fprintf(stderr, "'%s' is not a valid command\n", argv[1]);
		return -1;
	}

	if (argc == 2) {
		fprintf(stderr, "No files specified\n");
		return -1;
	}

	if (!strcmp(argv[1], "backup")) {
		for(i = 2; i < argc; i++) {
			if (access(argv[i], F_OK) == -1) {
				fprintf(stderr, "File %s does not exist\n", argv[i]);
				return -2;
			}
 
			if (access(argv[i], R_OK) == -1) {
				fprintf(stderr, "File %s is not readable\n", argv[i]);
				return -2;
			}
		}
	}	

	// Prepara e envia informação a partir dos argumentos
	server_fifo = open(SERVER_FIFO_PATH, O_WRONLY);

	if (server_fifo == -1){
		perror("Could not send request to server");
		return -3;
	}

	pid = getpid();
	num_messages = argc-2;
	
	for(i = 0; i < num_messages; i++) {
		sscanf(message, "%s %s %d", argv[1], argv[i], &pid);
		write(server_fifo, message, strlen(message)+1);
	}	
	
	return 0;
}
