#include <editor/editor.h>
#include <stdlib.h>
#include <memory/malloc.h>
#include <fs/file.h>
#include <fs/fat32.h>

int editor(char *fname)
{
    int ret = E_NOERR;
    struct environment *env;

    ret = init_env(&env, 0, NULL);

    while (!env->quit) {
        ret = input(&env);
        if (ret != E_NOERR) {
            goto exit;
        }
        if (env->new == 1) {
            draw(&env);
        }
    }

exit:
    destroy_env(&env);

    return ret;
}

int init_env(struct environment **env, int v, char *fname)
{
    int ret = E_NOERR;

    new((*env), 1, struct environment);

    if (v) {
        new((*env)->file_name, strlen(fname), char);
        memcpy((*env)->file_name, fname, strlen(fname)*sizeof(char));
        (*env)->valid_fname = 1;
    } else {
        (*env)->file_name = NULL;
        (*env)->valid_fname = 0;
    }

    new((*env)->file_data, 1, struct line);
    new((*env)->tmp1, 1, struct line);
    new((*env)->tmp2, 1, struct line);
    (*env)->alloc_size = 1;
    (*env)->max = 0;
    (*env)->index = 0;
    (*env)->new = 1;
    (*env)->quit = 0;
    (*env)->top = 0;
    (*env)->bottom = 23;
    init_line(env);

    if ((*env)->valid_fname) {
        read_file(env);
    }

exit:

    return ret;
}

void init_line(struct environment **env)
{
    (*env)->file_data[(*env)->index].index = 0;
    (*env)->file_data[(*env)->index].max = 0;
    memset((*env)->file_data[(*env)->index].ldata, '\0', 4096);
    (*env)->file_data[(*env)->index].ldata[0] = ' ';
    memset((*env)->file_data[(*env)->index].tmp1, '\0', 4096);
    memset((*env)->file_data[(*env)->index].tmp2, '\0', 4096);
}

int read_file(struct environment **env)
{
    int ret = E_NOERR;
    FILE *f = NULL;

    f_open(f, (*env)->file_name, FA_OPEN_ALWAYS | FA_READ);

    unsigned int i = 0;

    while(f_gets((*env)->file_data[i].ldata, 4096, f)) {
        (*env)->alloc_size++;
        (*env)->max++;
        (*env)->file_data[i].index = 0;
        (*env)->file_data[i].max = strlen((*env)->file_data[i].ldata) - 1;
        (*env)->file_data[i].ldata[(*env)->file_data[i].max] = '\0';
        //printf("%d: %ld\n", i, (*env)->file_data[i].max);
        alt((*env)->file_data, (*env)->alloc_size, struct line);
        i++;
        (*env)->index++;
        init_line(env);
    }
    (*env)->new = 1;
    (*env)->index = 0;
    alt((*env)->tmp1, (*env)->alloc_size, struct line);
    alt((*env)->tmp2, (*env)->alloc_size, struct line);

exit:
    if (f)
        f_close(f);

    return ret;
}

int write_file(struct environment **env)
{
    int ret = E_NOERR;

    FILE *f;

    char c = '\0';
    unsigned long l = 0;

    if (!(*env)->valid_fname) {
        printk("Enter file name:\n");
        while(c != '\n') {
            if (c = uart_getc()) {
                if (c != '\n') {
                    l++;
                    alt((*env)->file_name, l, char);
                    (*env)->file_name[l-1] = c;
                }
            }
        }
        (*env)->valid_fname = 1;
        (*env)->new = 1;
    }

    f_open(f, (*env)->file_name, FA_OPEN_ALWAYS | FA_WRITE);

    for (unsigned long i = 0; i <= (*env)->max; i++) {
        f_write(f, (*env)->file_data[i].ldata, sizeof(char), (*env)->file_data[i].max);
        f_putc('\n', f);
    }

exit:
    if (f)
        f_close(f);

    return ret;
}

void destroy_env(struct environment **env)
{
    del((*env)->file_name);
    del((*env)->file_data);
    del((*env)->tmp1);
    del((*env)->tmp2);
    del((*env));
exit:
    return;
}

void editorRefreshScreen(void) {
    printk("\x1b[2J");
}

int draw(struct environment **env)
{
    unsigned long r = 23;

    if (((*env)->bottom - (*env)->top) < r) {
        (*env)->bottom += (r - ((*env)->bottom - (*env)->top));
    }

    if ((*env)->index < (*env)->top) {
        (*env)->top--;
        (*env)->bottom--;
    }

    if ((*env)->index >= (*env)->bottom) {
        (*env)->top++;
        (*env)->bottom++;
    }

    editorRefreshScreen();

    for (unsigned long i = (*env)->top; i < (*env)->bottom; i++) {
        if (i <= (*env)->max) {
            printk("%3ld: ", i);
            //write(STDOUT_FILENO, ": ", 2);
            if (i == ((*env)->index)) {
                for (unsigned long j = 0; j <= (*env)->file_data[i].max; j++) {
                    if (j == ((*env)->file_data[i].index)){
                        if ((*env)->file_data[i].index == (*env)->file_data[i].max) {
                            printk("\033[30m\033[47m ");
                            //write(STDOUT_FILENO, "\033[30m\033[47m ", 11);
                            printk("\033[0m");
                            //write(STDOUT_FILENO, "\033[0m", 4);
                        } else {
                            printk("\033[30m\033[47m%c", (*env)->file_data[i].ldata[j]);
                            //write(STDOUT_FILENO, "\033[0m\033[47m", 9);
                            //write(STDOUT_FILENO, &(*env)->file_data[i].ldata[j], 1);
                            printk("\033[0m");
                            //write(STDOUT_FILENO, "\033[0m", 4);
                        }
                    } else {
                        printk("%c", (*env)->file_data[i].ldata[j]);
                        //write(STDOUT_FILENO, &(*env)->file_data[i].ldata[j], 1);
                    }
                }
                printk("\n");
                //write(STDOUT_FILENO, "\n", 1);
            } else {
                printk("%s\n", (*env)->file_data[i].ldata);
                //write(STDOUT_FILENO, (*env)->file_data[i].ldata, (*env)->file_data[i].max);
                //write(STDOUT_FILENO, "\n", 1);
            }
        } else {
            printk("~\n");
            //write(STDOUT_FILENO, "~\n", 2);
        }
    }

    (*env)->new = 0;

    return E_NOERR;
}

int input(struct environment **env)
{
    int ret = E_NOERR;
    char c = '\0';
    char str[9] = "\0\0\0\0\0\0\0\0\0";
    while(!(*env)->quit) {
        c = uart_getc();
        // Not updated
        if ((c >= 0x20) & (c < 0x7F)) {
            if ((*env)->file_data[(*env)->index].index == (*env)->file_data[(*env)->index].max) {
                (*env)->file_data[(*env)->index].ldata[(*env)->file_data[(*env)->index].index] = c;
            } else {
                memcpy((*env)->file_data[(*env)->index].tmp1, (*env)->file_data[(*env)->index].ldata, (*env)->file_data[(*env)->index].index);
                memcpy((*env)->file_data[(*env)->index].tmp2, (*env)->file_data[(*env)->index].ldata + (*env)->file_data[(*env)->index].index, (*env)->file_data[(*env)->index].max - (*env)->file_data[(*env)->index].index);
                
                memcpy((*env)->file_data[(*env)->index].ldata, (*env)->file_data[(*env)->index].tmp1, (*env)->file_data[(*env)->index].index);
                (*env)->file_data[(*env)->index].ldata[(*env)->file_data[(*env)->index].index] = c;
                memcpy((*env)->file_data[(*env)->index].ldata + (*env)->file_data[(*env)->index].index + 1, (*env)->file_data[(*env)->index].tmp2, (*env)->file_data[(*env)->index].max - (*env)->file_data[(*env)->index].index);

                memset((*env)->file_data[(*env)->index].tmp1, '\0', 4096);
                memset((*env)->file_data[(*env)->index].tmp2, '\0', 4096);
            }
            (*env)->file_data[(*env)->index].max++;
            (*env)->file_data[(*env)->index].index++;
            (*env)->new = 1;
        // Updated
        } else if (c == 0x7F) {
            if ((*env)->file_data[(*env)->index].index == 0) {
                if ((*env)->file_data[(*env)->index].max == 0) {
                    if ((*env)->index == 0) {
                    } else {
                        (*env)->max--;
                        for (unsigned long i = (*env)->index; i <= (*env)->max; i++) {
                            (*env)->file_data[i] = (*env)->file_data[i + 1];
                        }
                        (*env)->alloc_size--;
                        alt((*env)->file_data, (*env)->alloc_size, struct line);
                        alt((*env)->tmp1, (*env)->alloc_size, struct line);
                        alt((*env)->tmp2, (*env)->alloc_size, struct line);
                        (*env)->index--;
                        (*env)->new = 1;
                    }
                } else if ((*env)->index != 0) {
                    unsigned long nb = (*env)->file_data[(*env)->index].max;
                    memcpy((*env)->file_data[(*env)->index - 1].tmp1, (*env)->file_data[(*env)->index].ldata, nb);
                    (*env)->max--;
                    for (unsigned long i = (*env)->index; i <= (*env)->max; i++) {
                        (*env)->file_data[i] = (*env)->file_data[i + 1];
                    }
                    (*env)->alloc_size--;
                    alt((*env)->file_data, (*env)->alloc_size, struct line);
                    alt((*env)->tmp1, (*env)->alloc_size, struct line);
                    alt((*env)->tmp2, (*env)->alloc_size, struct line);
                    (*env)->index--;
                    memcpy((*env)->file_data[(*env)->index].ldata + (*env)->file_data[(*env)->index].max, (*env)->file_data[(*env)->index].tmp1, nb);
                    (*env)->file_data[(*env)->index].index = (*env)->file_data[(*env)->index].max;
                    (*env)->file_data[(*env)->index].max += nb;
                    memset((*env)->file_data[(*env)->index].tmp1, '\0', 4096);
                    (*env)->new = 1;
                }
            } else if ((*env)->file_data[(*env)->index].index == (*env)->file_data[(*env)->index].max) {
                (*env)->file_data[(*env)->index].ldata[(*env)->file_data[(*env)->index].index - 1] = '\0';
                (*env)->file_data[(*env)->index].index--;
                (*env)->file_data[(*env)->index].max--;
                (*env)->new = 1;
            } else {
                memcpy((*env)->file_data[(*env)->index].tmp1, (*env)->file_data[(*env)->index].ldata, (*env)->file_data[(*env)->index].index - 1);
                memcpy((*env)->file_data[(*env)->index].tmp2, (*env)->file_data[(*env)->index].ldata + (*env)->file_data[(*env)->index].index, (*env)->file_data[(*env)->index].max - (*env)->file_data[(*env)->index].index);

                memcpy((*env)->file_data[(*env)->index].ldata, (*env)->file_data[(*env)->index].tmp1, (*env)->file_data[(*env)->index].index - 1);
                memcpy((*env)->file_data[(*env)->index].ldata + (*env)->file_data[(*env)->index].index, (*env)->file_data[(*env)->index].tmp2, (*env)->file_data[(*env)->index].max - (*env)->file_data[(*env)->index].index);

                memset((*env)->file_data[(*env)->index].tmp1, '\0', 4096);
                memset((*env)->file_data[(*env)->index].tmp2, '\0', 4096);
                (*env)->file_data[(*env)->index].index--;
                (*env)->file_data[(*env)->index].max--;
                (*env)->new = 1;
            }
        // Updated
        } else if (c == 0x1B) {
            str[0] = uart_getc();
            str[1] = uart_getc();
            if (!strcmp(str, "[A")) { // Up Arrow
                if ((*env)->index == 0) {
                } else {
                    if ((*env)->file_data[(*env)->index].index < (*env)->file_data[(*env)->index - 1].index) {
                        (*env)->file_data[(*env)->index - 1].index = (*env)->file_data[(*env)->index].index;
                    } else {
                        (*env)->file_data[(*env)->index - 1].index = (*env)->file_data[(*env)->index - 1].max;
                    }
                    (*env)->index--;
                    (*env)->new = 1;
                }
            } else if (!strcmp(str, "[D")) { // Left Arrow
                if ((*env)->file_data[(*env)->index].index == 0) {
                } else {
                    (*env)->file_data[(*env)->index].index--;
                    (*env)->new = 1;
                }
            } else if (!strcmp(str, "[C")) { // Right Arrow
                if ((*env)->file_data[(*env)->index].index == (*env)->file_data[(*env)->index].max) {
                } else {
                    (*env)->file_data[(*env)->index].index++;
                    (*env)->new = 1;
                }
            } else if (!strcmp(str, "[B")) { // Down Arrow
                if ((*env)->index == ((*env)->max)) {
                } else {
                    if ((*env)->file_data[(*env)->index].index < (*env)->file_data[(*env)->index + 1].index) {
                        (*env)->file_data[(*env)->index + 1].index = (*env)->file_data[(*env)->index].index;
                    } else {
                        (*env)->file_data[(*env)->index + 1].index = (*env)->file_data[(*env)->index + 1].max;
                    }
                    (*env)->index++;
                    (*env)->new = 1;
                }
            }
        // Updated
        } else if (c == 0x0A) {
            (*env)->alloc_size++;
            alt((*env)->file_data, (*env)->alloc_size, struct line);
            alt((*env)->tmp1, (*env)->alloc_size, struct line);
            alt((*env)->tmp2, (*env)->alloc_size, struct line);
            if ((*env)->file_data[(*env)->index].index == 0) {
                (*env)->max++;
                for (unsigned long i = (*env)->max; i > (*env)->index; i--) {
                    (*env)->file_data[i] = (*env)->file_data[i-1];
                }
                init_line(env);
                (*env)->index++;
            } else if ((*env)->file_data[(*env)->index].index == (*env)->file_data[(*env)->index].max) {
                for (unsigned long i = (*env)->max; i > ((*env)->index + 1); i--) {
                    (*env)->file_data[i] = (*env)->file_data[i-1];
                }
                (*env)->max++;
                (*env)->index++;
                init_line(env);
            } else {
                (*env)->max++;
                unsigned long nb = (*env)->file_data[(*env)->index].max - (*env)->file_data[(*env)->index].index;
                memcpy((*env)->file_data[(*env)->index].tmp1, (*env)->file_data[((*env)->index)].ldata + (*env)->file_data[(*env)->index].index, nb);
                for (unsigned long i = (*env)->max; i > ((*env)->index + 1); i--) {
                    (*env)->file_data[i] = (*env)->file_data[i-1];
                }
                (*env)->file_data[(*env)->index].ldata[(*env)->file_data[(*env)->index].index] = '\0';
                (*env)->file_data[(*env)->index].max = (*env)->file_data[(*env)->index].index;
                (*env)->index++;
                init_line(env);
                memcpy((*env)->file_data[(*env)->index].ldata, (*env)->file_data[(*env)->index-1].tmp1, nb);
                (*env)->file_data[(*env)->index].index = 0;
                (*env)->file_data[(*env)->index].max = nb;
                memset((*env)->file_data[(*env)->index].tmp1, '\0', 4096);
            }
            (*env)->new = 1;
        } else if (c == 0x09) {
            if ((*env)->file_data[(*env)->index].index == (*env)->file_data[(*env)->index].max) {
                (*env)->file_data[(*env)->index].ldata[(*env)->file_data[(*env)->index].index] = c;
            } else {
                memcpy((*env)->file_data[(*env)->index].tmp1, (*env)->file_data[(*env)->index].ldata, (*env)->file_data[(*env)->index].index);
                memcpy((*env)->file_data[(*env)->index].tmp2, (*env)->file_data[(*env)->index].ldata + (*env)->file_data[(*env)->index].index, (*env)->file_data[(*env)->index].max - (*env)->file_data[(*env)->index].index);
                
                memcpy((*env)->file_data[(*env)->index].ldata, (*env)->file_data[(*env)->index].tmp1, (*env)->file_data[(*env)->index].index);
                (*env)->file_data[(*env)->index].ldata[(*env)->file_data[(*env)->index].index] = c;
                memcpy((*env)->file_data[(*env)->index].ldata + (*env)->file_data[(*env)->index].index + 1, (*env)->file_data[(*env)->index].tmp2, (*env)->file_data[(*env)->index].max - (*env)->file_data[(*env)->index].index);

                memset((*env)->file_data[(*env)->index].tmp1, '\0', 4096);
                memset((*env)->file_data[(*env)->index].tmp2, '\0', 4096);
            }
            (*env)->file_data[(*env)->index].max++;
            (*env)->file_data[(*env)->index].index++;
            (*env)->new = 1;
        } else if (c == 0x13) {
            ret = write_file(env);
            if (ret != E_NOERR) {
                goto exit;
            }
        } else if (c == 0x11) {
            char t = 'n';
            printk("Save? [y/n]:\n");
            t = uart_getc();
            if (t == 'y')
                ret = write_file(env);
                if (ret != E_NOERR) {
                    goto exit;
                }
            (*env)->quit = 1;
        }
        c = '\0';
	    memset(str, '\0', 9);
    }
exit:
    (*env)->quit = 1;
    return ret;
}