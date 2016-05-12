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
	msg->pid = atoi(s);

	s = strtok(NULL, " ");
	msg->uid = atoi(s);

	s = strtok(NULL, " ");
	if (s)	msg->argument = s;

	return msg;
}
