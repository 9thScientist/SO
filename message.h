#ifndef __MESSAGE_H__
#define __MESSAGE_H__
#define BACKUP 0

#include <fcntl.h>
#include <pwd.h>

typedef struct message {
	char* argument;
	char* current_dir;
	int operation;
	pid_t pid;
	uid_t uid;
} *MESSAGE;

/**
 * Cria uma mensagem a partir duma string
 * @param str string
 * @return mensagem
 */
MESSAGE toMessage(char* str);

#endif
