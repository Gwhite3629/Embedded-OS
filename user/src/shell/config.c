#include "../../include/syscalls.h"
#include "../../include/string.h"
#include "../../include/err.h"

#include "config.h"
#include "environment.h"
#include "help.h"

char colors[12][8] = {
    KNRM,
    KRED,
    KGRN,
    KYEL,
    KBLU,
    KMAG,
    KCYN,
    KWHT
};

int read_config(config_t *cfg)
{
    FILE *config_file;
    int r = 0;
    char p[4096];

    strcpy(p,ENV.home);
    strcat(p,"/.Fconfig");

    config_file = fopen(p, "r");
    if (config_file == NULL) {
        printf("Failed to open file\n");
        printf("%s: %s: %d\n", __FILE__, __func__, __LINE__);
        return -1;
    }

    fscanf(config_file, "user: %s\n", cfg->username);
    fscanf(config_file, "ncolor: %d\n", &cfg->name_color);
    fscanf(config_file, "pcolor: %d\n", &cfg->path_color);

    fclose(config_file);

    return r;
}

int write_config(config_t *cfg)
{
    FILE *config_file;
    int r = 0;
    char p[4096];

    strcpy(p,ENV.home);
    strcat(p,"/.Fconfig");

    config_file = fopen(p, "w");
    if (config_file == NULL) {
        printf("Failed to open file\n");
        printf("%s: %s: %d\n", __FILE__, __func__, __LINE__);
        return -1;
    }

    fprintf(config_file, "user: %s\n", cfg->username);
    fprintf(config_file, "ncolor: %d\n", cfg->name_color);
    fprintf(config_file, "pcolor: %d\n", cfg->path_color);

    fclose(config_file);

    return r;
}

int verify_files(void)
{
    FILE *config_file;
    FILE *path_file;
    char c[4096];
    char p[4096];

    strcpy(p,ENV.home);
    strcpy(c,ENV.home);
    strcat(p,"/.FPath");
    strcat(c,"/.Fconfig");

    path_file = fopen(p, "r");
    if (path_file == NULL) {
        path_file = fopen(p, "w+");
        if (path_file == NULL) {
            printf("Failed to create path file\n");
            printf("%s: %s: %d\n", __FILE__, __func__, __LINE__);
            return -1;
        }
        fprintf(path_file, "ls /bin/ls\n");
        fclose(path_file);
    } else {
        fclose(path_file);
    }

    config_file = fopen(c, "r");
    if (config_file == NULL) {
        config_file = fopen(c, "w+");
        if (config_file == NULL) {
            printf("Failed to create config file\n");
            printf("%s: %s: %d\n", __FILE__, __func__, __LINE__);
            return -1;
        }
        fprintf(config_file, "user: Default\n");
        fprintf(config_file, "ncolor: 0\n");
        fprintf(config_file, "pcolor: 0\n");
        fclose(config_file);
    } else {
        fclose(config_file);
    }

    return 0;
}
