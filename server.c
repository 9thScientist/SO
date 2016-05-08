#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pwd.h>
#include <errno.h>

#define SERVER_FIFO_PATH "sobuserver_fifo"
#define BUFFER_SIZE 512
#define PATH_SIZE 128 
#define MAX_CHILDREN 5
#define BACKUP 0

typedef struct message {
	char* argument;
	int operation;
	pid_t pid;
	uid_t uid;
} *MESSAGE;

int save_data(char* home_dir, char *file, char* hash); 
int compress_file(char* file_path);
char* generate_hash(char *file_path); 
void send_success(pid_t pid); 
void send_error(pid_t pid); 
MESSAGE toMessage(char* str);
void backup(MESSAGE msg);
void count_dead(int pid);
int create_root(char *home_dir);

int alive;

int main(void) {
	MESSAGE msg;
	struct passwd *pw;
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
	
	while(1) {
		if (!read(server_fifo, buffer, BUFFER_SIZE)) continue;
		
		msg = toMessage(buffer);
		
		if (alive == MAX_CHILDREN)
			pause();

		pw = getpwuid(msg->uid);
		create_root(pw->pw_dir);

		alive++;	
		if (!fork()) {
			switch(msg->operation) {
				case BACKUP: backup(msg);
							 break;
			}
			_exit(0);
		}

	}

	return 0;
}

void backup(MESSAGE msg) {
	int err;
	char root_dir[PATH_SIZE], ln_dir[PATH_SIZE], *hash;
	struct passwd *pw = getpwuid(msg->uid);
	
	//Gerar uma hash a partir do ficheiro
	hash = generate_hash(msg->argument);
	if (!hash) {
		send_error(msg->pid);
		return;
	}
	//Comprime o ficheiro na raiz com o nome hash
	err = save_data(pw->pw_dir, msg->argument, hash);
	if (err) {
		free(hash);
		send_error(msg->pid);
		return;
	}

	//Cria symlink .Backup/data/digest -> .Backup/metadata/ficheiro
	sprintf(root_dir, "%s/.Backup/data/%s", pw->pw_dir, hash);
	sprintf(ln_dir, "%s/.Backup/metadata/%s", pw->pw_dir, msg->argument);
	symlink(root_dir, ln_dir);

	free(hash);

	send_success(msg->pid);
}

/**
 * Comprime o ficheiro file e guarda-o na ~/.Backup/data com o nome da hash
 * @param home_dir home do utilizador
 * @param file Nome do ficheiro a guardar
 * @param hash Hash indicativa do ficheiro
 * @erturn Código de erro
 */
int save_data(char* home_dir, char *file, char* hash) {
	char data_path[PATH_SIZE];
	int status, nf;
	
	sprintf(data_path, "%s/.Backup/data/%s", home_dir, hash);
	
	nf = open(data_path, O_CREAT | O_EXCL, 0600);
	if (errno == EEXIST) return 0;
	close(nf);
	
	if (!fork()) {
		execlp("cp", "cp", file, data_path, NULL);
		perror("Erro ao tentar salvar ficheiro.");
		_exit(1);
	}
	
	wait(&status);
	status = WEXITSTATUS(status);
	if (status == 0)  
		status = compress_file(data_path);

	if (status != 0) return -1;

	return 0;
}

/**
 * Comprime o ficheiro e remove-lhe a extensão .gz
 * @param file_path Ficheiro a comprimir
 * @return Código de erro.
 */
int compress_file(char* file_path) {
	char data_dir[PATH_SIZE];
	int st=0;

	if (!fork()) {
		execlp("gzip", "gzip", file_path, NULL);
		perror("Erro 1 ao tentar comprimir ficheiro");
		_exit(1);
	}
	
	sprintf(data_dir, "%s.gz", file_path);
	wait(&st);
	st = WEXITSTATUS(st);
	if (st != 0) return st;

	if (!fork()) {
		execlp("mv", "mv", data_dir, file_path, NULL);
		perror("Erro 2 ao tentar comprimir ficheiro");
		_exit(2);
	}

	wait(&st);
	st = WEXITSTATUS(st);

	return st;
}

/**
 * Gera a hash calculada por sha1sum do ficheiro dado
 * @param file_path Ficheiro a encriptar
 * @return hash gerada
 */
char* generate_hash(char *file_path) {
	char sha1sum[PATH_SIZE], *hash, *ret = NULL;
	int pp[2], status;	

	pipe(pp);

	if (!fork()) {
		close(pp[0]);
		dup2(pp[1], 1);
		
		execlp("sha1sum", "sha1sum", file_path, NULL);
		close(pp[1]);
		perror("Erro ao tentar calcular hash");
		_exit(1);	
	}
	
	wait(&status);
	status = WEXITSTATUS(status);
	if (status != 0) return ret; 

	close(pp[1]);
	read(pp[0], sha1sum, PATH_SIZE);
	close(pp[0]);


	hash = strtok(sha1sum, " ");
	
	ret = malloc(strlen(hash) + 1);
	strcpy(ret, hash);
	return ret;
}

void send_success(pid_t pid) {
	kill(pid, SIGUSR1); //envia sinal de sucesso
}	

void send_error(pid_t pid) {
	kill(pid, SIGUSR2);	 //envia sinal de erro 
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

/**
 * Verifica se existe a raiz do backup. Caso não exista, cria-a.
 * @param home_dir Home do utilizador
 * @return 1 caso crie, 0 caso contrário
 */
int create_root(char *home_dir) {
	struct stat st;
	char root_dir[PATH_SIZE];
	
	// Se a raiz do backup não existir, cria-a.
	sprintf(root_dir, "%s/.Backup", home_dir);	
	if (stat(root_dir, &st) == -1) {
		mkdir(root_dir,0700);
		sprintf(root_dir, "%s/.Backup/data", home_dir);	
		mkdir(root_dir,0700);
		sprintf(root_dir, "%s/.Backup/metadata", home_dir);	
		mkdir(root_dir,0700);
		return 1;
	}

	return 0;
}

