#include "restore.h"
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define DATA_PATH "/.Backup/data/"
#define METADATA_PATH "/.Backup/metadata/"
#define PATHS_PATH "/.Backup/paths/"

void send_file(char* file_path, uid_t uid); 
char* get_original_path(char *file, char* original_path, int size); 
int copy(char* src, char* dest);
void decompress(char* file_path); 
void remove_gz(char* file_path); 

int restore(MESSAGE msg) {
	char *f_name, aux_path[PATH_SIZE], file_path[PATH_SIZE];

	f_name = get_file_name(msg->file_path);
	
	strncpy(file_path, getenv("HOME"), PATH_SIZE);
	strncat(file_path, METADATA_PATH, PATH_SIZE);
	strncat(file_path, f_name, PATH_SIZE);

	readlink(file_path, aux_path, PATH_SIZE);	

	strncpy(file_path, aux_path, PATH_SIZE);
	strncat(file_path, ".gz", PATH_SIZE);

	copy(aux_path, file_path);
	decompress(file_path);
	send_file(file_path, msg->uid);

	return 0;
}

/**
 * Envia o ficheiro para o user dado
 * @param file_path ficheiro a enviar
 * @param uid User de destino
 */
void send_file(char* file_path, uid_t uid) {
	char client_pipe_path[PATH_SIZE], original_path[PATH_SIZE], chunk[CHUNK_SIZE];
	char* f_name;
	int client_pipe, file, sz;
	MESSAGE msg = empty_message();

	sprintf(client_pipe_path, "/tmp/sobu/%d", (int) uid);
	
	client_pipe = open(client_pipe_path, O_WRONLY);
	
	file = open(file_path, O_RDONLY);
	f_name = get_file_name(file_path); 
	get_original_path(f_name, original_path, PATH_SIZE);

	while((sz = read(file, chunk, CHUNK_SIZE)))	{
		change_message(msg, "restore", uid, 0, original_path, chunk, sz, NOT_FNSHD);
		write(client_pipe, msg, sizeof(*msg));
	}
	
	freeMessage(msg);
	close(file);
	close(client_pipe);
}

/**
 * Devolve o path original do ficheiro dado
 * @param file 
 * @param original_path String de retorno
 * @param size Tamanho da original_path
 * @return o path original
 */
char* get_original_path(char *file, char* original_path, int size) {
	char path[PATH_SIZE];

	strncpy(path, getenv("HOME"), PATH_SIZE);
	strncat(path, PATHS_PATH, PATH_SIZE);	
	strncat(path, file, PATH_SIZE);	

	readlink(path, original_path, size);

	return original_path;
}

int copy(char* src, char* dest) {
	int st;

	if (!fork()) {
		execlp("cp", "cp", src, dest, NULL);
		perror("Erro ao tentar buscar ficheiro");
		_exit(1);
	}

	wait(&st);
	return WIFEXITED(st);
}

void decompress(char* file_path) {
	if (!fork()) {
		execlp("gzip", "gzip", "-d", file_path, NULL);
		perror("Erro ao tentar decomprimir ficheiro");
		_exit(1);
	}

	wait(NULL);
	remove_gz(file_path);	
}

void remove_gz(char* file_path) {
	int c = strlen(file_path) - 3;
	file_path[c] = 0;
}
