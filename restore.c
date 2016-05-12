#include "restore.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define PATH_SIZE 128

int restore(MESSAGE msg) {
	int err;
	char data_path[PATH_SIZE], *home;

	home = getenv("HOME");
	sprintf(data_path, "%s/.Backup/metadata/%s", home, msg->argument);

	if (access(data_path, F_OK) == -1) return N_EXIST;

	

	return 0;
}
