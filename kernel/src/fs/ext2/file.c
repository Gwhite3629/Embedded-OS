#include <fs/ext2/file.h>
#include <fs/ext2/fstypes.h>
#include <stdlib.h>
#include <memory/malloc.h>

tree_t *fs_tree;
FILE *fs_root;

void fs_init(void)
{
    fs_tree = tree_create();
    fs_entry_t *root = NULL;
    new(root, 1, fs_entry_t);
    root->name = strdup("root");
    root->file = NULL;
    tree_insert(fs_tree, NULL, root);
exit:
}

uint32_t file_get_size(FILE *file)
{
    if(file)
        return ext2_get_file_size(file);
    return 0;
}

uint32_t f_read(FILE *file, uint32_t size, char *buffer)
{
    uint32_t offset = file->cur_offset;
    if (file) {
        unsigned int ret = ext2_read(file, offset, size, buffer);
        file->cur_offset += ret;
        return ret;
    }

    return -1;
}

uint32_t f_write(FILE *file, uint32_t size, char *buffer)
{
    uint32_t offset = file->cur_offset;
    if (file) {
        unsigned int ret = ext2_write(file, offset, size, buffer);
        file->cur_offset += ret;
        return ret;
    }

    return -1;
}

void fs_open(FILE *file, uint32_t flags)
{
    if (!file) return;
    if (file->refcount >= 0) file->refcount++;
    file->cur_offset = 0;
    ext2_open(file, flags);
}

void f_close(FILE *file)
{
    if (!file || file == fs_root || file->refcount == -1) return;
    file->refcount--;
    if (file->refcount == 0)
        ext2_close(file);
}

char* f_gets (FILE* fp, int len, char* buff)
{
    int nc = 0;
	char *p = buff;
	char s[4];
	unsigned int rc;
	uint32_t dc;
    unsigned int ct;

	while (nc < len) {
        rc = f_read(fp, 1, s);		/* Get a code unit */
        if (rc != 1) break;			/* EOF? */
        dc = s[0];
        if (dc >= 0x80) {			/* Multi-byte sequence? */
            ct = 0;
            if ((dc & 0xE0) == 0xC0) {	/* 2-byte sequence? */
                dc &= 0x1F; ct = 1;
            }
            if ((dc & 0xF0) == 0xE0) {	/* 3-byte sequence? */
                dc &= 0x0F; ct = 2;
            }
            if ((dc & 0xF8) == 0xF0) {	/* 4-byte sequence? */
                dc &= 0x07; ct = 3;
            }
            if (ct == 0) continue;
            rc = f_read(fp, ct, s);	/* Get trailing bytes */
            if (rc != ct) break;
            rc = 0;
            do {	/* Merge the byte sequence */
                if ((s[rc] & 0xC0) != 0x80) break;
                dc = dc << 6 | (s[rc] & 0x3F);
            } while (++rc < ct);
            if (rc != ct || dc < 0x80 || dc >= 0x110000) continue;	/* Wrong encoding? */
        }
    }
	len -= 1;	/* Make a room for the terminator */
	while (nc < len) {
		rc = f_read(fp, 1, s);	/* Get a byte */
		if (rc != 1) break;		/* EOF? */
		dc = s[0];
		if (dc == '\r') continue;
		*p++ = (char)dc; nc++;
		if (dc == '\n') break;
	}

	*p = 0;		/* Terminate the string */
	return nc ? buff : 0;	/* When no data read due to EOF or error, return with error. */
}

int f_putc (FILE* fp, char c)
{
	int ret = E_NOERR;

    char buf[2] = {c, '\0'};

    ret = f_write(fp, 1, buf);

    return ret;
}

FILE *finddir(FILE *file, char *name)
{
    if (file && (file->flags & FS_DIR))
        return ext2_finddir(file, name);
    return NULL;
}

void file_mkdir(char *name, uint16_t perm)
{
    int i = strlen(name);
    char *dirname = strdup(name);
    char *save_dirname = dirname;
    char *parent_path = "/";
    while (i > 0) {
        if (dirname[i] == '/') {
            if (i != 0) {
                dirname[i] = '\0';
                parent_path = dirname;
            }
            dirname = &dirname[i+1];
            break;
        }
        i--;
    }

    FILE *parent_node = f_open(parent_path, 0);
    if (!parent_node) {
        del(save_dirname);
    }

    ext2_mkdir(parent_node, dirname, perm);
    del(save_dirname);
    f_close(parent_node);
exit:
}

int f_create(char *name, uint16_t perm)
{
    int i = strlen(name);
    char *dirname = strdup(name);
    char *save_dirname = dirname;
    char *parent_path = "/";
    while(i >= 0) {
        if(dirname[i] == '/') {
            if(i != 0) {
                dirname[i] = '\0';
                parent_path = dirname;
            }
            dirname = &dirname[i+1];
            break;
        }
        i--;
    }

    FILE *parent_node = f_open(parent_path, 0);
    if(!parent_node) {
        del(save_dirname);
        return -1;
    }

    ext2_mkfile(parent_node, dirname, perm);
    del(save_dirname);
    f_close(parent_node);
exit:
    return 0;
}

FILE *get_mountpoint_recurrent(char **path, treenode_t *subroot) {
    int found = 0;
    char *curr_token = strsep(path, "/");
    
    if(curr_token == NULL || !strcmp(curr_token, "")) {
        fs_entry_t *ent = (fs_entry_t *)subroot->value;
        return ent->file;
    }
    
    foreach(child, subroot->children) {
        treenode_t *tchild = (treenode_t *)child->value;
        fs_entry_t *ent = (fs_entry_t *)(tchild->value);
        if(strcmp(ent->name, curr_token) == 0) {
            found = 1;
            subroot = tchild;
            break;
        }
    }

    if(!found) {
        // This token is not found, make path point to this token so that file_open knows from where to start searching in the physical filesystem
        // In another words, for a path like "/home/szhou42", the vfs tree is only aware of the root path "/", because it's the only thing mounted here
        // For the rest of the path "home/szhou42", file_open have to find them in the physical filesystem(such as ext2)
        *path = curr_token;
        return ((fs_entry_t *)(subroot->value))->file;
    }
    // Recursion !
    return get_mountpoint_recurrent(path, subroot);
}

FILE *get_mountpoint(char **path) {
    if(strlen(*path) > 1 && (*path)[strlen(*path) - 1] == '/')
        *(path)[strlen(*path) - 1] = '\0';
    if(!*path || *(path)[0]!= '/') return NULL;
    if(strlen(*path) == 1) {
        *path = '\0';
        fs_entry_t *ent = (fs_entry_t *)fs_tree->root->value;
        return ent->file;

    }
    (*path)++;
    return get_mountpoint_recurrent(path, fs_tree->root);
}

FILE *f_open(const char *name, uint32_t flags)
{
    char *cur_token = NULL;
    char *filename = strdup(name);
    char *save = strdup(filename);
    char *original_filename = filename;
    char *new_start = NULL;

    FILE *nextnode = NULL;
    FILE *startpoint = get_mountpoint(&filename);
    if (!startpoint) return NULL;
    if (filename)
        new_start = strstr(save + (filename - original_filename), filename);
    while (filename != NULL && ((cur_token = strsep(&new_start, "/")) != NULL)) {
        nextnode = finddir(startpoint, cur_token);
        if (!nextnode) return NULL;
        startpoint = nextnode;
    }
    if (!nextnode)
        nextnode = startpoint;

    fs_open(nextnode, flags);
    del(save);
    del(original_filename);
exit:
    return nextnode;
}

char *expand_path(char *input);

int fs_ioctl(FILE *file, int request, void *args);

void fs_chmod(FILE *file, uint32_t mode);

void fs_unlink(char *name);

int fs_symlink(char *value, char *name);

int fs_readlink(FILE *file, char *buf, uint32_t size);

void file_mount_recurrent(char *path, treenode_t *subroot, FILE *file)
{
    int found = 0;
    char *cur_token = strsep(&path, "/");

    if (cur_token == NULL || !strcmp(cur_token, "")) {
        fs_entry_t *ent = (fs_entry_t *)subroot->value;
        if (ent->file) {
            return;
        }
        if (!strcmp(ent->name, "/")) {
            fs_root = file;
        }
        ent->file = file;
        return;
    }

    foreach(child, subroot->children) {
        treenode_t *tchild = (treenode_t *)child->value;
        fs_entry_t *ent = (fs_entry_t *)(tchild->value);
        if (strcmp(ent->name, cur_token) == 0) {
            found = 1;
            subroot = tchild;
        }
    }

    if (!found) {
        fs_entry_t *ent = NULL;
        new(ent, 1, fs_entry_t);
        ent->name = strdup(cur_token);
        subroot = tree_insert(fs_tree, subroot, ent);
    }

    file_mount_recurrent(path, subroot, file);
exit:
}

void file_mount(char *mountpoint, FILE *file)
{
    file->refcount = -1;
    if (mountpoint[0] == '/' && strlen(mountpoint) == 1) {
        fs_entry_t *ent = (fs_entry_t *)fs_tree->root->value;
        fs_root = file;
        ent->file = file;
    }
    file_mount_recurrent(mountpoint + 1, fs_tree->root, file);
}