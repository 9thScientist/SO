#ifndef __BACKUP_H__
#define __BACKUP_H__
#include "message.h"

/**
 * Faz o backup de acordo com a mensagem dada
 * @param msg mensagem
 * @return código de erro
 */
int backup(MESSAGE msg);

/**
 * Comprime o ficheiro file e guarda-o na ~/.Backup/data com o nome da hash
 * @param file Nome do ficheiro a guardar
 * @param hash Hash indicativa do ficheiro
 * @erturn Código de erro
 */
 int save_data(char *file, char* hash);

 /**
  * Comprime o ficheiro e remove-lhe a extensão .gz
  * @param file_path Ficheiro a comprimir
  * @return Código de erro.
  */
 int compress_file(char* file_path);

 /**
  * Gera a hash calculada por sha1sum do ficheiro dado
  * @param file_path Ficheiro a encriptar
  * @return hash gerada
  */
 char* generate_hash(char *file_path);

#endif
