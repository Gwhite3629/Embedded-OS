#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

#include "file.h"
#include "environment.h"
#include "path.h"
#include "utils.h"

int verify_path(char *path)
{
    char u_path[4096];

    strcpy(u_path, ENV.path);

    strcat(u_path, "/");
    strcat(u_path, path);

    DIR *r = opendir(u_path);
    if (r != NULL) {
        closedir(r);
        return 1;
    }

    return 0;
}

int verify_absolute(char *path)
{
    DIR *r = opendir(path);
    if (r != NULL) {
        return 1;
    }
    return 0;
}

int read_path(path_t **path_table, int *size)
{
    FILE *path_file;
    char pname[32];
    char ppath[4096];
    path_t *temp = NULL;

    char p[4096];

    strcpy(p,ENV.home);
    strcat(p,"/.FPath");

    path_file = fopen(p, "r");
    if (path_file == NULL) {
        printf("Failed to open file\n");
        printf("%s: %s: %d\n", __FILE__, __func__, __LINE__);
        perror(strerror(errno));
        return -1;
    }

    (*path_table) = malloc(1*sizeof(path_t));
    if ((*path_table) == NULL) {
        printf("Failed to allocate\n");
        printf("%s: %s: %d\n", __FILE__, __func__, __LINE__);
        perror(strerror(errno));
        return -1;
    }
    memset((*path_table), 0, 1*sizeof(path_t));
    (*size) = 1;

    while (fscanf(path_file, "%s %s\n", pname, ppath) != EOF) {
        (*size)++;
        temp = realloc((*path_table), (*size)*sizeof(path_t));
        if (temp == NULL) {
            printf("Failed to reallocate\n");
            printf("%s: %s: %d\n", __FILE__, __func__, __LINE__);
            perror(strerror(errno));
            return -1;
        }
        (*path_table) = temp;
        strcpy((*path_table)[(*size)-2].name, pname);
        strcpy((*path_table)[(*size)-2].path, ppath);
        (*path_table)[(*size)-2].hash = hash(pname);
    }
    if ((*size) > 1) {
        (*size)--;
        temp = realloc((*path_table), (*size)*sizeof(path_t));
        if (temp == NULL) {
            printf("Failed to reallocate\n");
            printf("%s: %s: %d\n", __FILE__, __func__, __LINE__);
            perror(strerror(errno));
            return -1;
        }
        (*path_table) = temp;
    }

    qsort((*path_table), (*size), sizeof(path_t), &path_cmp);

    fclose(path_file);

    return 0;
}

int add_path(path_t **path_table, int *size, char *pname, char *ppath)
{
    path_t temp = path_lookup(pname, (*path_table), (*size));
    path_t *scan = bsearch(&temp, (*path_table), (*size), sizeof(path_t), &path_cmp);
    if (scan != NULL) {
        printf("Variable already in path\n");
        return 0;
    }

    path_t *tmp = NULL;
    (*size)++;
    tmp = realloc((*path_table), (*size)*sizeof(path_t));
    if (tmp == NULL) {
        printf("Failed to reallocate\n");
        printf("%s: %s: %d\n", __FILE__, __func__, __LINE__);
        perror(strerror(errno));
        return -1;
    }
    (*path_table) = tmp;
    strcpy((*path_table)[(*size)-1].name, pname);
    strcpy((*path_table)[(*size)-1].path, ppath);
    (*path_table)[(*size)-1].hash = hash(pname);
    qsort((*path_table), (*size), sizeof(path_t), &path_cmp);

    return 0;
}

int del_path(path_t **path_table, int *size, char *pname)
{
    path_t *tmp = NULL;
    path_t temp = path_lookup(pname, (*path_table), (*size));
    path_t *scan = bsearch(&temp, (*path_table), (*size), sizeof(path_t), &path_cmp);
    if ((scan != NULL) & ((*size) > 1)) {
        temp = (*path_table)[(*size)-1];
        (*path_table)[(*size)-1] = (*scan);
        (*scan) = temp;
        (*size)--;
        tmp = realloc((*path_table), (*size)*sizeof(path_t));
        if (tmp == NULL) {
            printf("Failed to reallocate\n");
            printf("%s: %s: %d\n", __FILE__, __func__, __LINE__);
            perror(strerror(errno));
            return -1;
        }
        (*path_table) = tmp;
    } else {
        printf("Variable name not in path\n");
    }

    return 0;
}

int write_path(path_t **path_table, int *size)
{
    FILE *path_file;

    char p[4096];

    strcpy(p,ENV.home);
    strcat(p,"/.FPath");

    path_file = fopen(p, "w");
    if (path_file == NULL) {
        printf("Failed to open file\n");
        printf("%s: %s: %d\n", __FILE__, __func__, __LINE__);
        perror(strerror(errno));
        return -1;
    }

    for (int i = 0; i < (*size); i++) {
        fprintf(path_file, "%s %s\n", (*path_table)[i].name, (*path_table)[i].path);
    }

    fclose(path_file);

    return 0;
}
