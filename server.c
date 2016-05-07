#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pwd.h>

#define SERVER_FIFO_PATH "sobuserver_fifo"
#define BUFFER_SIZE 512
#define PATH_SIZE 64
#define MAX_CHILDREN 5
#define BACKUP 0

typedef struct message {
	char* argument;
	int operation;
	pid_t pid;
	uid_t uid;
} *MESSAGE;

MESSAGE toMessage(char* str);
void backup(MESSAGE msg);
void count_dead(int pid);

int alive;

int main(void) {
	MESSAGE msg;
	char buffer[BUFFER_SIZE]; 
	int server_fifo;

	signal(SIGCHLD, count_dead);

	if (access(SERVER_FIFO_PATH, F_OK) == -1 && mkfifo(SERVER_FIFO_PATH, 0600) == -1) {
			perror("Erro ao tentar criar canal de pedidos.");
			return -1;
	}

	server_fifo = open(SERVER_FIFO_PATH, O_RDONLY);
	
	if (server_fifo == -1) {
		perror("Erro ao ler pedidos.");
		return -2;
	}
	
	while(read(server_fifo, buffer, BUFFER_SIZE)) {
		msg = toMessage(buffer);
		
		if (alive == MAX_CHILDREN)
			pause();
	
		alive++;	
		if (!fork()) {
			switch(msg->operation) {
				case BACKUP: backup(msg);
							 break;
			}
			_exit(0);
		}


	}

	close(server_fifo); //TODO remover server_fifo
	return 0;
}

void backup(MESSAGE msg) {
	int status;
	char root_dir[PATH_SIZE];
	struct passwd *pw = getpwuid(msg->uid);
	struct stat st;

	sprintf(root_dir, "%s/.Backup", pw->pw_dir);	
	if (stat(root_dir, &st) == -1) {
		mkdir(root_dir,0700);
		sprintf(root_dir, "%s/.Backup/data", pw->pw_dir);	
		mkdir(root_dir,0700);
		sprintf(root_dir, "%s/.Backup/metadata", pw->pw_dir);	
		mkdir(root_dir,0700);
	} 

	//TODO copiar o conteúdo para /data e comprimir aí

	if (!fork()) {
		// gzip -c ficheiro > ficheiro.gz
		execlp("gzip", "gzip", msg->argument, NULL);

		perror("Erro ao tentar comprimir ficheiro.\n\
			   	Talvez o 'gzip' não esteja corretamente instalado no sistema.");
		_exit(-1);
	}

	wait(&status);
	status = WEXITSTATUS(status);
	if (status == 0) 
		kill(msg->pid, SIGUSR1); //envia sinal de sucesso
	else
	   kill(msg->pid, SIGUSR2);	 //envia sinal de erro
}

MESSAGE toMessage(char* str) {
	MESSAGE msg = malloc(sizeof(*msg));
	char* s;

	s = strtok(str, " ");
	if (!strcmp(s, "backup"))
		msg->operation = BACKUP;

	s = strtok(NULL, " ");
	msg->pid = atoi(s);

	s = strtok(NULL, " ");
	msg->uid = atoi(s);

	s = strtok(NULL, " ");
	if (s)
		msg->argument = s;

	return msg;
}

void count_dead(int pid) {
	waitpid(pid, NULL, WCONTINUED);
	alive--;	
} 
