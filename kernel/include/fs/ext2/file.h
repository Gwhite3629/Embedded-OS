#ifndef _FILE_H_
#define _FILE_H_

#include "../../stdlib/types.h"

#include "ext2.h"
#include "fstypes.h"

#define PATH_SEP_CHR    '/'
#define PATH_SEP_STR    "/"
#define PATH_UP         ".."
#define PATH_DOT        "."
#define FS_EXT2_MAGIC   0xeeee2222

#define O_RDONLY        0x0000
#define O_WRONLY        0x0001
#define O_RDWR          0x0002
#define O_APPEND        0x0008
#define O_CREATE        0x0200
#define O_TRUNC         0x0400
#define O_EXCL          0x0800
#define O_NOFOLLOW      0x1000
#define O_PATH          0x2000

#define FS_UNKNOWN      0x00
#define FS_FILE         0x01
#define FS_DIR          0x02
#define FS_CHARDEV      0x03
#define FS_BLOCKDEV     0x04
#define FS_FIFO         0x05
#define FS_SOCK         0x06
#define FS_SYMLINK      0x07

/*
typedef union fs_entry {
    struct fs_tree *dir;
    FILE *file;
} fs_entry;

typedef struct fs_tree {
    char *name;
    uint32_t hash;
    uint32_t inode;
    uint32_t n_entries;
    uint8_t *type;
    fs_entry **entries;
} fs_tree;

struct entry_info {
    fs_entry *entry;
    uint8_t type;
    int ret;
};
*/

void fs_init(void);

uint32_t file_get_size(FILE *file);

void fs_open(FILE *file, uint32_t flags);

FILE *finddir(FILE *file, char *name);

void file_mkdir(char *name, uint16_t perm);
void file_mkfile(char *name, uint16_t perm);

int f_create(char *name, uint16_t perm);
uint32_t f_read(FILE *file, uint32_t size, char *buffer);
uint32_t f_write(FILE *file, uint32_t size, char *buffer);
FILE *f_open(const char *name, uint32_t flags);
void f_close(FILE *file);
char* f_gets (FILE* fp, int len, char* buff);
int f_putc (FILE* fp, char c);

char *expand_path(char *input);

int fs_ioctl(FILE *file, int request, void *args);

void fs_chmod(FILE *file, uint32_t mode);

void fs_unlink(char *name);

int fs_symlink(char *value, char *name);

int fs_readlink(FILE *file, char *buf, uint32_t size);

int file_mount(char *mountpoint, FILE *file);

extern fs_tree *root;

#endif // _FILE_H_
