#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <fcntl.h>
#include <pwd.h>

#define BACKUP 0
#define PATH_SIZE 768
#define NAME_SIZE 248
#define DATA_SIZE 3056 

#define NOT_FNSHD 1 //O ficheiro não está completo
#define FINISHED  0 //O ficheiro terminou
#define CORRUPT  -1 //Houve um erro ao tentar ler do ficheiro

//sizeof(message) = 4kbytes
typedef struct message {
	char argument[DATA_SIZE];
	char real_path[PATH_SIZE];
	char file_name[NAME_SIZE];
	int operation;
	int status;
	pid_t pid;
} *MESSAGE;

/**
 * Cria uma mensagem a partir duma string
 * @param str string
 * @return mensagem
 */
MESSAGE toMessage(char* str);

#endif
