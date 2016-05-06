#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <stdio.h>

#define SERVER_FIFO_PATH "sobuserver_fifo"
#define BUFFER_SIZE 512;

int main(void) {
	char buffer[BUFFER_SIZE], *command, *file;
	int server_fifo;
	pid_t pid;

	if (mkfifo(SERVER_FIFO_PATH, 0600) == -1) {
		fprintf(stderr, "Could not set up request channel");
		return -1
	}

	server_fifo = open(SERVER_FIFO_PATH, O_RDONLY);
	
	if (requests_fifo == -1) {
		fprintf(stderr, "Failed to read requests\n");
		return -2;
	}

	while(read(server_fifo, buffer, BUFFER_SIZE)) {
		command = strtok(buffer, " ");

		if (!strcmp(command, "backup")){
			file = strtok(NULL, " ");
			pid = atoi(strtok(NULL, " "));
			
			backup(file, pid);
		}
	}

	return 0;
}
