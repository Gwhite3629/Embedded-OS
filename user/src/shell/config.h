#ifndef _CONFIG_H_
#define _CONFIG_H_

typedef struct config_t {
    char username[32];
    int name_color;
    int path_color;
} config_t;

int read_config(config_t *cfg);

int write_config(config_t *cfg);

int verify_files(void);

extern char colors[12][8];

#endif // CONFIG_H
