#include "message.h"
#include <string.h>
#include <stdlib.h>

MESSAGE toMessage(char* str) {
	MESSAGE msg = malloc(sizeof(*msg));
	char* s;

	s = strtok(str, " ");
	if (!strcmp(s, "backup"))
		msg->operation = BACKUP;
	
	s = strtok(NULL, " ");
	strncpy(msg->real_path, s, PATH_SIZE);

	s = strtok(NULL, " ");
	msg->pid = atoi(s);

	s = strtok(NULL, " ");
	if (s) 
		strncpy(msg->argument, s, DATA_SIZE);

	return msg;
}

void freeMessage(MESSAGE m) {
	free(m);	
}
