#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "delete.h"

#define BACKUP_PATH "/.Backup/" 
#define DATA_PATH "/.Backup/data/"
#define METADATA_PATH "/.Backup/metadata/"
#define PATHS_PATH "/.Backup/paths/"

int delete(MESSAGE msg) {
	char *f_name, file_path[PATH_SIZE];

	f_name = get_file_name(msg->file_path);

	strncpy(file_path, getenv("HOME"), PATH_SIZE);
	strncat(file_path, METADATA_PATH, PATH_SIZE);
	strncat(file_path, f_name, PATH_SIZE);

	unlink(file_path);
	
	strncpy(file_path, getenv("HOME"), PATH_SIZE);
	strncat(file_path, PATHS_PATH, PATH_SIZE);
	strncat(file_path, f_name, PATH_SIZE);

	unlink(file_path);

	return 0;
}
