#include "message.h"
#include <string.h>
#include <stdlib.h>

MESSAGE init_message(char* operation, uid_t uid, pid_t pid, char* file_path, 
						char* chunk, int chunk_size, int status) {
	MESSAGE m = malloc(sizeof(*m));

	m->operation = strcmp(operation, "backup");
	m->status = status;
	m->chunk_size = chunk_size;
	m->pid = pid;
	m->uid = uid;
	strncpy(m->file_path, file_path, PATH_SIZE);
	memcpy(m->chunk, chunk, CHUNK_SIZE);

	return m;
}

MESSAGE empty_message() {
	MESSAGE m = malloc(sizeof(*m));

	return m;
}

void freeMessage(MESSAGE m) {
	free(m);	
}

char* get_file_name(char *file_path) {

	char* base = strrchr(file_path,'/');
	return base ? base+1 : file_path;
}
