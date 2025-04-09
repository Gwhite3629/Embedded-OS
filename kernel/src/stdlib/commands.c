#include <stdlib.h>

#include <fs/ext2/file.h>
#include <fs/ext2/ext2.h>

#include <memory/malloc.h>

static struct entry_info fs_helper(char *buf, int len)
{
    char c = 0;
    char *spath = NULL;
    struct entry_info res;
    printk(YELLOW("BUF: \"%s\", len: %d\n"), buf, strlen(buf));
    printk(YELLOW("PWD: \"%s\", len: %d\n"), fs->pwd, strlen(fs->pwd));
    if (strlen(buf) < len+2) {
        new(spath, strlen(fs->pwd) + 1, char);
        memset(spath, 0, strlen(fs->pwd) + 1);
        memcpy(spath, fs->pwd, strlen(fs->pwd));
    } else {
        new(spath, strlen(buf + len + 1) + 1, char);
        memset(spath, 0, strlen(buf + len + 1) + 1);
        memcpy(spath, buf + len + 1, strlen(buf + len + 1));
    }

    printk("Raw path: %s\n", spath);
    int j = 0;
    while ((c = spath[j])) {
        printk("%x ", c);
        j++;
    }
    printk("\n");
    if ((strlen(spath) == 1) & (spath[0] == '/')) {
        new(res.entry, 1, fs_entry);
        res.entry[0].dir = root;
        res.type = FS_DIR;
        res.ret = 0;
    } else {
        res = traverse_fs(spath);
    }

exit:
    if (spath) {
        del(spath);
    }
    return res;
}

int cat(char *buf)
{
    int ret = E_NOERR;
    FILE *file = NULL;
    char *buffer = NULL;
    struct entry_info res = fs_helper(buf, 3);

    ret = res.ret;
    if ((ret != E_NOERR) | (res.type != FS_FILE)) {
        printk(RED("Not a regular file\n"));
        goto exit;
    }
    file = res.entry->file;

    new(buffer, file->size, char);

    ret = f_read(file, file->size, buffer);

    printk("%s", buffer);

exit:
    if (file)
        f_close(file);
    return ret;
}

int ls(char *buf)
{
    int ret = E_NOERR;
    fs_tree *dir = NULL;
    struct entry_info res = fs_helper(buf, 2);

    ret = res.ret;
    if ((ret != E_NOERR) | (res.type != FS_DIR)) {
        printk(RED("ls: invalid path\n"));
        goto exit;
    }
    dir = res.entry->dir;
   
    printk("n entries: %d\n", dir->n_entries);

    for (int i = 0; i < dir->n_entries; i++) {
        printk("%1x ", dir->type[i]);
        switch (dir->type[i]) {
            case FS_FILE:
                printk(DIM("%16s %8d\n"), dir->entries[i]->file->name, dir->entries[i]->file->size);
                break;
            case FS_DIR:
                printk(BLUE("%16s %8d\n"), dir->entries[i]->dir->name, dir->entries[i]->dir->n_entries);
                break;
            default:
                printk(DIM(YELLOW("UNKNOWN TYPE\n")));
                break;
        }
    }

exit:
    return ret;
}

int echo(char *buf)
{
    printk("\n%s\n", buf+5);
    return 0;
}

int clear(char *buf)
{
    printk("\x1b[2J]");
    return 0;
}

int show_time(char *buf)
{
    printk("\n%d\n", tick_counter);
    return 0;
}

/*
int call_editor(char *buf)
{
    int ret = E_NOERR;
    char c = 0;
    char *spath = NULL;
    printk(YELLOW("BUF: \"%s\", len: %d\n"), buf, strlen(buf));
    printk(YELLOW("PWD: \"%s\", len: %d\n"), fs->pwd, strlen(fs->pwd));
    if (strlen(buf) < 6) {
        new(spath, strlen(fs->pwd) + 1, char);
        memset(spath, 0, strlen(fs->pwd) + 1);
        memcpy(spath, fs->pwd, strlen(fs->pwd));
    } else {
        new(spath, strlen(buf + 5) + 1, char);
        memset(spath, 0, strlen(buf + 5) + 1);
        memcpy(spath, buf + 5, strlen(buf + 5));
    }

    printk("Raw edit path: %s, %d\n", spath, strlen(spath));
    int j = 0;
    while (c = spath[j]) {
        printk("%x ", c);
        j++;
    }
    printk("\n");

    ret = editor(spath);

exit:
    if (spath) {
        del(spath);
    }

    return ret;
}
*/