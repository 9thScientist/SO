#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <stdio.h>

#define SERVER_FIFO_PATH "sobuserver_fifo"
#define BUFFER_SIZE 512
#define MAX_CHILDREN 5 

void count_dead(int pid);
void write_succ_message();
void write_fail_message();

int alive;
char** current_file;

int main(int argc, char* argv[]) {
	char message[BUFFER_SIZE];
	int i, server_fifo;
	uid_t uid;
	pid_t pid;

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

	signal(SIGCHLD, count_dead);

	// Prepara e envia informação a partir dos argumentos
	server_fifo = open(SERVER_FIFO_PATH, O_WRONLY);

	if (server_fifo == -1){
		perror("Erro ao tentar comunicar com servidor.");
		return -3;
	}

	uid = getuid();

	for(i = 2; i < argc; i++) {
		
		if (alive == MAX_CHILDREN) 
			pause();

		alive++;
		if (!fork()) {		
			pid = getpid();
			current_file = &argv[i];
			signal(SIGUSR1, write_succ_message);
			signal(SIGUSR2, write_fail_message);

			sprintf(message, "%s %d %d %s", argv[1], (int) pid, (int) uid, argv[i]);
			write(server_fifo, message, strlen(message)+1);

			pause();

			_exit(0);
		}
	}	

	close(server_fifo);
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
}

// escreve a mensagem de erro enviada pelo utilizador
void write_fail_message() {
	printf("%s: ERRO - Impossível copiar\n", *current_file);
}
