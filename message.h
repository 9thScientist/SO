#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <fcntl.h>
#include <pwd.h>

#define BACKUP 0
#define RESTORE 1
#define DELETE 2
#define CLEAN 3

#define PATH_SIZE 1024
#define CHUNK_SIZE 4096 

#define NOT_FNSHD 1 //O ficheiro não está completo
#define FINISHED  0 //O ficheiro terminou
#define ERROR -1 //Houve um erro ao tentar ler do ficheiro

typedef struct message {
	char chunk[CHUNK_SIZE];
	char file_path[PATH_SIZE];
	int operation;
	int status;
	int chunk_size;
	pid_t pid;
	uid_t uid;
} *MESSAGE;

/**
 * Cria uma nova message.
 * @param operation 
 * @param uid User ID
 * @param pid PID do processo da origem da mensagem
 * @param file_path Path real do ficheiro
 * @param chunk Pedaço de ficheiro a enviar
 * @param chunk_size Tamanho do pedaço
 * @param status NOT_FNSHD caso ainda haja mais chunks, FINISHED caso tenha terminado, ERROR caso a leitura do ficheiro deu um erro
 */
MESSAGE init_message(char* operation, uid_t uid, pid_t pid, char* file_path, 
						char* chunk, int chunk_size, int status); 

/**
 * Altera o valor da mensagem dada
 * @param m Mensagem a alterar
 * @param operation 
 * @param uid User ID
 * @param pid PID do processo da origem da mensagem
 * @param file_path Path real do ficheiro
 * @param chunk Pedaço de ficheiro a enviar
 * @param chunk_size Tamanho do pedaço
 * @param status NOT_FNSHD caso ainda haja mais chunks, FINISHED caso tenha terminado, ERROR caso a leitura do ficheiro deu um erro
 */
void change_message(MESSAGE m, char* operation, uid_t uid, pid_t pid, char* file_path, 
						char* chunk, int chunk_size, int status);
/**
 * Cria uma mensagem vazia
 */
MESSAGE empty_message();
 
char* get_file_name(char *file_path);

void freeMessage(MESSAGE m);

#endif
