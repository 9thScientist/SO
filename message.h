#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <fcntl.h>
#include <pwd.h>

#define BACKUP 0
#define PATH_SIZE 128
#define DATA_SIZE 4096 

typedef struct message {
	char real_path[PATH_SIZE];
	char argument[DATA_SIZE];
	int operation;
	pid_t pid;
} *MESSAGE;

/**
 * Cria uma mensagem a partir duma string
 * @param str string
 * @return mensagem
 */
MESSAGE toMessage(char* str);

#endif
