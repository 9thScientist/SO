#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVER_FIFO_PATH "sobuserver_fifo"
#define BUFFER_SIZE 512
#define PATH_SIZE 64
#define MAX_CHILDREN 5
#define BACKUP 0

typedef struct message {
	char* argument;
	int operation;
	pid_t pid;
	uid_t uid;
} *MESSAGE;

MESSAGE toMessage(char* str);
void backup(MESSAGE msg);

int main(void) {
	MESSAGE msg;
	char buffer[BUFFER_SIZE]; 
	int server_fifo, alive;

	if (access(SERVER_FIFO_PATH, F_OK) == -1 && mkfifo(SERVER_FIFO_PATH, 0600) == -1) {
			fprintf(stderr, "Could not set up request channel");
			return -1;
	}

	server_fifo = open(SERVER_FIFO_PATH, O_RDONLY);
	
	if (server_fifo == -1) {
		fprintf(stderr, "Failed to read requests\n");
		return -2;
	}
	
	alive = 0;
	while(read(server_fifo, buffer, BUFFER_SIZE)) {
		msg = toMessage(buffer);
		
		if (alive == MAX_CHILDREN)
			pause();
		
		if (!fork()) {
			switch(msg->operation) {
				case BACKUP: backup(msg);
							 break;
			}
		}

	}

	return 0;
}

void backup(MESSAGE msg) {
	char pipe[PATH_SIZE], response[BUFFER_SIZE];
	int response_pipe;

	sprintf(pipe, "/tmp/%d", msg->pid);
	response_pipe = open(pipe, O_WRONLY);

	sprintf(response, "%s: copiado\n", msg->argument);
	write(response_pipe, response, strlen(response)+1);

	close(response_pipe);
}

MESSAGE toMessage(char* str) {
	MESSAGE msg = malloc(sizeof(*msg));
	char* s;

	s = strtok(str, " ");
	if (!strcmp(s, "backup"))
		msg->operation = BACKUP;

	s = strtok(NULL, " ");
	msg->pid = atoi(s);

	s = strtok(NULL, " ");
	msg->uid = atoi(s);

	s = strtok(NULL, " ");
	if (s)
		msg->argument = s;

	return msg;
}
