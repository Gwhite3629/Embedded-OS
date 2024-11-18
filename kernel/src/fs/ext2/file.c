#include <fs/ext2/file.h>
#include <fs/ext2/fstypes.h>
#include <stdlib.h>
#include <memory/malloc.h>
#include <trace/strace.h>

fs_tree *root;

uint32_t file_get_size(FILE *file)
{
    push_trace("uint32_t file_get_size(FILE *)","file_get_size",file,0,0,0);
    if(file) {
        pop_trace();
        return ext2_get_file_size(file);
    }

    pop_trace();
    return 0;
}

uint32_t f_read(FILE *file, uint32_t size, char *buffer)
{
    push_trace("uint32_t f_read(FILE*,uint32_t,char*)","f_read",file,size,buffer,0);
    uint32_t offset = file->cur_offset;
    if (file) {
        unsigned int ret = ext2_read(file, offset, size, buffer);
        file->cur_offset += ret;
        pop_trace();
        return ret;
    }

    pop_trace();
    return -1;
}

uint32_t f_write(FILE *file, uint32_t size, char *buffer)
{
    push_trace("uint32_t f_write(FILE*,uint32_t,char*)","f_write",file,size,buffer,0);
    uint32_t offset = file->cur_offset;
    if (file) {
        unsigned int ret = ext2_write(file, offset, size, buffer);
        file->cur_offset += ret;
        pop_trace();
        return ret;
    }

    pop_trace();
    return -1;
}

void fs_open(FILE *file, uint32_t flags)
{
    push_trace("void fs_open(FILE*,uint32_t)","fs_open",file,flags,0,0);
    if (!file) {
        pop_trace();
        return;
    }
    if (file->refcount >= 0) file->refcount++;
    file->cur_offset = 0;
    ext2_open(file, flags);

    pop_trace();
}

void f_close(FILE *file)
{
    push_trace("void f_close(FILE*)","f_close",file,0,0,0);
    if (!file || file->refcount == -1) return;
    file->refcount--;
    if (file->refcount == 0)
        ext2_close(file);

    pop_trace();
}

char* f_gets (FILE* fp, int len, char* buff)
{
    push_trace("char *f_gets(FILE*,int,char*)","f_gets",fp,len,buff,0);
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

    pop_trace();
	return nc ? buff : 0;	/* When no data read due to EOF or error, return with error. */
}

int f_putc (FILE* fp, char c)
{
    push_trace("int f_putc(FILE*,char)","f_putc",fp,c,0,0);
	int ret = E_NOERR;

    char buf[2] = {c, '\0'};

    ret = f_write(fp, 1, buf);
    
    pop_trace();
    return ret;
}

FILE *finddir(FILE *file, char *name)
{
    push_trace("FILE *finddir(FILE*,char*)","finddir",file,name,0,0);
    if (file && (file->flags & FS_DIR)) {
        pop_trace();
        return ext2_finddir(file, name);
    }
    pop_trace();
    return NULL;
}
/*
void file_mkdir(char *name, uint16_t perm)
{
    push_trace("void file_mkdir(char*,uint16_t)","file_mkdir",name,perm,0,0);

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
    pop_trace();
}

int f_create(char *name, uint16_t perm)
{
    push_trace("int f_create(name,perm)","f_create",name,perm,0,0);
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
        pop_trace();
        return -1;
    }

    ext2_mkfile(parent_node, dirname, perm);
    del(save_dirname);
    f_close(parent_node);
exit:
    pop_trace();
    return 0;
}

FILE *get_mountpoint_recurrent(char **path, treenode_t *subroot) {
    push_trace("FILE *get_mnt_recur(char**,treenode_t)","get_,mnt_recur",path,subroot,0,0);
    
    int found = 0;
    char *curr_token = strsep(path, "/");
    
    if(curr_token == NULL || !strcmp(curr_token, "")) {
        fs_entry_t *ent = (fs_entry_t *)subroot->value;
        pop_trace();
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
        pop_trace();
        return ((fs_entry_t *)(subroot->value))->file;
    }
    // Recursion !
    pop_trace();
    return get_mountpoint_recurrent(path, subroot);
}

FILE *get_mountpoint(char **path) {
    push_trace("FILE *get_mnt(char**)","get_mnt",path,0,0,0);
    
    if(strlen(*path) > 1 && (*path)[strlen(*path) - 1] == '/')
        *(path)[strlen(*path) - 1] = '\0';
    if(!*path || *(path)[0]!= '/') return NULL;
    if(strlen(*path) == 1) {
        *path = '\0';
        fs_entry_t *ent = (fs_entry_t *)fs_tree->root->value;
        pop_trace();
        return ent->file;

    }
    (*path)++;
    pop_trace();
    return get_mountpoint_recurrent(path, fs_tree->root);
}

FILE *f_open(const char *name, uint32_t flags)
{
    push_trace("FILE *f_open(const char*,uint32_t)","f_open",name,flags,0,0);
    
    char *cur_token = NULL;
    char *filename = strdup(name);
    char *save = strdup(filename);
    char *original_filename = filename;
    char *new_start = NULL;

    FILE *nextnode = NULL;
    FILE *startpoint = get_mountpoint(&filename);
    if (!startpoint) { 
        pop_trace();
        return NULL;
    }
    if (filename) {
        new_start = strstr(save + (filename - original_filename), filename);
    }
    while (filename != NULL && ((cur_token = strsep(&new_start, "/")) != NULL)) {
        nextnode = finddir(startpoint, cur_token);
        if (!nextnode) {
            pop_trace();
            return NULL;
        }
        startpoint = nextnode;
    }
    if (!nextnode) {
        nextnode = startpoint;
    }

    fs_open(nextnode, flags);
    del(save);
    del(original_filename);
exit:
    pop_trace();
    return nextnode;
}
*/
char *expand_path(char *input);

int fs_ioctl(FILE *file, int request, void *args);

void fs_chmod(FILE *file, uint32_t mode);

void fs_unlink(char *name);

int fs_symlink(char *value, char *name);

int fs_readlink(FILE *file, char *buf, uint32_t size);
/*
int file_mount_recurrent(char *path, treenode_t *subroot, FILE *file)
{
    push_trace("int f_mnt_recur(char*,treenode_t*,FILE*)","f_mnt_recur",path,subroot,file,0);
    int ret = E_NOERR;
    int found = 0;
    char *cur_token = NULL;
    
    if (strlen(path) != 1) {
        cur_token = strsep(&path, "/");
    }

    if (cur_token == NULL || !strcmp(cur_token, "")) {
        fs_entry_t *ent = (fs_entry_t *)subroot->value;
        if (ent->file) {
            pop_trace();
            return ret;
        }
        if (!strcmp(ent->name, "/")) {
            fs_root = file;
        }
        ent->file = file;
        pop_trace();
        return ret;
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

    ret = file_mount_recurrent(path, subroot, file);
exit:
    pop_trace();
    return ret;
}

int file_mount(char *mountpoint, FILE *file)
{
    push_trace("int f_mnt(char*,FILE*)","f_mnt",mountpoint,file,0,0);
    int ret = E_NOERR;
    file->refcount = -1;
    if (strlen(mountpoint) > 1 && mountpoint[strlen(mountpoint) - 1] == '/') {
        mountpoint[strlen(mountpoint) - 1] = '\0';
    }
    if (!mountpoint || mountpoint[0] != '/') {
        printk(RED("VFS: Invalid mount point provided\n"));
        pop_trace();
        return E_FINT;
    }
    if (strlen(mountpoint) == 1) {
        mountpoint[0] = '\0';
        fs_entry_t *ent = (fs_entry_t *)fs_tree->root->value;
        printk(YELLOW("VFS: Assigning root values\n"));
        fs_root = file;
        ent->file = file;
        printk(YELLOW("VFS: Returning early for root case\n"));
        pop_trace();
        return E_NOERR;
    }
    ret = file_mount_recurrent(mountpoint + 1, fs_tree->root, file);
    pop_trace();
    return ret;
}
*/
struct entry_info traverse_fs(const char *path)
{
    int ret = E_NOERR;
    int i = 0;
    int found = 0;
    list_t *path_list = NULL;
    unsigned int n_token = 0;

    struct entry_info entry_return;

    char *use_path;

    new(use_path,strlen(path),char);
    
    int plen = strlen(path);

    // Relative case, prepend pwd
    // THIS NEEDS TO BE A PID RELATIVE VALUE
    // CURRENTLY GLOBAL KERNEL VALUE HELD IN FS
    // PWD also must begin with '/' but not end with '/' unless root
    if (path[0] != '/') {
        if ((fs->pwd != NULL) & (fs->pwd[strlen(fs->pwd) - 1] != '/')) {
            strncpy(use_path,fs->pwd,strlen(fs->pwd));
            strncat(use_path,path,plen);
        } else {
            printk(RED("VFS: pwd invalid\n"));
            ret = E_BADPATH;
            goto exit;
        }
    } else {
        strncpy(use_path,path,plen);
    }

    // Get linked list of path names, values are strings
    path_list = str_split(use_path,"/",&n_token);
    
    // Path has no entries, root is handled seperately
    if ((n_token == 0) | (path_list == NULL)) {
        printk(RED("Failed to traverse: null path\n"));
        ret = E_BADPATH;
        goto exit;
    }

    fs_tree *fs_head = root;
    listnode_t *t = path_list->head;
    // Traversal of only directories
    for (int i = 0; i < n_token - 1; i++) {
        // Check for good search conditions:
        // Path still good
        // Path has valid string
        // VFS dir has '.' and '..' minimum
        // VFS dir has real entries
        if ((found == 0) | (t == NULL) | (fs_head->n_entries < 2) | (fs_head->entries == NULL)) {
            printk(RED("VFS: path hashing failed\n"));
            ret = E_BADPATH;
            goto exit;
        }

        found = 0;

        // Traverse all VFS dir entries and assume dir type
        // Assumption is safe because an invalid access is a read
        for (int j = 0; j < fs_head->n_entries; j++) {
            if (fs_head->entries[j]->dir->hash == hash32(t->value)) {
                fs_head = fs_head->entries[j]->dir;
                found = 1;
                break;
            }
        }

        // Next path string
        t = t->next;
    }

    // Path not valid, fail
    if (found == 0) {
        printk(RED("VFS: path hashing failed\n"));
        ret = E_BADPATH;
        goto exit;
    }

    uint32_t hash = 0;

    // Type agnostic endpoint, can add more types here eventually
    for (int j = 0; j < fs_head->n_entries; j++) {
        switch (fs_head->type[j]) {
            case FS_FILE:
                hash = fs_head->entries[j]->file->hash;
                break;
            case FS_DIR:
                hash = fs_head->entries[j]->dir->hash;
                break;
            default:
                printk(RED("VFS: invalid file type\n"));
                ret = E_BADPATH;
                goto exit;
        }
        if (hash == hash32(t->value)) {
            entry_return.entry = fs_head->entries[j];
            entry_return.type = fs_head->type[j];
            break;
        }
    }

exit:
    if (path_list) {
        del(path_list);
    }
    entry_return.ret = ret;
    return entry_return;
}
