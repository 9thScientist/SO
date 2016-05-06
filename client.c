#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <stdio.h>

#define PATH_SIZE 64
#define BUFFER_SIZE 512
#define SERVER_FIFO_PATH "sobuserver_fifo"

int main(int argc, char* argv[]) {
	char message[BUFFER_SIZE], pipe_path[PATH_SIZE];
	int i, server_fifo, client_fifo;
	pid_t pid;
	uid_t uid;

	// Verifica se os argumentos são válidos
	if (argc == 1 || (strcmp(argv[1], "backup") && strcmp(argv[1], "restore"))) {
		fprintf(stderr, "Utilização: sobucli [MODO] ...[FICHEIROS]\nTente 'sobucli --help' para mais ajuda.");
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

	// Prepara e envia informação a partir dos argumentos
	server_fifo = open(SERVER_FIFO_PATH, O_WRONLY);

	if (server_fifo == -1){
		perror("Erro ao tentar comunicar com servidor.");
		return -3;
	}

	pid = getpid();
	uid = getuid();
	
	for(i = 2; i < argc; i++) {
		sprintf(message, "%s %d %d %s", argv[1], (int) pid, (int) uid, argv[i]);
		write(server_fifo, message, strlen(message)+1);
	}	

	// Cria pipe para o servidor comunicar com o cliente
	sprintf(pipe_path, "/tmp/%d", (int) pid);
	if (mkfifo(pipe_path, 0600) == -1) {
		perror("Erro ao tentar criar canal com servidor.");
		return -4;
	}

	client_fifo = open(pipe_path, O_RDONLY);
	
	if (client_fifo == -1) {
		perror("Erro ao tentar abrir canal com servidor.");
		return -5;
	}
	
	while(read(client_fifo, message, BUFFER_SIZE))
		write(1, message, strlen(message));


	close(client_fifo);
	close(server_fifo);
	return 0;
}
