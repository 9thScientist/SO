#include "message.h"
#include <string.h>
#include <stdlib.h>

MESSAGE init_message(char* operation, pid_t pid, char* file_path, 
						char* chunk, int chunk_size, int status) {
	MESSAGE m = malloc(sizeof(*m));

	m->operation = strcmp(operation, "backup");
	m->status = status;
	m->chunk_size = chunk_size;
	m->pid = pid;
	strncpy(m->file_path, file_path, PATH_SIZE);
	strncpy(m->chunk, chunk, CHUNK_SIZE);

	return m;
}

MESSAGE toMessage(char* str) {
	MESSAGE msg = malloc(sizeof(*msg));
	char* s;

	s = strtok(str, " ");
	if (!strcmp(s, "backup"))
		msg->operation = BACKUP;
	
	s = strtok(NULL, " ");
	strncpy(msg->file_path, s, PATH_SIZE);

	s = strtok(NULL, " ");
	msg->status = atoi(s);

	s = strtok(NULL, " ");
	msg->pid = atoi(s);

	s = strtok(NULL, " ");
	msg->chunk_size = atoi(s);

	s = strtok(NULL, " ");
	if (s) 
		strncpy(msg->chunk, s, CHUNK_SIZE);

	return msg;
}

void freeMessage(MESSAGE m) {
	free(m);	
}
