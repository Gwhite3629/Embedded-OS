#include <fs/ext2/ext2.h>
#include <fs/ext2/inode.h>
#include <fs/ext2/file.h>
#include <fs/ext2/part.h>

#include <drivers/sd.h>

#include <memory/malloc.h>

#include <trace/strace.h>

#include <stdlib.h>

EXT2_t *fs;

uint32_t ext2_get_file_size(FILE *file)
{   
    push_trace("uint32_t ext2_get_file_size(FILE*)","ext2_get_file_size",file,0,0,0);
    inode_base_t inode;
    read_inode_metadata(&inode, file->inode);
    pop_trace();
    return inode.size_lower;
}

void ext2_mkdir(FILE *file, char *name, uint16_t perm)
{
    push_trace("uint32_t ext2_mkdir(FILE*,char*,uint16_t)","ext2_mkdir",file,name,perm,0);
    uint32_t index = alloc_inode();
    inode_base_t *inode = NULL;
    new(inode, 1, inode_base_t);
    read_inode_metadata(inode, index);
    inode->inode_type_perm = INODE_DIR;
    inode->inode_type_perm |= 0xFFF & perm;
    inode->last_access_time = 0;
    inode->creation_time = 0;
    inode->deletion_time = 0;
    inode->group_ID = 0;
    inode->user_ID = 0;
    inode->frag_block_addr = 0;
    inode->n_sectors = 0;
    inode->size_lower = fs->block_size;
    inode->n_hard_links = 2;
    inode->flags = 0;
    inode->ext_attr_block = 0;
    inode->f_size_dir_acl = 0;
    inode->gen_num = 0;
    inode->OS_Val_1 = 0;
    memset(inode->blocks, 0, sizeof(uint32_t) * 15);
    memset(inode->OS_Val_2, 0, 12);
 
    // Need to add . and ..

    alloc_inode_block(inode, index, 0);
    write_inode_metadata(inode, index);
    ext2_create_entry(file, name, index);

    inode_base_t *p_inode = NULL;
    new(p_inode, 1, inode_base_t);
    read_inode_metadata(p_inode, file->inode);
    p_inode->n_hard_links++;
    write_inode_metadata(p_inode, file->inode);
    write_bgd();
exit:
    pop_trace();
}

void ext2_mkfile(FILE *file, char *name, uint16_t perm)
{
    push_trace("void ext2_mkfile(FILE*,char*,uint16_t)","ext2_mkfile",file,name,perm,0);
    uint32_t index = alloc_inode();

    inode_base_t *inode = NULL;
    new(inode, 1, inode_base_t);
    read_inode_metadata(inode, index);
    inode->inode_type_perm = INODE_FILE;
    inode->inode_type_perm |= 0xFFF & perm;
    inode->last_access_time = 0;
    inode->creation_time = 0;
    inode->deletion_time = 0;
    inode->group_ID = 0;
    inode->user_ID = 0;
    inode->frag_block_addr = 0;
    inode->n_sectors = 0;
    inode->size_lower = fs->block_size;
    inode->n_hard_links = 2;
    inode->flags = 0;
    inode->ext_attr_block = 0;
    inode->f_size_dir_acl = 0;
    inode->gen_num = 0;
    inode->OS_Val_1 = 0;
    memset(inode->blocks, 0, sizeof(uint32_t) * 15);
    memset(inode->OS_Val_2, 0, 12);

    alloc_inode_block(inode, index, 0);
    write_inode_metadata(inode, index);
    ext2_create_entry(file, name, index);

    inode_base_t *p_inode = NULL;
    new(p_inode, 1, inode_base_t);
    read_inode_metadata(p_inode, file->inode);
    p_inode->n_hard_links++;
    write_inode_metadata(p_inode, file->inode);
    write_bgd();
exit:
    pop_trace();
}

void ext2_unlink(FILE *file, char *name)
{
    push_trace("void ext2_unlink(FILE*,char*)","ext2_unlink",file,name,0,0);
    ext2_remove_entry(file, name);

    inode_base_t *p_inode = NULL;
    new(p_inode, 1, inode_base_t);
    read_inode_metadata(p_inode, file->inode);
    p_inode->n_hard_links--;
    write_inode_metadata(p_inode, file->inode);
    write_bgd();
exit:
    pop_trace();
}

char **ext2_listdir(FILE *file)
{
    push_trace("char**ext2_listdir(FILE*)","ext2_listdir",file,0,0,0);
    char **ll = NULL;
    inode_base_t *p_inode = NULL;
    new(p_inode, 1, inode_base_t);
    read_inode_metadata(p_inode, file->inode);

    uint32_t cur_offset = 0;
    uint32_t block_offset = 0;
    uint32_t in_block_offset = 0;
    int size = 0;
    int cap = 10;

    new(ll, cap, char *);
    char *block_buf = read_inode_block(p_inode, block_offset);

    while (cur_offset < p_inode->size_lower) {
        if (in_block_offset >= fs->block_size) {
            block_offset++;
            in_block_offset = 0;
            block_buf = read_inode_block(p_inode, block_offset);
        }
        if (size + 1 == cap) {
            alt(ll, cap*2, char *);
            cap = cap * 2;
        }
        dirent_t *cur_dir = (dirent_t *)(block_buf + in_block_offset);
        if (cur_dir->inode_number != 0) {
            char *temp = NULL;
            new(temp, cur_dir->name_len + 1, char);
            memcpy(temp, cur_dir->name, cur_dir->name_len);
            ll[size++] = temp;
        }
        uint32_t expected_size = ((sizeof(dirent_t) + cur_dir->name_len) & 0xfffffffc) + 0x4;
        uint32_t real_size = cur_dir->size;
        if (real_size != expected_size) {
            break;
        }
        in_block_offset += cur_dir->size;
        cur_offset += cur_dir->size;
    }

    ll[size] = NULL;
exit:
    pop_trace();
    return ll;
}

FILE *file_from_dirent(dirent_t *dir, inode_base_t *inode)
{
    push_trace("FILE *file_from_dirent(dirent_t*,inode_bast_t*)","file_from_dirent",dir,inode,0,0);
    FILE *f = NULL;
    new(f, 1, FILE);

    f->fs = fs;

    f->inode = dir->inode_number;
    memcpy(f->name, dir->name, dir->name_len);

    f->size = inode->size_lower;
    f->nlink = inode->n_hard_links;

    f->flags = 0;
    
    f->access_time = inode->last_access_time;
    f->modify_time = inode->last_mod_time;
    f->create_time = inode->creation_time;
exit:
    pop_trace();
    return f;
}

FILE *ext2_finddir(FILE *file, char *name)
{
    push_trace("FILE *ext2_finddir(FILE*,char*)","ext2_finddir",file,name,0,0);
    inode_base_t *p_inode = NULL;
    new(p_inode, 1, inode_base_t);
    read_inode_metadata(p_inode, file->inode);
    uint32_t expected_size;
    uint32_t real_size;
    uint32_t cur_offset = 0;
    uint32_t block_offset = 0;
    uint32_t in_block_offset = 0;
    char *block_buf = read_inode_block(p_inode, block_offset);
    
    while(cur_offset < p_inode->size_lower) {
        if (in_block_offset >= fs->block_size) {
            block_offset++;
            in_block_offset = 0;
            block_buf = read_inode_block(p_inode, block_offset);
        }

        dirent_t *cur_dir = (dirent_t *)(block_buf + in_block_offset);
        char *temp = NULL;
        new(temp, cur_dir->name_len + 1, char);
        memcpy(temp, cur_dir->name, cur_dir->name_len);

        if (cur_dir->inode_number != 0 && !strcmp(temp, name)) {
            inode_base_t *inode = NULL;
            new(inode, 1, inode_base_t);
            read_inode_metadata(inode, cur_dir->inode_number);
            pop_trace();
            return file_from_dirent(cur_dir, inode);
        }
        if (((sizeof(dirent_t) + cur_dir->name_len) & 0x3) != 0) {
            expected_size = ((sizeof(dirent_t) + cur_dir->name_len) & 0xfffffffc) + 0x4;
        } else {
            expected_size = ((sizeof(dirent_t) + cur_dir->name_len) & 0xfffffffc);
        }
        real_size = cur_dir->size;
        if (real_size != expected_size) {
            break;
        }
        in_block_offset += cur_dir->size;
        cur_offset += cur_dir->size;
    }
exit:
    pop_trace();
    return NULL;
}

void ext2_create_entry(FILE *file, char *name, uint32_t inode)
{
    push_trace("void ext2_create_entry(FILE*,char*,uint32_t)","ext2_create_entry",file,name,inode,0);
    inode_base_t * p_inode = NULL;
    new(p_inode, 1, inode_base_t);
    read_inode_metadata(p_inode, file->inode);
    uint32_t cur_offset = 0;
    uint32_t block_offset = 0;
    uint32_t in_block_offset = 0;
    uint32_t found = 0;
    uint32_t entry_name_len = strlen(name);

    char * check = NULL;
    new(check, entry_name_len + 1, char);
    char * block_buf = read_inode_block( p_inode, block_offset);
    
    while(cur_offset < p_inode->size_lower) {
        if(in_block_offset >= fs->block_size) {
            block_offset++;
            in_block_offset = 0;
            block_buf = read_inode_block(p_inode, block_offset);
        }
        dirent_t * cur_dir = (dirent_t*)(block_buf + in_block_offset);
        if(cur_dir->name_len == entry_name_len) {
            memcpy(check, cur_dir->name, entry_name_len);
            if(cur_dir->inode_number != 0 && !strcmp(name, check)) {
                pop_trace();
                return;
            }
        }
        // Found the last entry
        if(found) {
            // Overwrite this last entry with our new entry
            cur_dir->inode_number = inode;
            cur_dir->size = (uint16_t)fs->block_size;
            cur_dir->name_len = strlen(name);
            cur_dir->type = 0;
            // Must use memcpy instead of strcpy, because name in direntry does not contain ending '\0'
            memcpy(cur_dir->name, name, strlen(name));
            write_inode_block(p_inode, block_offset, block_buf);
            // Then, append a new ending entry
            // Be careful about a spcial case here, if not enough space for ending entry, put it in the new block
            in_block_offset += cur_dir->size;
            if(in_block_offset >= fs->block_size) {
                 block_offset++;
                 in_block_offset = 0;
                 block_buf = read_inode_block(p_inode, block_offset);
            }
            cur_dir = (dirent_t*)(block_buf + in_block_offset);
            memset(cur_dir, 0, sizeof(dirent_t));
            write_inode_block(p_inode, block_offset, block_buf);
            pop_trace();
            return;
        }
        uint32_t expected_size = ((sizeof(dirent_t) + cur_dir->name_len) & 0xfffffffc) + 0x4;
        uint32_t real_size = cur_dir->size;
        if(real_size != expected_size) {
            // Mark found and fix the size
            found = 1;
            cur_dir->size = expected_size;
            in_block_offset += expected_size;
            cur_offset += expected_size;
            continue;
        }
        in_block_offset += cur_dir->size;
        cur_offset += cur_dir->size;
    }
exit:
    pop_trace();
}

void ext2_remove_entry(FILE *file, char *name)
{
    push_trace("void ext2_remove_entry(FILE*,char*)","ext2_remove_entry",file,name,0,0);
    inode_base_t * p_inode = NULL;
    new(p_inode, 1, inode_base_t);
    read_inode_metadata(p_inode, file->inode);
    uint32_t cur_offset = 0;
    uint32_t block_offset = 0;
    uint32_t in_block_offset = 0;
    uint32_t entry_name_len = strlen(name);
    
    char * check = NULL;
    new(check, entry_name_len + 1, char);

    char * block_buf = read_inode_block(p_inode, block_offset);
    // Note: It is required that no directory entry cross block boundary && each directory entry be aligned on 4-byte boundary
    while(cur_offset < p_inode->size_lower) {
        if(in_block_offset >= fs->block_size) {
            block_offset++;
            in_block_offset = 0;
            block_buf = read_inode_block(p_inode, block_offset);
        }
        dirent_t * cur_dir = (dirent_t*)(block_buf + in_block_offset);
        if(cur_dir->name_len == entry_name_len) {
            memcpy(check, cur_dir->name, entry_name_len);
            if(cur_dir->inode_number != 0 && !strcmp(name, check)) {
                cur_dir->inode_number = 0;
                write_inode_block(p_inode, block_offset, block_buf);
                pop_trace();
                return;
            }
        }
        uint32_t expected_size = ((sizeof(dirent_t) + cur_dir->name_len) & 0xfffffffc) + 0x4;
        uint32_t real_size = cur_dir->size;
        // Found the last entry
        if(real_size != expected_size) {
            pop_trace();
            return;
        }
        in_block_offset += cur_dir->size;
        cur_offset += cur_dir->size;
    }
exit:
    pop_trace();
}


void ext2_chmod(FILE *file, uint32_t mode)
{
    push_trace("void ext2_chmod(FILE*,uint32_t)","ext2_chmod",file,mode,0,0);
    inode_base_t * inode = NULL;
    new(inode, 1, inode_base_t);
    read_inode_metadata(inode, file->inode);
    inode->inode_type_perm = (inode->inode_type_perm & 0xFFFFF000) | mode;
    write_inode_metadata(inode, file->inode);
exit:
    pop_trace();
}

uint32_t ext2_read(FILE *file, uint32_t offset, uint32_t size, char * buf)
{
    push_trace("uint32_t ext2_read(FILE*,uint32_t,uint32_t,char*)","ext2_read",file,offset,size,buf);
    // Extract the ext2 filesystem object and inode from vfs node
    inode_base_t * inode = NULL;
    new(inode, 1, inode_base_t);
    read_inode_metadata(inode, file->inode);
    read_inode_filedata(inode, offset, size, buf);
exit:
    pop_trace();
    return size;
}

uint32_t ext2_write(FILE *file, uint32_t offset, uint32_t size, char * buf)
{
    push_trace("uint32_t ext2_write(FILE*,uint32_t,uint32_t,char*)","ext2_write",file,offset,size,buf);
    // Extract the ext2 filesystem object and inode from vfs node
    inode_base_t * inode =  NULL;
    new(inode, 1, inode_base_t);
    read_inode_metadata(inode, file->inode);
    write_inode_filedata(inode, file->inode, offset, size, buf);
exit:
    pop_trace();
    return size;
}

void ext2_open(FILE *file, uint32_t flags)
{
    push_trace("void ext2_open(FILE*,uint32_t)","ext2_open",file,flags,0,0);
    // Overwrite the file on open
    // Add flags for other modes and create them
    
    if (file->file_buffer == NULL) {
        new(file->file_buffer, file->size, char);
        printk(GREEN("Created File buffer of size: %d\n"), file->size);
        ext2_read(file, 0, file->size, file->file_buffer);
    }

exit:
    pop_trace();
}

void ext2_close(FILE *file)
{
    if (file->file_buffer != NULL) {
        del(file->file_buffer);
    }
exit:
    return;
}

void write_bgd(void)
{
    for(uint32_t i = 0; i < fs->bgd_blocks; i++)
        write_disk_block(2, (void*)fs->BGD + i * fs->block_size);
}

void write_superblock(void)
{
    write_disk_block(1, (void*)fs->superblock);
}

uint32_t ext2_alloc_block(void)
{
    push_trace("uint32_t ext2_alloc_block(void)","ext2_alloc_block",0,0,0,0);
    uint32_t *buf = NULL;
    new(buf, fs->block_size, uint32_t);
    // Read the inode bitmap, find free inode, return its index
    for(uint32_t i = 0; i < fs->n_groups; i++) {
        if(!fs->BGD[i].n_unalloc_blocks)
            continue;

        uint32_t bitmap_block = fs->BGD[i].block_bitmap_addr;
        read_disk_block(bitmap_block, (void*)buf);
        for(uint32_t j = 0; j < fs->block_size / 4; j++) {
            uint32_t sub_bitmap = buf[j];
            if(sub_bitmap == 0xFFFFFFFF)
                continue;
            for(uint32_t k = 0; k < 32; k++) {
                uint32_t free = !((sub_bitmap >> k) & 0x1);
                if(free) {
                    // Set bitmap and return
                    uint32_t mask = (0x1 << k);
                    buf[j] = buf[j] | mask;
                    write_disk_block(bitmap_block, (void*)buf);
                    // update free_inodes
                    fs->BGD[i].n_unalloc_blocks--;
                    write_bgd();
                    pop_trace();
                    return i * fs->blocks_per_group + j * 32 + k;
                }
            }
        }
    }
exit:
    pop_trace();
    return (uint32_t)-1;
}

/*
 * Free block from the ext2 block bitmaps
 * */
void ext2_free_block(uint32_t block)
{
    push_trace("void ext2_free_block(uint32_t)","ext2_free_block",block,0,0,0);
    uint32_t *buf = NULL;
    new(buf, fs->block_size, uint32_t);
    // Which group it belongs to ?
    uint32_t group_idx = block / fs->blocks_per_group;
    // Which sub_bitmap it belongs to ?
    uint32_t sub_bitmap_idx = (block - (fs->blocks_per_group * group_idx)) / 4;
    // Index in sub_bitmap ?
    uint32_t idx = (block - (fs->blocks_per_group * group_idx)) % 4;

    uint32_t bitmap_block = fs->BGD[group_idx].block_bitmap_addr;
    read_disk_block(bitmap_block, (void*)buf);

    // Mask out that inode and write back the bitmap
    uint32_t mask = ~(0x1 << idx);
    buf[sub_bitmap_idx] = buf[sub_bitmap_idx] & mask;

    write_disk_block(bitmap_block, (void*)buf);

    // update free_inodes
    fs->BGD[group_idx].n_unalloc_blocks++;
    write_bgd();
exit:
    pop_trace();
}

fs_tree *get_ext2_root(inode_base_t * inode)
{
    push_trace("FILE *get_ext2_root(inode_base_t*)","get_ext2_root",inode,0,0,0);
    fs_tree *ext2root = NULL;
    new(ext2root, 1, fs_tree);
    new(ext2root->name, 2, char);
    new(ext2root->type, 1, uint8_t);
    new(ext2root->entries, 1, fs_entry *);
    strncpy(ext2root->name, "/", 1);
    ext2root->inode = 2;
    ext2root->hash = hash32(ext2root->name);
    ext2root->n_entries = 0;

    for (int i = 0; i < 12; i++) {
        printk(CYAN("INODE BLOCK[%d]: %x\n"), i, inode->blocks[i]);
    }

    uint32_t __attribute__((__packed__, aligned(4))) test[512/4];
    fs->dev->read((uint8_t *)test, 1, fs->start_block + (inode->blocks[0]*8));

    dirent_t root;
    root.inode_number = ((dirent_t *)test)[0].inode_number;
    root.size = ((dirent_t *)test)[0].size;
    root.name_len = ((dirent_t *)test)[0].name_len;
    root.type = ((dirent_t *)test)[0].type;
    new(root.name,root.name_len + 1,char);

    for (int i = 0; i < root.name_len; i++) {
        root.name[i] = (char)(uint64_t)(((dirent_t *)test)[0].name + i);
    }

    printk(CYAN("ROOT: inode: %u\n"), root.inode_number);
    printk(CYAN("ROOT: size: %u\n"), root.size & 0xffff);
    printk(CYAN("ROOT: name_len: %2x\n"),root.name_len & 0xff);
    printk(CYAN("ROOT: type: %2x\n"),root.type & 0xff);
    printk(CYAN("ROOT: name: %s\n"), root.name);

exit:
    pop_trace();
    return ext2root;
}

void print_fs(void)
{
    push_trace("void print_fs(void)","print_fs",0,0,0,0);
    printk("SUPERBLOCK: \n");
    printk("\tNumber of Inodes: %d\n", fs->superblock->n_inodes);
    printk("\tNumber of Blocks: %d\n", fs->superblock->n_blocks);
    printk("\tSignature: %x\n", fs->superblock->signature);

    printk("Block Size:       %d\n", fs->block_size);
    printk("Blocks per Group: %d\n", fs->blocks_per_group);
    printk("Inodes per Group: %d\n", fs->inodes_per_group);
    printk("Number of Groups: %d\n", fs->n_groups);
    pop_trace();
}

void scan_superblock(uint32_t super_sector)
{
    push_trace("void scan_superblock(uint32_t)","scan_superblock",super_sector,0,0,0);
    superblock_t s;
    s.signature = 0;

    sd_read((uint8_t *)&s, 1, super_sector+2);
    if ((s.signature != 0xef53)) {
        printk(RED("EXT2: Superblock invalid or not found: %8x\n"), s.signature);
    }
    printk("\t\tSIG: %x\n", s.signature);
    pop_trace();
}

int initial_fs(struct block_device *dev)
{
    push_trace("void initial_fs(struct block_device)","initial_fs",dev,0,0,0);
    int ret = E_NOERR;
    new(fs, 1, EXT2_t);
    new(fs->pwd, 2, char);

    fs->pwd[0] = '/';
    fs->pwd[1] = '\0';
    fs->dev = dev;
    

exit:
    pop_trace();
    return ret;
}

int assign_superblock(uint32_t super_sector)
{
    push_trace("int assign_sb(uint32_t)","assign_sb",super_sector,0,0,0);
    int ret = E_NOERR;

    fs->superblock = NULL;
    new(fs->superblock, 1, superblock_t);

    //scan_superblock(super_sector);

    fs->start_block = super_sector;
    
    fs->dev->read((uint8_t *)fs->superblock, 2, fs->start_block+2);

    if ((fs->superblock->signature != 0xef53)) {
        printk(RED("EXT2: Superblock invalid or not found: %8x\n"), fs->superblock->signature);
    }
 
    fs->block_size = (1024 << fs->superblock->block_shift);
    fs->blocks_per_group = fs->superblock->n_block_in_group;
    fs->inodes_per_group = fs->superblock->n_inode_in_group;

    fs->n_groups = 1 + (fs->superblock->n_blocks-1) / fs->blocks_per_group;

    printk(CYAN("Block Size:            %10d\n"), fs->block_size);
    printk(CYAN("Inode Size:            %10d\n"), fs->superblock->inode_size);
    printk(CYAN("Number of groups:      %10d\n"), fs->n_groups);
    printk(CYAN("Size of BGD:           %10d\n"), sizeof(block_group_descriptor_t));
    printk(CYAN("BGDs per block:        %10d\n"), \
    fs->block_size / sizeof(block_group_descriptor_t));
    printk(CYAN("Number of block:       %10d\n"), \
    fs->n_groups / (fs->block_size / sizeof(block_group_descriptor_t)));

exit:
    pop_trace();
    return ret;
}

int read_BGD(void)
{
    push_trace("int read_BGD(void)","read_BGD",0,0,0,0);
    int ret = E_NOERR;

    fs->BGD = NULL;

    printk(YELLOW("Size of BGD: %d\n"), sizeof(block_group_descriptor_t));

    uint32_t n_sectors = ((sizeof(block_group_descriptor_t) * fs->n_groups) / 512) + 1;
    uint32_t bgd_block = ((2048) / fs->block_size) + 1;
    uint32_t n_blocks = (fs->n_groups * sizeof(block_group_descriptor_t) / fs->block_size) + 1;
    uint32_t bgd_sector = bgd_block * 8;

    //volatile uint32_t __attribute__((__packed__, aligned(4))) buf[512/4];

    uint8_t *tmp;

    new(tmp, n_blocks * fs->block_size, uint8_t);

    fs->BGD = (block_group_descriptor_t *)tmp;

    for (int i = 0; i < n_blocks; i++) {
        fs->dev->read(((uint8_t *)fs->BGD + fs->block_size*i), fs->block_size / 512, \
        fs->start_block + (bgd_block + i)*8);
    }

    //fs->dev->read((uint8_t *)fs->BGD, n_sectors, fs->start_block + bgd_sector);

    printk(CYAN("BGD INODE TABLE: %d\n"),\
    fs->BGD[0].inode_table_addr);
    volatile uint32_t __attribute__((__packed__, aligned(4))) test[2*512/4];
    fs->dev->read((uint8_t *)test, 2, fs->start_block + 8);
    block_group_descriptor_t test_bgd = *((block_group_descriptor_t *)test);
    
    printk(YELLOW("TEST INODE TABLE: %d\n"),\
    test_bgd.inode_table_addr);

exit:
    pop_trace();
    return ret;
}
/*
int prepare_root(void)
{
    push_trace("int prepare_root(void)","prepare_root",0,0,0,0);
    int ret = E_NOERR;
    inode_base_t *root_inode = NULL;
    
    new(root_inode, 1, inode_base_t);
    read_inode_metadata(root_inode, 2);
    printk(CYAN("Inode: size: %ld\n"), (int64_t)(((int64_t)(root_inode->f_size_dir_acl <<\
    32)) | root_inode->size_lower));
    printk(CYAN("Inode: perms: %4x\n"), root_inode->inode_type_perm & 0xffff);

    printk(YELLOW("EXT2: Mounting root\n"));
    ret = file_mount("/", get_ext2_root(root_inode));
    printk(YELLOW("EXT2: Mounted root\n"));

exit:
    pop_trace();
    return ret;
}
*/

int print_dirent(dirent_t *dir)
{
    int ret = 0;

    printk(YELLOW("------ INODE %8d ------\n"), dir->inode_number);
    printk("INODE SIZE: %8d\n", dir->size);
    printk("INODE NAME LEN: %4d\n", dir->name_len);
    printk("INODE TYPE: %2x\n", dir->type);
    printk("INODE NAME:");
    for (int i = 0; i < dir->name_len; i++) {
        printk("%c", ((char *)(&dir->name))[i]);
    }
    printk("\n" YELLOW("-------- INODE END --------\n"));

    return ret;
}

// Recurrent dirent reading function which
int populate_recur(fs_tree *start)
{
    int ret = E_NOERR;
    uint32_t cur_offset = 0;
    char cur_name[256];
    dirent_t *tmp_dir;

    inode_base_t *entry_inode = NULL;
    inode_base_t *cur_inode = NULL;
    uint8_t *raw_block = NULL;

    // Create Inode structure for start directory
    new(cur_inode, 1, inode_base_t);
    new(entry_inode, 1, inode_base_t);

    // Read metadata from start directory
    read_inode_metadata(cur_inode, start->inode);
    printk(YELLOW("INDIRECT BLOCK POINTER: %d\n"), cur_inode->blocks[0]);
   
    int n_blocks = (cur_inode->n_sectors/8) + 1;

    new(raw_block, fs->block_size*n_blocks, uint8_t);

    for (int i = 0; (i < n_blocks) & (i < 12); i++) {
        // Create and read initial inode file blocks
        fs->dev->read(raw_block + fs->block_size*i, 8, fs->start_block + (cur_inode->blocks[i]*8));
    }

    // Handle indirect blocks
    if (n_blocks > 12) {
       // Ignored for now
       // Who has more than 12 blocks worth of dirs in one dir
    }

/*
    if (cur_inode != NULL) {
        printk(YELLOW("Culling cur_inode\n"));
        del(cur_inode);
    }
*/
    // Read all dirents in inode data block
    do {
        // Get dirent at offset, offset is dynamic and depends on
        // A field in the dirent
        tmp_dir = ((dirent_t *)((uint64_t)raw_block + cur_offset));
        if (tmp_dir->type == 0) {break;}

        start->n_entries++;
        printk(YELLOW("Resizing types\n"));
        alt(start->type, start->n_entries, uint8_t);
        printk(YELLOW("Resizing entries\n"));
        alt(start->entries, start->n_entries, fs_entry *);
        for (int i = 0; i < tmp_dir->name_len; i++) {
            cur_name[i] = ((char *)(&tmp_dir->name))[i];
        }

        start->type[start->n_entries - 1] = tmp_dir->type;
        new(start->entries[start->n_entries - 1], 1, fs_entry);

        print_dirent(tmp_dir);

        switch (tmp_dir->type) {
            case FS_FILE:
                new(start->entries[start->n_entries - 1]->file, 1, FILE);
                FILE *tfile = start->entries[start->n_entries - 1]->file;
                strncpy(tfile->name, cur_name, tmp_dir->name_len);
                printk(CYAN("found file: %s\n"), tfile->name);
                tfile->hash = hash32(tfile->name);
                tfile->inode = tmp_dir->inode_number;
                read_inode_metadata(entry_inode, tfile->inode);
                tfile->size = entry_inode->size_lower;
                tfile->create_time = entry_inode->creation_time;
                tfile->access_time = entry_inode->last_access_time;
                tfile->modify_time = entry_inode->last_mod_time;
                tfile->flags = 0;
                tfile->nlink = entry_inode->n_hard_links;
                break;
            case FS_DIR:
                new(start->entries[start->n_entries - 1]->dir, 1, fs_tree);
                fs_tree *tdir = start->entries[start->n_entries - 1]->dir;
                new(tdir->name, tmp_dir->name_len + 1, char);
                memset(tdir->name, 0, tmp_dir->name_len + 1);
                new(tdir->type, 1, uint8_t);
                new(tdir->entries, 1, fs_entry *);
                strncpy(tdir->name, cur_name, tmp_dir->name_len);
                printk(CYAN("found dir: %s\n"), tdir->name);
                tdir->hash = hash32(tdir->name);
                tdir->inode = tmp_dir->inode_number;
                tdir->n_entries = 0;
                if ((strncmp(tdir->name, ".", 1) != 0) & (strncmp(tdir->name, "..", 1) != 0)) {
                    printk(YELLOW("RECURSING\n"));
                    ret = populate_recur(tdir);
                    if (ret != E_NOERR) {
                        printk(RED("VFS: failed to populate in recurrence %x\n"), ret);
                        goto exit;
                    }
                }
                break;
            default:
                start->entries[start->n_entries - 1] = NULL;
                printk(YELLOW("Unknown file type %d: placing dummy\n"), tmp_dir->type);
                break;
        }
        printk(CYAN("Next dirent: %d ahead\n"), tmp_dir->size);
        cur_offset += tmp_dir->size;
    } while(tmp_dir->type);
    
exit:
/*
    if (raw_block != NULL) {
        printk(YELLOW("Culling raw_block\n"));
        del(raw_block);
    }
*/
    return ret;
}

int populate_fs(void)
{
    int ret = 0;
    inode_base_t *root_inode = NULL;
    new(root_inode, 1 , inode_base_t);
    read_inode_metadata(root_inode, 2);
    root = get_ext2_root(root_inode);

    ret = populate_recur(root);

exit:
    return ret;
}
