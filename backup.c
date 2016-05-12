#include "backup.h"
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define PATH_SIZE 128 

int backup(MESSAGE msg) {
	int err;
	char root_dir[PATH_SIZE], ln_dir[PATH_SIZE], *hash;
	struct passwd *pw = getpwuid(msg->uid);

	//Gerar uma hash a partir do ficheiro
	hash = generate_hash(msg->argument);
	if (!hash)	return 1;

	//Comprime o ficheiro na raiz com o nome hash
	err = save_data(pw->pw_dir, msg->argument, hash);
	if (err) {
		free(hash);
		return 1;
	}

	//Cria symlink .Backup/data/digest -> .Backup/metadata/ficheiro
	sprintf(root_dir, "%s/.Backup/data/%s", pw->pw_dir, hash);
	sprintf(ln_dir, "%s/.Backup/metadata/%s", pw->pw_dir, msg->argument);
	symlink(root_dir, ln_dir);

	free(hash);

	return 0;
}

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
