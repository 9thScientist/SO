#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "message.h"
#include "backup.h"

#define BUFFER_SIZE 512
#define MAX_CHILDREN 5

#define DATA_PATH "/.Backup/data/"
#define METADATA_PATH "/.Backup/metadata/"
#define PATHS_PATH "/.Backup/paths/"

void send_success(pid_t pid);
void send_error(pid_t pid);
void count_dead(int pid);
int create_root();
void check_in();

int alive; // Número de filhos atualmente vivas
int server_fifo;

int main(void) {
	MESSAGE msg; 
	char server_fifo_path[PATH_SIZE], bu_root[PATH_SIZE], new_file[PATH_SIZE], *home, *file_name;
	int err, f;

	signal(SIGCHLD, count_dead);

	create_root();
	check_in();
	home = getenv("HOME");
	strncpy(server_fifo_path, home, PATH_SIZE);
	strncat(server_fifo_path, "/.Backup/sobupipe", PATH_SIZE);

	if (server_fifo == -1) {
		perror("Erro ao ler pedidos");
		return -2;
	}
	
	strncpy(bu_root, home, PATH_SIZE);
	strncat(bu_root, DATA_PATH, PATH_SIZE);



	if (!fork()) { 
	server_fifo = open(server_fifo_path, O_RDONLY);
	while(1) {
		msg = empty_message();
		printf("a ler...\n");
		if (!read(server_fifo, msg, sizeof(*msg) )) {
			close(server_fifo);
			server_fifo = open(server_fifo_path, O_RDONLY);
			freeMessage(msg);
			continue;
		}
	printf("file: %s\nread: %d\n", msg->file_path, msg->chunk_size);
		file_name = get_file_name(msg->file_path);
		strncpy(new_file, bu_root, PATH_SIZE);
		strncat(new_file, file_name, PATH_SIZE);
		f = open(new_file, O_WRONLY | O_APPEND | O_CREAT, 0600);
		write(f, msg->chunk, msg->chunk_size);
		close(f);

		if (msg->status == NOT_FNSHD) {
			freeMessage(msg);
			continue;
		} else if (msg->status == ERROR) {
			send_error(msg->pid);
			freeMessage(msg);
			exit(1);
		}

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
			
			printf("A enviar %d para %d\n", err, msg->pid);
			printf("SEND!\n");
			kill(msg->pid, SIGCONT);
			sleep(2);
			printf("THERE!\n");
			err ? send_error(msg->pid) : send_success(msg->pid);
			sleep(5);
			printf("afrit\n");
			free(msg);
			_exit(err);
		}
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
	
	if (mkdir(root_dir,0766) != -1) {
		strncpy(root_dir, home, PATH_SIZE);
		strncat(root_dir, DATA_PATH , PATH_SIZE);
		mkdir(root_dir,0744);
		
		strncpy(root_dir, home, PATH_SIZE);
		strncat(root_dir, METADATA_PATH , PATH_SIZE);
		mkdir(root_dir,0744);
	
		strncpy(root_dir, home, PATH_SIZE);
		strncat(root_dir, PATHS_PATH, PATH_SIZE);
		mkdir(root_dir,0744);
		
		strncpy(root_dir, home, PATH_SIZE);
		strncat(root_dir, "/.Backup/sobupipe", PATH_SIZE);
		mkfifo(root_dir, 0777);
		return 1;
	}

	return 0;
}

void check_in() {
	char fp[PATH_SIZE], uid_str[100];
	int f;
	uid_t uid = getuid();

	strncpy(fp, "/usr/share/sobuserv/running_user", PATH_SIZE);
	sprintf(uid_str, "%d", uid);

	f = open(fp, O_WRONLY);	
	write(f, uid_str, strlen(uid_str));
	close(f);
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

