#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <fcntl.h>
#include <pwd.h>

#define BACKUP 0
#define PATH_SIZE 1008
#define CHUNK_SIZE 3056 

#define NOT_FNSHD 1 //O ficheiro não está completo
#define FINISHED  0 //O ficheiro terminou
#define ERROR -1 //Houve um erro ao tentar ler do ficheiro

//sizeof(message) = 4kbytes
typedef struct message {
	char chunk[CHUNK_SIZE];
	char file_path[PATH_SIZE];
	int operation;
	int status;
	int chunk_size;
	pid_t pid;
} *MESSAGE;

/**
 * Cria uma mensagem a partir duma string
 * <operacao> <path> <nome do ficheiro> <status> <pid> <argument_size> <argument>
 * @param str string
 * @return mensagem
 */
MESSAGE toMessage(char* str);

/**
 * Cria uma nova message.
 * @param operation 
 * @param pid PID do processo da origem da mensagem
 * @param file_path Path real do ficheiro
 * @param chunk Pedaço de ficheiro a enviar
 * @param chunk_size Tamanho do pedaço
 * @param status NOT_FNSHD caso ainda haja mais chunks, FINISHED caso tenha terminado, ERROR caso a leitura do ficheiro deu um erro
 */
MESSAGE init_message(char* operation, pid_t pid, char* file_path, 
						char* chunk, int chunk_size, int status); 

#endif
