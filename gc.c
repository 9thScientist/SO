#include "gc.h"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>

#include "message.h"
#include "vec.h"

#define DATA_PATH "/.Backup/data/"
#define METADATA_PATH "/.Backup/metadata"

void get_files_dir(char * dir_path, vec_str_t* vec);
void fill_vec(char *dir, char *aux, int aux_size, vec_str_t* vec);
void get_all_files(char *dir, char *aux, int aux_size);
int is_linked(char * data_path);

int global_clean() {

	char aux_path[PATH_SIZE], file_path[PATH_SIZE], aux[CHUNK_SIZE];
	vec_str_t files;
	int i;

	strncpy(aux_path, getenv("HOME"), PATH_SIZE);
	strncat(aux_path, DATA_PATH, PATH_SIZE);

	vec_init(&files);
	fill_vec(aux_path, aux, CHUNK_SIZE, &files);

	for (i = 0; i < files.length; i++) {
		strncpy(aux_path, getenv("HOME"), PATH_SIZE);
		strncat(aux_path, DATA_PATH, PATH_SIZE);
		strncat(aux_path, files.data[i], PATH_SIZE);
		realpath(aux_path, file_path);

		printf("A testar se %s tem link\n", file_path);
		if (!is_linked(file_path)) 
			unlink(file_path);
	}

	return 0;
}
	
void fill_vec(char *dir, char *aux, int aux_size, vec_str_t* vec) {
	char* tmp;

	get_all_files(dir, aux, aux_size);

	strtok(aux, "\n");
	while((tmp = strtok(NULL, "\n"))) {
		vec_push(vec, tmp);
	}

}

void get_all_files(char *dir, char *aux, int aux_size) {
	int pp[2];

	pipe(pp);
	if(!fork()) {
		close(pp[0]);
		dup2(pp[1], 1);
		printf("find %s\n", dir);
		execlp("find", "find", dir, NULL);
		perror("Erro ao tentar abrir diretoria");
		_exit(1);
	}

	wait(NULL);

	close(pp[1]);
	read(pp[0], aux, aux_size);
	close(pp[0]);
}


int is_linked(char * data_path) {
	char metadata_path[PATH_SIZE], aux[100];
	int pp[2];

	fcntl(pp[0], F_SETFL, O_NONBLOCK);

	strncpy(metadata_path, getenv("HOME"), PATH_SIZE);
	strncat(metadata_path, METADATA_PATH, PATH_SIZE);

	pipe(pp);
	if (!fork()) {
		close(pp[0]);
		dup2(pp[1],1);
	
		printf("find %s -lname %s", metadata_path, data_path );
		execlp("find", "find", metadata_path, "-lname", data_path, NULL);
		perror("Erro ao tentar verificar se ficheiro está linkado");
		close(pp[1]);
		_exit(1);
	}

	wait(NULL);
	close(pp[1]);
	read(pp[0], aux, 100);
	close(pp[0]);

	return !aux[0];	
}
