#include <editor/editor.h>
#include <stdlib.h>
#include <memory/malloc.h>
#include <fs/ext2/file.h>
#include <trace/strace.h>

int editor(char *fname)
{
    push_trace("int editor(char*)","editor",fname,0,0,0);
    int ret = E_NOERR;
    struct environment *env;

    ret = init_env(&env, 1, fname);

    while (!env->quit) {
        ret = input(&env);
        if (ret != E_NOERR) {
            goto exit;
        }
        if (env->new == 1) {
            //draw(&env);
        }
    }

exit:
    destroy_env(&env);
    pop_trace();
    return ret;
}

int init_env(struct environment **env, int v, char *fname)
{
    push_trace("int init_env(struct environment**,int,char*)","init_env",env,v,fname,0);
    int ret = E_NOERR;

    new((*env), 1, struct environment);

    if (v) {
        new((*env)->file_name, strlen(fname)+1, char);
        memset((*env)->file_name, 0, strlen(fname)+1);
        memcpy((*env)->file_name, fname, strlen(fname)*sizeof(char));
        (*env)->valid_fname = 1;
    } else {
        (*env)->file_name = NULL;
        (*env)->valid_fname = 0;
    }

    if ((*env)->valid_fname) {
        read_file(env);
    } else {
        (*env)->alloc_size = 4096;
        new((*env)->file_data, 4096, char);
    }
    (*env)->new = 1;
    (*env)->quit = 0;
    (*env)->clr = 0;

    (*env)->info.x0 = 1300;
    (*env)->info.y0 = 48;
    (*env)->info.color = 0x919CA6;

    if ((*env)->valid_fname) {
        read_file(env);
    }
    //draw(env);
exit:
    pop_trace();
    return ret;
}

int read_file(struct environment **env)
{
    push_trace("int read_file(struct environment **)","read_file",env,0,0,0);
    int ret = E_NOERR;
    FILE *f = NULL;

    f = f_open((*env)->file_name, 0);
    if (f == NULL) {
        ret = E_NOFILE;
        goto exit;
    }

    new((*env)->file_data, (f->size/4096)+1, char);
    memcpy((*env)->file_data, f->file_buffer, f->size);

    (*env)->index = 0;
    (*env)->new = 1;

exit:
    if (f)
        f_close(f);

    pop_trace();
    return ret;
}

void destroy_env(struct environment **env)
{
    del((*env)->file_name);
    del((*env)->file_data);
    del((*env));
exit:
    return;
}

void editorRefreshScreen(void) {
    draw_rect(1296, 32, 1919, 240, 0x0, 1);
    //printk("\x1b[2J");
}
/*
int draw(struct environment **env)
{
    unsigned long r = 23;

    if ((*env)->clr) {
        editorRefreshScreen();
        (*env)->clr = 0;
    }
    (*env)->info.x = (*env)->info.x0;
    (*env)->info.y = (*env)->info.y0;

    for (unsigned long i = (*env)->top; i < (*env)->bottom; i++) {
        if (i <= (*env)->max) {
            print_screen(&(*env)->info, "%3ld: ", i);
            //printk("%3ld: ", i);
            //write(STDOUT_FILENO, ": ", 2);
            if (i == ((*env)->l_num)) {
                for (unsigned long j = 0; j <= (*env)->file_data[i].max; j++) {
                    if (j == ((*env)->file_data[i].index)){
                        if ((*env)->file_data[i].index == (*env)->file_data[i].max) {
                            print_screen(&(*env)->info, " ");
                            //printk("\033[30m\033[47m ");
                            //write(STDOUT_FILENO, "\033[30m\033[47m ", 11);
                            //print_screen(&(*env)->info, "\033[0m");
                            //printk("\033[0m");
                            //write(STDOUT_FILENO, "\033[0m", 4);
                        } else {
                            print_screen(&(*env)->info, "%c", (*env)->file_data[i].ldata[j]);
                            //printk("\033[30m\033[47m%c", (*env)->file_data[i].ldata[j]);
                            //write(STDOUT_FILENO, "\033[0m\033[47m", 9);
                            //write(STDOUT_FILENO, &(*env)->file_data[i].ldata[j], 1);
                            //print_screen(&(*env)->info, "\033[0m");
                            //printk("\033[0m");
                            //write(STDOUT_FILENO, "\033[0m", 4);
                        }
                    } else {
                        print_screen(&(*env)->info, "%c", (*env)->file_data[i].ldata[j]);
                        //printk("%c", (*env)->file_data[i].ldata[j]);
                        //write(STDOUT_FILENO, &(*env)->file_data[i].ldata[j], 1);
                    }
                }
                print_screen(&(*env)->info, "\n");
                //printk("\n");
                //write(STDOUT_FILENO, "\n", 1);
            } else {
                print_screen(&(*env)->info, "%s\n", (*env)->file_data[i].ldata);
                //printk("%s\n", (*env)->file_data[i].ldata);
                //write(STDOUT_FILENO, (*env)->file_data[i].ldata, (*env)->file_data[i].max);
                //write(STDOUT_FILENO, "\n", 1);
            }
        } else {
            print_screen(&(*env)->info, "~\n");
            //printk("~\n");
            //write(STDOUT_FILENO, "~\n", 2);
        }
    }

    (*env)->new = 0;

    return E_NOERR;
}
*/
int input(struct environment **env)
{
    int ret = E_NOERR;
    char c = '\0';
    char str[9] = "\0\0\0\0\0\0\0\0\0";

    if (!(*env)->quit) {
        c = uart_getc();
        printk(YELLOW("GOT INPUT: %x\n"), c);
        // Not updated
        // ASCII Text Characters
        switch(c) {
            // Backspace
            case 0x08:
                break;
            // Horizontal tab
            case 0x09:
                break;
            // linefeed, carriage return
            case 0x0A:
            case 0x0D:
                break;
            case 0x11:
                char t = 'n';
                printk("Save? [y/n]:\n");
                t = uart_getc();
                if (t == 'y') {
                    ret = 0;
                    (*env)->quit = 1;
                }
                break;
            // Arrow key
            case 0x1B:
                break;
            // Regular characters
            default:
        }
        c = '\0';
    }
exit:
    //(*env)->quit = 1;
    return ret;

}
