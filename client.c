#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "message.h"

#define BUFFER_SIZE 512
#define MAX_CHILDREN 5 

void backup(char *file, int server_fifo); 
void restore(char *file,char* client_fifo_path, int server_fifo); 
int get_server_pipe(char* fifo_path, int size);
int get_server_root(char* server_root, int size); 
void count_dead(int pid);
void write_succ_message();
void write_fail_message();

int alive;
char* current_file;
int ret;

int main(int argc, char* argv[]) {
	char server_fifo_path[PATH_SIZE], client_fifo_path[PATH_SIZE];
	int i, server_fifo;
	uid_t uid = getuid();

	// Verifica se os argumentos são válidos
	if (argc == 1 || (strcmp(argv[1], "backup") && strcmp(argv[1], "restore"))) {
		fprintf(stderr, "Utilização: sobucli [MODO] ...[FICHEIROS]\n\
						 Tente 'sobucli --help' para mais ajuda.");
		return -1;
	}

	if (argc == 2) {
		fprintf(stderr, "No files specified\n");
		return -1;
	}

	if (!strcmp(argv[1], "backup")) {
		for(i = 2; i < argc; i++) {
			if (access(argv[i], F_OK) == -1) {
				fprintf(stderr, "Ficheiro '%s' não existe.\n", argv[i]);
				return -2;
			}
 
			if (access(argv[i], R_OK) == -1) {
				fprintf(stderr, "Sem Permissões. Impossível ler ficheiro '%s'.\n", argv[i]);
				return -2;
			}
		}
	}	

	if (!strcmp(argv[1], "restore")) {
		get_server_root(client_fifo_path, PATH_SIZE);
		sprintf(client_fifo_path, "%s%d", client_fifo_path, (int) uid);
		mkfifo(client_fifo_path, 0777);
	}


	signal(SIGCHLD, count_dead);

	// Prepara e envia informação a partir dos argumentos
	get_server_pipe(server_fifo_path, PATH_SIZE); 
	server_fifo = open(server_fifo_path, O_WRONLY);

	if (server_fifo == -1){
		perror("Erro ao tentar comunicar com servidor.");
		return -3;
	}

	for(i = 2; i < argc; i++) {
		if (alive == MAX_CHILDREN) 
			pause();

		alive++;
		if (!fork()) {		
		
			current_file = get_file_name(argv[i]);

			if (!strcmp(argv[1], "backup")) backup(argv[i], server_fifo);
			else if (!strcmp(argv[1], "restore")) restore(argv[i], client_fifo_path, server_fifo);

			_exit(0);
		}

		if (!strcmp(argv[1], "restore")) wait(NULL);
	}	

	while (alive > 0) wait(NULL);

	unlink(client_fifo_path);
	close(server_fifo);
	return ret;
}

void backup(char *file, int server_fifo) {
	MESSAGE msg;
	char cdir[PATH_SIZE], chunk[CHUNK_SIZE];
	int f, status;
	pid_t pid;
	uid_t uid = getuid();

	msg = empty_message();
	pid = getpid();
	realpath(file, cdir);

	signal(SIGUSR1, write_succ_message);	
	signal(SIGUSR2, write_fail_message);
	
	f = open(file, O_RDONLY);
	while( (status = read(f, chunk, CHUNK_SIZE)) > 0) {
		change_message(msg, "backup", uid, pid, cdir, chunk, status, NOT_FNSHD);
		write(server_fifo, msg, sizeof(*msg));
	}
					
	change_message(msg, "backup", uid, pid, cdir, "", 0, FINISHED);
	write(server_fifo, msg, sizeof(*msg));

	pause(); 
	
	if (!ret) {
		printf("%s: copiado\n", current_file);
	} else printf("%s: erro ao copiar\n", current_file);

	close(f);
	freeMessage(msg);
}

void restore(char *file,char* client_fifo_path, int server_fifo) {
	MESSAGE msg = empty_message();
	int f, st=1, client_fifo;
	uid_t uid = getuid();
	pid_t pid = getpid();
	
	change_message(msg, "restore", uid, pid, file, "", 0, FINISHED);
	write(server_fifo, msg, sizeof(*msg));

	client_fifo = open(client_fifo_path, O_RDONLY);

	printf("ler...\n");
	while(st) {
		if (!read(client_fifo, msg, sizeof(*msg))) continue;
		st = msg->status;
		f = open(msg->file_path, O_CREAT | O_WRONLY | O_APPEND, 0644);
		write(f, msg->chunk, msg->chunk_size);
		perror("file");

		close(f);
	}

	freeMessage(msg);
	close(client_fifo);
}

/**
 * Coloca o path para o pipe do servidor em fifo_path
 */
int get_server_pipe(char* fifo_path, int size) {
	char server_user[BUFFER_SIZE], info_path[PATH_SIZE];
	int file;
	uid_t uid;
	struct passwd *pw;

	strncpy(info_path, "/usr/share/sobuserv/running_user", PATH_SIZE);
	file = open(info_path, O_RDONLY);
	if (errno == ENOENT) return -1;

	read(file, server_user, BUFFER_SIZE);
	close(file);
	
	uid = (uid_t) atoi(server_user);
	pw = getpwuid(uid);
	strncpy(fifo_path, pw->pw_dir, size);
	strncat(fifo_path, "/.Backup/sobupipe", size);	

	return 0;
}

/**
 * Coloca o path para o pipe do servidor em fifo_path
 */
int get_server_root(char* server_root, int size) {
	char server_user[BUFFER_SIZE], info_path[PATH_SIZE];
	int file;
	uid_t uid;
	struct passwd *pw;

	strncpy(info_path, "/usr/share/sobuserv/running_user", PATH_SIZE);
	file = open(info_path, O_RDONLY);
	if (errno == ENOENT) return -1;

	read(file, server_user, BUFFER_SIZE);
	close(file);
	
	uid = (uid_t) atoi(server_user);
	pw = getpwuid(uid);
	strncpy(server_root, pw->pw_dir, size);
	strncat(server_root, "/.Backup/", size);	

	return 0;
}

// decrementa o numero de filhos vivos
void count_dead(int pid) {
	waitpid(pid, NULL, WCONTINUED);
	alive--;	
}

// escreve a mensagem de sucesso enviada pelo utilizador
void write_succ_message() {
	//printf("%s: copiado\n", *current_file);
	ret = 0;
}

// escreve a mensagem de erro enviada pelo utilizador
void write_fail_message() {
	//printf("%s: ERRO - Impossível copiar\n", *current_file);
	ret = 1;
}
