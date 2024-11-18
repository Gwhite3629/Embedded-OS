#ifndef __EDITOR_H_
#define __EDITOR_H_

#include "../stdlib/types.h"

struct environment {
    char *file_name; // Name of output file
    int valid_fname; // Indicates if user specified file at startup
    int new; // Indicates if there is a new character to write
    struct line *file_data; // File contents
    unsigned long alloc_size; // Current size of file
    unsigned long index; // Current line number
    unsigned long max; // Max number of lines typed
    unsigned long top;
    unsigned long bottom;
    struct line *tmp1;
    struct line *tmp2;
    int quit;
    int clr;
    struct chr_dat info;
};

struct line {
    char ldata[4096]; // Characters in current line
    unsigned long index; // Current index in line
    unsigned long max; // Max index in line
    char tmp1[4096];
    char tmp2[4096];
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
