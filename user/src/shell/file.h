#ifndef FILE_H
#define FILE_H

#include "path.h"

int verify_path(char *path);

int verify_absolute(char *path);

int read_path(path_t **path_table, int *size);

int add_path(path_t **path_table, int *size, char *pname, char *ppath);

int del_path(path_t **path_table, int *size, char *pname);

int write_path(path_t **path_table, int *size);

#endif // FILE_H