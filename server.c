#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "message.h"
#include "backup.h"

#define SERVER_FIFO_PATH "/tmp/sobuserver_fifo"
#define BUFFER_SIZE 512
#define PATH_SIZE 128
#define MAX_CHILDREN 5

int save_data(char* home_dir, char *file, char* hash);
int compress_file(char* file_path);
char* generate_hash(char *file_path);
void send_success(pid_t pid);
void send_error(pid_t pid);
void count_dead(int pid);
int create_root();
void afrit();

int alive; // Número de filhos atualmente vivas
int server_fifo;

int main(void) {
	MESSAGE msg;
	char buffer[BUFFER_SIZE];
	int err;

	signal(SIGCHLD, count_dead);
	signal(SIGINT, afrit);
	signal(SIGQUIT, afrit);

	create_root();

	if (access(SERVER_FIFO_PATH, F_OK) == -1 && mkfifo(SERVER_FIFO_PATH, 0600) == -1) {
		perror("Erro ao tentar criar canal de pedidos.");
		return -1;
	}

	server_fifo = open(SERVER_FIFO_PATH, O_RDONLY);

	if (server_fifo == -1) {
		perror("Erro ao ler pedidos.");
		return -2;
	}

	while(1) {
		if (!read(server_fifo, buffer, BUFFER_SIZE)) continue;

		msg = toMessage(buffer);

		if (alive == MAX_CHILDREN)
			pause();

		alive++;
		if (!fork()) {
			switch(msg->operation) {
				case BACKUP: err = backup(msg);
							 break;
				default: err = 1;
						 break;

			}
			
			err ? send_error(msg->pid) : send_success(msg->pid);
			_exit(0);
		}

	}

	return 0;
}

/**
 * Verifica se existe a raiz do backup. Caso não exista, cria-a.
 * @return 1 caso crie, 0 caso contrário
 */
int create_root() {
	char root_dir[PATH_SIZE], *home;

	home = getenv("HOME");

	strncpy(root_dir, home, PATH_SIZE);
	strncat(root_dir, "/.Backup", PATH_SIZE);

	if (mkdir(root_dir,0700) != -1) {
		strncpy(root_dir, home, PATH_SIZE);
		strncat(root_dir, "/.Backup/data", PATH_SIZE);
		mkdir(root_dir,0700);
		strncpy(root_dir, home, PATH_SIZE);
		strncat(root_dir, "/.Backup/metadata", PATH_SIZE);
		mkdir(root_dir,0700);
		return 1;
	}

	return 0;
}

void send_success(pid_t pid) {
	kill(pid, SIGUSR1); //envia sinal de sucesso
}

void send_error(pid_t pid) {
	kill(pid, SIGUSR2);	 //envia sinal de erro
}

void count_dead(int pid) {
	waitpid(pid, NULL, WCONTINUED);
	alive--;
}

void afrit() {
	close(server_fifo);
	unlink(SERVER_FIFO_PATH);
	write(1, "\nVaarwel.\n", 10);
	exit(0);
}
