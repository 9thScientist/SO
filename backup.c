#include "backup.h"
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define BUFFER_SIZE 512 
#define DATA_PATH "/.Backup/data/"
#define METADATA_PATH "/.Backup/metadata/"

int backup(MESSAGE msg) {
	int err;
	char root_dir[PATH_SIZE], ln_dir[PATH_SIZE], *hash, *home, *f_name;
	
	home = getenv("HOME");
	f_name = get_file_name(msg->file_path);

	strncpy(root_dir, home, PATH_SIZE);
	strncat(root_dir, DATA_PATH, PATH_SIZE); 
	strncpy(ln_dir, root_dir, PATH_SIZE);
	strncat(root_dir, f_name, PATH_SIZE);

getchar();
	//Gerar uma hash a partir do ficheiro
	hash = generate_hash(root_dir);
	if (!hash) return 1;
printf("Maria Amélia\n");

	strncat(ln_dir, hash, PATH_SIZE);
	if (access(ln_dir, F_OK) == -1) {
		rename(root_dir, ln_dir); 
		//Comprime o ficheiro na raiz com o nome hash
		err = compress_file(ln_dir);
		if (err) {
			free(hash);
			return 1;
		}
	} else remove(root_dir);

	strncpy(root_dir, home, PATH_SIZE);
	strncat(root_dir, "/.Backup/data/", PATH_SIZE);
	strncat(root_dir, hash, PATH_SIZE);
	strncpy(ln_dir, home, PATH_SIZE);
	strncat(ln_dir, "/.Backup/metadata/", PATH_SIZE);
	strncat(ln_dir, f_name, PATH_SIZE); 
	
	//Cria symlink .Backup/data/digest -> .Backup/metadata/ficheiro
	printf("symlinkg %s -> %s", ln_dir, root_dir); getchar();
	symlink(root_dir, ln_dir);

	free(hash);
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

	st = rename(data_dir, file_path);

	return st;
}

char* generate_hash(char *file_path) {
	char sha1sum[BUFFER_SIZE], *hash, *ret = NULL;
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
