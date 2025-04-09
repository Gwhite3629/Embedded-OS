#ifndef __EDITOR_H_
#define __EDITOR_H_

#include "../stdlib/types.h"

struct environment {
    char *file_name; // Name of output file
    int valid_fname; // Indicates if user specified file at startup
    int new; // Indicates if there is a new character to write
    char *file_data; // File contents
    uint32_t index;
    unsigned long alloc_size; // Current size of file
    int quit;
    int clr;
    struct chr_dat info;
};

int editor(char *fname);
int init_env(struct environment **env, int v, char *fname);
void init_line(struct environment **env);
int read_file(struct environment **env);
int write_file(struct environment **env);
void destroy_env(struct environment **env);
void editorRefreshScreen(void);
int draw(struct environment **env);
int input(struct environment **env);

#endif // __EDITOR_H_
