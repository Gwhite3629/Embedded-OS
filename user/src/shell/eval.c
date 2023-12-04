#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#include "environment.h"
#include "commands.h"
#include "eval.h"
#include "help.h"
#include "file.h"

int eval(char **tokens, int n_tokens, atomic_bool *quit, path_t **path_table, int *size, config_t *cfg)
{
    pid_t c;
    int r = 0;

    if (n_tokens <= 0) {
        return 0;
    }

    command_t temp;
    path_t temppath;
    temp = lookup(tokens[0]);
    temppath = path_lookup(tokens[0], (*path_table), (*size));

    if (temp.hash == 0) {
        if (temppath.hash != 0) {
            c = fork();
            if (0 == c)
                execv(temppath.path, tokens);
            wait(NULL);
        } else if (strchr(tokens[0],'/') == NULL) {
            printf("Invalid command: %s\n", tokens[0]);
        } else {
            c = fork();
            if (0 == c)
                execv(tokens[0], tokens);
            wait(NULL);
        }
    }

    switch (temp.id) {
        case 0: // EXIT
            (*quit) = 1;
            printf("Exiting\n");
            break;
        case 1: // PRINT
            for (int i = 1; i < n_tokens; i++) {
                printf("%s ", tokens[i]);
            }
            printf("\n");
            break;
        case 2: // HELP
            printf("\
exit:\n\tExit the program\n\
print:\n\tPrint the rest of the line\n\
help:\n\tDisplay this prompt\n\
clear:\n\tClear the screen\n\
user:\n\tSet username\n\
pwd:\n\tDisplay current directory path\n\
cd:\n\tChange Directory\n\
add-path\n\tAdd variable to path\n\
del-path\n\tRemove variable from path\n\
path\n\tPrint the path\n\
ncolor\n\tSet color of name\n\
pcolor\n\tSet color of path\n\
");
            break;
        case 3: // CLEAR
            system("clear"); // try "clear" on linux, cls on windows
            break;
        case 4: // USER
            if (n_tokens < 2) {
                printf("Invalid Username\n");
                return 0;
            } else {
                strcpy(ENV.USERNAME, tokens[1]);
                strcpy(cfg->username, tokens[1]);
                r = write_config(cfg);
            }
            break;
        case 5: // PWD
            printf("%s\n", ENV.path);
            break;
        case 6: // CD
            if (n_tokens < 2) {
                printf("Invalid: No target directory\n");
            } else if (!strcmp(tokens[1], "..") | !strcmp(tokens[1], "../")) {
                if (!strcmp(ENV.path, "/")) {
                    printf("Invalid: Already in root\n");
                } else {
                    char *tmp = strrchr(ENV.path, '/');
                    memset(tmp, '\0', strlen(tmp));
                    if (tmp == ENV.path) {
                        ENV.path[0] = '/';
                    }
                }
            } else if (!strcmp(tokens[1], ".") | !strcmp(tokens[1], "./")) {

            } else if (tokens[1][0] == '/') {
                int v = verify_absolute(tokens[1]);
                if (v == 1) {
                    strcpy(ENV.path, tokens[1]);
                } else {
                    printf("Invalid: Can't find dir\n");
                }
            } else {
                int v = verify_path(tokens[1]);
                if (v == 1) {
                    strcat(ENV.path, "/");
                    strcat(ENV.path, tokens[1]);
                } else {
                    printf("Invalid: Can't find dir\n");
                }
            }
            chdir(ENV.path);
            char *p = malloc(MAX_LINE);
            if (p == NULL) {
                printf("Failed to allocate\n");
                printf("%s: %s: %d\n", __FILE__, __func__, __LINE__);
                perror(strerror(errno));
                return -1;
            }
            if (getcwd(p, MAX_LINE) == NULL) {
                return -1;
            }
            strcpy(ENV.path, p);

            free(p);
            break;
        case 7: // add-path
            if (n_tokens == 1) {
                printf("Please enter variable name and path\n");
                return 0;
            }
            if (n_tokens == 2) {
                printf("Please enter path after variable name\n");
                return 0;
            }
            r = add_path(path_table, size, tokens[1], tokens[2]);
            if (r < 0) {
                printf("Failed to add variable to path\n");
                return r;
            }
            r = write_path(path_table, size);
            if (r < 0) {
                printf("Failed to write path\n");
                return r;
            }
            break;
        case 8: // del-path
            if (n_tokens == 1) {
                printf("Please enter variable name\n");
                return 0;
            }
            r = del_path(path_table, size, tokens[1]);
            if (r < 0) {
                printf("Failed to remove variable from path\n");
                return r;
            }
            r = write_path(path_table, size);
            if (r < 0) {
                printf("Failed to write path\n");
                return r;
            }
            break;
        case 9: // path
            for (int i = 0; i < (*size); i++) {
                printf("\t%s %s\n", (*path_table)[i].name, (*path_table)[i].path);
            }
            break;
        case 10: // ncolor
            if (n_tokens < 2) {
                printf("Enter a color\n");
                return r;
            }
            cfg->name_color = atoi(tokens[1]);
            r = write_config(cfg) % ncolors;
            break;
        case 11: // pcolor
            if (n_tokens < 2) {
                printf("Enter a color\n");
                return r;
            }
            cfg->path_color = atoi(tokens[1]) % ncolors;
            r = write_config(cfg);
            break;
        default:
            break;
    };

    return r;
}
