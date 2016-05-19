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
void restore(char *file, int client_fifo, int server_fifo); 
int get_server_pipe(char* fifo_path, int size);
void count_dead(int pid);
void write_succ_message();
void write_fail_message();

int alive;
char** current_file;
int ret;

int main(int argc, char* argv[]) {
	char cdir[PATH_SIZE];
	char server_fifo_path[PATH_SIZE], client_fifo_path[PATH_SIZE];
	int i, pp=0, server_fifo;
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

	if (!strcmp(argv[1], "backup") || !strcmp(argv[1], "restore")) {
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
		strncpy(cdir, "/tmp/sobu/", PATH_SIZE);
		mkdir(cdir, 0666);
		sprintf(cdir, "/tmp/sobu/%d", (int) uid);
		mkfifo(cdir, 0666);
		pp = open(cdir, O_RDONLY);
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
			
			if (!strcmp(argv[1], "backup")) backup(argv[i], server_fifo);
			else if (!strcmp(argv[1], "restore")) restore(argv[i], pp, server_fifo);

			_exit(0);
		}
	}	

	while (alive > 0) wait(NULL);

	close(pp);
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

	printf("pid: %d\n", pid);
	//pause(); 
	raise(SIGSTOP);
	printf("viste?\n");
	close(f);
	freeMessage(msg);
}

void restore(char *file, int client_fifo, int server_fifo) {
	MESSAGE msg = empty_message();
	int f;
	uid_t uid = getuid();
	pid_t pid = getpid();

	change_message(msg, "restore", uid, pid, file, "", 0, FINISHED);
	write(server_fifo, msg, sizeof(*msg));

	while(read(client_fifo, msg, sizeof(*msg))) {
		f = open(msg->file_path, O_CREAT | O_WRONLY | O_APPEND, 0644);
		write(f, msg->chunk, msg->chunk_size);

		close(f);
	}

	freeMessage(msg);
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

// decrementa o numero de filhos vivos
void count_dead(int pid) {
	waitpid(pid, NULL, WCONTINUED);
	alive--;	
}

// escreve a mensagem de sucesso enviada pelo utilizador
void write_succ_message() {
	printf("%s: copiado\n", *current_file);
	ret = 0;
}

// escreve a mensagem de erro enviada pelo utilizador
void write_fail_message() {
	printf("%s: ERRO - Impossível copiar\n", *current_file);
	ret = 1;
}
