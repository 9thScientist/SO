#include "message.h"
#include <string.h>
#include <stdlib.h>

MESSAGE init_message(char* operation, uid_t uid, pid_t pid, char* file_path, 
						char* chunk, int chunk_size, int status) {
	MESSAGE m = malloc(sizeof(*m));

	change_message(m, operation, uid, pid, file_path, chunk, chunk_size, status);
	return m;
}

MESSAGE empty_message() {
	MESSAGE m = malloc(sizeof(*m));

	return m;
}

void change_message(MESSAGE m, char* operation, uid_t uid, pid_t pid, char* file_path, 
						char* chunk, int chunk_size, int status) {

	if (!strcmp(operation, "backup")) 
		m->operation = BACKUP;
	else if (!strcmp(operation, "restore")) 
		m->operation = RESTORE;
	else if (!strcmp(operation, "delete"))
		m->operation = DELETE;
	else if (!strcmp(operation, "gc"))
		m->operation = CLEAN;

	m->status = status;
	m->chunk_size = chunk_size;
	m->pid = pid;
	m->uid = uid;
	strncpy(m->file_path, file_path, PATH_SIZE);
	memcpy(m->chunk, chunk, m->chunk_size);
}

void freeMessage(MESSAGE m) {
	free(m);	
}

char* get_file_name(char *file_path) {

	char* base = strrchr(file_path,'/');
	return base ? base+1 : file_path;
}
