#include <fs/ext2/ext2.h>
#include <fs/ext2/inode.h>
#include <fs/ext2/file.h>

#include <drivers/sd.h>

#include <memory/malloc.h>

#include <stdlib.h>

uint32_t start_block;

uint32_t ext2_get_file_size(FILE *file)
{
    inode_base_t inode;
    read_inode_metadata(file->fs, &inode, file->inode);
    return inode.size_lower;
}

void ext2_mkdir(FILE *file, char *name, uint16_t perm)
{
    EXT2_t *fs = file->fs;
    uint32_t index = alloc_inode(fs);
    inode_base_t *inode = NULL;
    new(inode, 1, inode_base_t);
    read_inode_metadata(fs, inode, index);
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

    alloc_inode_block(fs, inode, index, 0);
    write_inode_metadata(fs, inode, index);
    ext2_create_entry(file, name, index);

    inode_base_t *p_inode = NULL;
    new(p_inode, 1, inode_base_t);
    read_inode_metadata(fs, p_inode, file->inode);
    p_inode->n_hard_links++;
    write_inode_metadata(fs, p_inode, file->inode);
    write_bgd(fs);
exit:
}

void ext2_mkfile(FILE *file, char *name, uint16_t perm)
{
    EXT2_t *fs = file->fs;
    uint32_t index = alloc_inode(fs);
    inode_base_t *inode = NULL;
    new(inode, 1, inode_base_t);
    read_inode_metadata(fs, inode, index);
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

    alloc_inode_block(fs, inode, index, 0);
    write_inode_metadata(fs, inode, index);
    ext2_create_entry(file, name, index);

    inode_base_t *p_inode = NULL;
    new(p_inode, 1, inode_base_t);
    read_inode_metadata(fs, p_inode, file->inode);
    p_inode->n_hard_links++;
    write_inode_metadata(fs, p_inode, file->inode);
    write_bgd(fs);
exit:
}

void ext2_unlink(FILE *file, char *name)
{
    EXT2_t *fs = file->fs;
    ext2_remove_entry(file, name);

    inode_base_t *p_inode = NULL;
    new(p_inode, 1, inode_base_t);
    read_inode_metadata(fs, p_inode, file->inode);
    p_inode->n_hard_links--;
    write_inode_metadata(fs, p_inode, file->inode);
    write_bgd(fs);
exit:
}

char **ext2_listdir(FILE *file)
{
    char **ll = NULL;
    EXT2_t *fs = file->fs;
    inode_base_t *p_inode = NULL;
    new(p_inode, 1, inode_base_t);
    read_inode_metadata(fs, p_inode, file->inode);

    uint32_t cur_offset = 0;
    uint32_t block_offset = 0;
    uint32_t in_block_offset = 0;
    int size = 0;
    int cap = 10;

    new(ll, cap, char *);
    char *block_buf = read_inode_block(fs, p_inode, block_offset);

    while (cur_offset < p_inode->size_lower) {
        if (in_block_offset >= fs->block_size) {
            block_offset++;
            in_block_offset = 0;
            block_buf = read_inode_block(fs, p_inode, block_offset);
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
    return ll;
}

FILE *file_from_dirent(EXT2_t *fs, dirent_t *dir, inode_base_t *inode)
{
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
    return f;
}

FILE *ext2_finddir(FILE *file, char *name)
{
    EXT2_t *fs = file->fs;
    inode_base_t *p_inode = NULL;
    new(p_inode, 1, inode_base_t);
    read_inode_metadata(fs, p_inode, file->inode);
    uint32_t expected_size;
    uint32_t real_size;
    uint32_t cur_offset = 0;
    uint32_t block_offset = 0;
    uint32_t in_block_offset = 0;
    char *block_buf = read_inode_block(fs, p_inode, block_offset);
    
    while(cur_offset < p_inode->size_lower) {
        if (in_block_offset >= fs->block_size) {
            block_offset++;
            in_block_offset = 0;
            block_buf = read_inode_block(fs, p_inode, block_offset);
        }

        dirent_t *cur_dir = (dirent_t *)(block_buf + in_block_offset);
        char *temp = NULL;
        new(temp, cur_dir->name_len + 1, char);
        memcpy(temp, cur_dir->name, cur_dir->name_len);

        if (cur_dir->inode_number != 0 && !strcmp(temp, name)) {
            inode_base_t *inode = NULL;
            new(inode, 1, inode_base_t);
            read_inode_metadata(fs, inode, cur_dir->inode_number);
            return file_from_dirent(fs, cur_dir, inode);
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
    return NULL;
}

void ext2_create_entry(FILE *file, char *name, uint32_t inode)
{
    EXT2_t * fs = file->fs;
    inode_base_t * p_inode = NULL;
    new(p_inode, 1, inode_base_t);
    read_inode_metadata(fs, p_inode, file->inode);
    uint32_t cur_offset = 0;
    uint32_t block_offset = 0;
    uint32_t in_block_offset = 0;
    uint32_t found = 0;
    uint32_t entry_name_len = strlen(name);

    char * check = NULL;
    new(check, entry_name_len + 1, char);
    char * block_buf = read_inode_block(fs, p_inode, block_offset);
    
    while(cur_offset < p_inode->size_lower) {
        if(in_block_offset >= fs->block_size) {
            block_offset++;
            in_block_offset = 0;
            block_buf = read_inode_block(fs, p_inode, block_offset);
        }
        dirent_t * cur_dir = (dirent_t*)(block_buf + in_block_offset);
        if(cur_dir->name_len == entry_name_len) {
            memcpy(check, cur_dir->name, entry_name_len);
            if(cur_dir->inode_number != 0 && !strcmp(name, check)) {
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
            write_inode_block(fs, p_inode, block_offset, block_buf);
            // Then, append a new ending entry
            // Be careful about a spcial case here, if not enough space for ending entry, put it in the new block
            in_block_offset += cur_dir->size;
            if(in_block_offset >= fs->block_size) {
                 block_offset++;
                 in_block_offset = 0;
                 block_buf = read_inode_block(fs, p_inode, block_offset);
            }
            cur_dir = (dirent_t*)(block_buf + in_block_offset);
            memset(cur_dir, 0, sizeof(dirent_t));
            write_inode_block(fs, p_inode, block_offset, block_buf);
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
}

void ext2_remove_entry(FILE *file, char *name) {
    EXT2_t *fs = file->fs;
    inode_base_t * p_inode = NULL;
    new(p_inode, 1, inode_base_t);
    read_inode_metadata(fs, p_inode, file->inode);
    uint32_t cur_offset = 0;
    uint32_t block_offset = 0;
    uint32_t in_block_offset = 0;
    uint32_t entry_name_len = strlen(name);
    
    char * check = NULL;
    new(check, entry_name_len + 1, char);

    char * block_buf = read_inode_block(fs, p_inode, block_offset);
    // Note: It is required that no directory entry cross block boundary && each directory entry be aligned on 4-byte boundary
    while(cur_offset < p_inode->size_lower) {
        if(in_block_offset >= fs->block_size) {
            block_offset++;
            in_block_offset = 0;
            block_buf = read_inode_block(fs, p_inode, block_offset);
        }
        dirent_t * cur_dir = (dirent_t*)(block_buf + in_block_offset);
        if(cur_dir->name_len == entry_name_len) {
            memcpy(check, cur_dir->name, entry_name_len);
            if(cur_dir->inode_number != 0 && !strcmp(name, check)) {
                cur_dir->inode_number = 0;
                write_inode_block(fs, p_inode, block_offset, block_buf);
                return;
            }
        }
        uint32_t expected_size = ((sizeof(dirent_t) + cur_dir->name_len) & 0xfffffffc) + 0x4;
        uint32_t real_size = cur_dir->size;
        // Found the last entry
        if(real_size != expected_size)
            return;
        in_block_offset += cur_dir->size;
        cur_offset += cur_dir->size;
    }
exit:
}


void ext2_chmod(FILE *file, uint32_t mode) {
    EXT2_t *fs = file->fs;
    inode_base_t * inode = NULL;
    new(inode, 1, inode_base_t);
    read_inode_metadata(fs, inode, file->inode);
    inode->inode_type_perm = (inode->inode_type_perm & 0xFFFFF000) | mode;
    write_inode_metadata(fs, inode, file->inode);
exit:
}

uint32_t ext2_read(FILE *file, uint32_t offset, uint32_t size, char * buf) {
    // Extract the ext2 filesystem object and inode from vfs node
    EXT2_t *fs = file->fs;
    inode_base_t * inode = NULL;
    new(inode, 1, inode_base_t);
    read_inode_metadata(fs, inode, file->inode);
    read_inode_filedata(fs, inode, offset, size, buf);
exit:
    return size;
}

uint32_t ext2_write(FILE *file, uint32_t offset, uint32_t size, char * buf) {
    // Extract the ext2 filesystem object and inode from vfs node
    EXT2_t *fs = file->fs;
    inode_base_t * inode =  NULL;
    new(inode, 1, inode_base_t);
    read_inode_metadata(fs, inode, file->inode);
    write_inode_filedata(fs, inode, file->inode, offset, size, buf);
exit:
    return size;
}

void ext2_open(FILE *file, uint32_t flags) {
    EXT2_t *fs = file->fs;
    // Overwrite the file on open
    // Add flags for other modes and create them
    if (flags & 1) {
        inode_base_t * inode = NULL;
        new(inode, 1, inode_base_t);
        read_inode_metadata(fs, inode, file->inode);
        inode->size_lower = 0;
        write_inode_metadata(fs, inode, file->inode);
    }
exit:
}

void ext2_close(FILE *file) {
    return;
}

void write_bgd(EXT2_t *fs) {
    for(uint32_t i = 0; i < fs->bgd_blocks; i++)
        write_disk_block(fs, 2, (void*)fs->BGD + i * fs->block_size);
}

void write_superblock(EXT2_t *fs) {
    write_disk_block(fs, 1, (void*)fs->superblock);
}

uint32_t ext2_alloc_block(EXT2_t *fs) {
    uint32_t *buf = NULL;
    new(buf, fs->block_size, uint32_t);
    // Read the inode bitmap, find free inode, return its index
    for(uint32_t i = 0; i < fs->n_groups; i++) {
        if(!fs->BGD[i].n_unalloc_blocks)
            continue;

        uint32_t bitmap_block = fs->BGD[i].block_bitmap_addr;
        read_disk_block(fs, bitmap_block, (void*)buf);
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
                    write_disk_block(fs, bitmap_block, (void*)buf);
                    // update free_inodes
                    fs->BGD[i].n_unalloc_blocks--;
                    write_bgd(fs);
                    return i * fs->blocks_per_group + j * 32 + k;
                }
            }
        }
    }
exit:
    return (uint32_t)-1;
}

/*
 * Free block from the ext2 block bitmaps
 * */
void ext2_free_block(EXT2_t *fs, uint32_t block) {
    uint32_t *buf = NULL;
    new(buf, fs->block_size, uint32_t);
    // Which group it belongs to ?
    uint32_t group_idx = block / fs->blocks_per_group;
    // Which sub_bitmap it belongs to ?
    uint32_t sub_bitmap_idx = (block - (fs->blocks_per_group * group_idx)) / 4;
    // Index in sub_bitmap ?
    uint32_t idx = (block - (fs->blocks_per_group * group_idx)) % 4;

    uint32_t bitmap_block = fs->BGD[group_idx].block_bitmap_addr;
    read_disk_block(fs, bitmap_block, (void*)buf);

    // Mask out that inode and write back the bitmap
    uint32_t mask = ~(0x1 << idx);
    buf[sub_bitmap_idx] = buf[sub_bitmap_idx] & mask;

    write_disk_block(fs, bitmap_block, (void*)buf);

    // update free_inodes
    fs->BGD[group_idx].n_unalloc_blocks++;
    write_bgd(fs);
exit:
}

FILE *get_ext2_root(EXT2_t * fs, inode_base_t * inode) {
    FILE *ext2root = NULL;
    new(ext2root, 1, FILE);
    strncpy(ext2root->name, "/", 1);
    ext2root->fs = fs;
    ext2root->inode = 2;

    ext2root->access_time   = inode->last_access_time;
    ext2root->modify_time   = inode->last_mod_time;
    ext2root->create_time   = inode->creation_time;

    ext2root->flags |= 0;
exit:
    return ext2root;
}

void print_fs(EXT2_t *fs)
{
    printk("SUPERBLOCK: \n");
    printk("\tNumber of Inodes: %d\n", fs->superblock->n_inodes);
    printk("\tNumber of Blocks: %d\n", fs->superblock->n_blocks);
    printk("\tSignature: %d\n", fs->superblock->signature);

    printk("Block Size:       %d\n", fs->block_size);
    printk("Blocks per Group: %d\n", fs->blocks_per_group);
    printk("Inodes per Group: %d\n", fs->inodes_per_group);
    printk("Number of Groups: %d\n", fs->n_groups);
}

void scan_superblock(void)
{
    uint32_t temp_block = 0;
    superblock_t s;
    s.signature = 0;
    while ((s.signature != 0xef53) & (temp_block < 600000)) {
        sd_read((uint8_t *)&s, 512, temp_block);
        temp_block++;
    }
    sd_read((uint8_t *)&s, 512, 532480);
    printk("\t\tSIG: %x\n", s.signature);
    sd_read((uint8_t *)&s, 512, 532481);
    printk("\t\tSIG: %x\n", s.signature);
    start_block = temp_block;
}

void ext2_init(struct block_device *dev) {
    // First, we need to store some information about the ext2-formatted disk
    EXT2_t *fs = NULL;
    new(fs, 1, EXT2_t);
    fs->superblock = NULL;
    new(fs->superblock, 1, superblock_t);
    fs->dev = dev;

    scan_superblock();

    // Read supedisk_block from disk
    fs->dev->read(fs->dev, (uint8_t *)fs->superblock, 1024, start_block);
    // Determine some helpful vars
    fs->block_size = (1024 << fs->superblock->block_shift);
    fs->blocks_per_group = fs->superblock->n_block_in_group;
    fs->inodes_per_group = fs->superblock->n_inode_in_group;

    fs->n_groups = fs->superblock->n_blocks / fs->blocks_per_group;
    if(fs->blocks_per_group * fs->n_groups < fs->n_groups)
        fs->n_groups++;

    // Now that we know the total number of groups, we can read in the BGD(Block Group Descriptors), it's placed immediately after supedisk_block
    // But how many disk blocks the BGD take?
    fs->bgd_blocks = (fs->n_groups * sizeof(block_group_descriptor_t)) / fs->block_size;
    if(fs->bgd_blocks * fs->block_size < fs->n_groups * sizeof(block_group_descriptor_t))
        fs->bgd_blocks++;

    fs->BGD = NULL;
    new(fs->BGD, fs->bgd_blocks * fs->block_size, block_group_descriptor_t);
    for(uint32_t i = 0; i < fs->bgd_blocks; i++) {
        read_disk_block(fs, 2, (void*)fs->BGD + i * fs->block_size);
    }

    print_fs(fs);

    // Then, mount it onto the vfs tree
    inode_base_t *root_inode = NULL;
    new(root_inode, 1, inode_base_t);
    read_inode_metadata(fs, root_inode, 2);
    file_mount("/", get_ext2_root(fs, root_inode));
exit:
}