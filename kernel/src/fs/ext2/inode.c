#include <fs/ext2/inode.h>
#include <fs/ext2/file.h>
#include <fs/ext2/ext2.h>
#include <fs/ext2/fstypes.h>

#include <drivers/sd.h>

#include <memory/malloc.h>

void read_inode_metadata(EXT2_t *fs, inode_base_t *inode, uint32_t index) {
    // Which group the inode lives in
    uint32_t group = index / fs->inodes_per_group;
    // The block that points to a table of all inodes
    uint32_t inode_table_block = fs->BGD[group].inode_table_addr;
    uint32_t idx_in_group = index - group * fs->inodes_per_group;
    // Which block does the inode live in ? You may wonder why inode is subtracted by 1 here, it's because inode number starts from 1, not 0.(inode of 0 means error)
    uint32_t block_offset = (idx_in_group - 1) * fs->superblock->inode_size / fs->block_size;
    // Offset within block
    uint32_t offset_in_block = (idx_in_group - 1) - block_offset * (fs->block_size / fs->superblock->inode_size);
    char *block_buf = NULL;
    new(block_buf, fs->block_size, char);

    read_disk_block(fs, inode_table_block + block_offset, block_buf);

    memcpy(inode, block_buf + offset_in_block * fs->superblock->inode_size, fs->superblock->inode_size);
exit:
    del(block_buf);
}

void write_inode_metadata(EXT2_t *fs, inode_base_t *inode, uint32_t index) {
    // Which group the inode lives in
    uint32_t group = index / fs->inodes_per_group;
    // The block that points to a table of all inodes
    uint32_t inode_table_block = fs->BGD[group].inode_table_addr;
    //uint32_t idx_in_group = inode_idx - group * ext2fs->inodes_per_group;
    // Which block does the inode live in ? You may wonder why inode is subtracted by 1 here, it's because inode number starts from 1, not 0.(inode of 0 means error)
    uint32_t block_offset = (index - 1) * fs->superblock->inode_size / fs->block_size;
    // Offset within block
    uint32_t offset_in_block = (index - 1) - block_offset * (fs->block_size / fs->superblock->inode_size);
    char *block_buf = NULL;
    new(block_buf, fs->block_size, char);
    read_disk_block(fs, inode_table_block + block_offset, block_buf);
    memcpy(block_buf + offset_in_block * fs->superblock->inode_size, inode, fs->superblock->inode_size);
    write_disk_block(fs, inode_table_block + block_offset, block_buf);
    del(block_buf);
exit:
}

uint32_t read_inode_filedata(EXT2_t *fs, inode_base_t *inode, uint32_t offset, uint32_t size, char *buf) {
    uint32_t end_offset = (inode->size_lower >= offset + size) ? (offset + size) : (inode->size_lower);
    // Convert the offset/size to some starting/end iblock numbers
    uint32_t start_block = offset / fs->block_size;
    uint32_t end_block = end_offset / fs->block_size;
    // What's the offset into the start block
    uint32_t start_off = offset % fs->block_size;
    // How much bytes to read for the end block
    uint32_t end_size = end_offset - end_block * fs->block_size;

    uint32_t i = start_block;
    uint32_t cur_off = 0;
    while(i <= end_block) {
        uint32_t left = 0, right = fs->block_size - 1;
        char *block_buf = read_inode_block(fs, inode, i);
        if(i == start_block)
            left = start_off;
        if(i == end_block)
            right = end_size - 1;
        memcpy(buf + cur_off, block_buf + left, (right - left + 1));
        cur_off = cur_off + (right - left + 1);
        del(block_buf);
        i++;
    }
exit:
    return end_offset - offset;
}

void write_inode_filedata(EXT2_t *fs, inode_base_t *inode, uint32_t index, uint32_t offset, uint32_t size, char *buf) {
    if(offset + size > inode->size_lower) {
        inode->size_lower = offset + size;
        write_inode_metadata(fs, inode, index);
    }
    // Writing inode filedata is similar to reading inode filedata
    uint32_t end_offset = (inode->size_lower >= offset + size) ? (offset + size) : (inode->size_lower);
    // Convert the offset/size to some starting/end iblock numbers
    uint32_t start_block = offset / fs->block_size;
    uint32_t end_block = end_offset / fs->block_size;
    // What's the offset into the start block
    uint32_t start_off = offset % fs->block_size;
    // How much bytes to read for the end block
    uint32_t end_size = end_offset - end_block * fs->block_size;

    uint32_t i = start_block;
    uint32_t cur_off = 0;
    while(i <= end_block) {
        uint32_t left = 0, right = fs->block_size;
        char *block_buf = read_inode_block(fs, inode, i);

        if(i == start_block)
            left = start_off;
        if(i == end_block)
            right = end_size - 1;
        memcpy(block_buf + left, buf + cur_off, (right - left + 1));
        cur_off = cur_off + (right - left + 1);
        write_inode_block(fs, inode, i, block_buf);
        del(block_buf);
        i++;
    }
exit:
}

char * read_inode_block(EXT2_t *fs, inode_base_t *inode, uint32_t iblock) {
    char *buf = NULL;
    new(buf, fs->block_size, char);
    // Get the actual disk block number of the iblock
    uint32_t disk_block = get_disk_block_number(fs, inode, iblock);
    // Then just read a disk block
    read_disk_block(fs, disk_block, buf);
exit:
    return buf;
}

void write_inode_block(EXT2_t *fs, inode_base_t *inode, uint32_t iblock, char *buf) {
    // Get the actual disk block number of the iblock
    uint32_t disk_block = get_disk_block_number(fs, inode, iblock);
    // Then just write a disk block
    write_disk_block(fs, disk_block, buf);
}
void read_disk_block(EXT2_t *fs, uint32_t block, char *buf) {
    // Simply call the hard disk/floppy/whatever driver to read two consecutive sectors
    fs->dev->read(fs->dev, (uint8_t *)buf, fs->block_size, fs->block_size * block);
}

void write_disk_block(EXT2_t * fs, uint32_t block, char * buf) {
    // Simply call the hard disk/floppy/whatever driver to write two consecutive sectors
    fs->dev->write(fs->dev, (uint8_t *)buf, fs->block_size, fs->block_size * block);
}

int alloc_inode_metadata_block(uint32_t *block_ptr, EXT2_t *fs, inode_base_t *inode, uint32_t index, char *buffer, unsigned int block_overwrite) {
    if(!(*block_ptr)) {
        unsigned int block_no = ext2_alloc_block(fs);
        if(!block_no) return 0;
        *block_ptr = block_no;
        if(buffer)
            write_disk_block(fs, block_overwrite, (void*)buffer);
        else
            write_inode_metadata(fs, inode, index);
        return 1;
    }
    return 0;
}

uint32_t get_disk_block_number(EXT2_t *fs, inode_base_t *inode, uint32_t inode_block) {
    unsigned int p = fs->block_size / 4, ret;
    int a, b, c, d, e, f, g;
    unsigned int *tmp = NULL;
    new(tmp, fs->block_size, unsigned int);
    // How many blocks are left except for direct blocks ?
    a = inode_block - 12;
    if(a < 0) {
        ret = inode->blocks[inode_block];
        goto exit;
    }
    b = a - p;
    if(b < 0) {
        read_disk_block(fs, inode->blocks[12], (void*)tmp);
        ret = tmp[a];
        goto exit;
    }
    c = b - p * p;
    if(c < 0) {
        c = b / p;
        d = b - c * p;
        read_disk_block(fs, inode->blocks[12 + 1], (void*)tmp);
        read_disk_block(fs, tmp[c], (void*)tmp);
        ret = tmp[d];
        goto exit;
    }
    d = c - p * p * p;
    if(d < 0) {
        e = c / (p * p);
        f = (c - e * p * p) / p;
        g = (c - e * p * p - f * p);
        read_disk_block(fs, inode->blocks[12 + 2], (void*)tmp);
        read_disk_block(fs, tmp[e], (void*)tmp);
        read_disk_block(fs, tmp[f], (void*)tmp);
        ret = tmp[g];
        goto exit;
    }

exit:
    cull(global_heap, tmp);
    return ret;
}

void set_disk_block_number(EXT2_t *fs, inode_base_t *inode, uint32_t index, uint32_t inode_block, uint32_t disk_block) {
    unsigned int p = fs->block_size / 4;
    int a, b, c, d, e, f, g;
    int iblock = inode_block;
    unsigned int *tmp = NULL;
    new(tmp, fs->block_size, unsigned int);

    a = iblock - 12;
    if(a <= 0) {
        inode->blocks[inode_block] = disk_block;
        goto exit;
    }
    b = a - p;
    if(b <= 0) {
        if(!alloc_inode_metadata_block(&(inode->blocks[12]), fs, inode, index, NULL, 0));
        read_disk_block(fs, inode->blocks[12], (void*)tmp);
        ((unsigned int*)tmp)[a] = disk_block;
        write_disk_block(fs, inode->blocks[12], (void*)tmp);
        tmp[a] = disk_block;
        goto exit;
    }
    c = b - p * p;
    if(c <= 0) {
        c = b / p;
        d = b - c * p;
        if(!alloc_inode_metadata_block(&(inode->blocks[12 + 1]), fs, inode, index, NULL, 0));
        read_disk_block(fs, inode->blocks[12 + 1], (void*)tmp);
        if(!alloc_inode_metadata_block(&(tmp[c]), fs, inode, index, (void*)tmp, inode->blocks[12 + 1]));
        unsigned int temp = tmp[c];
        read_disk_block(fs, temp, (void*)tmp);
        tmp[d] = disk_block;
        write_disk_block(fs, temp, (void*)tmp);
        goto exit;
    }
    d = c - p * p * p;
    if(d <= 0) {
        e = c / (p * p);
        f = (c - e * p * p) / p;
        g = (c - e * p * p - f * p);
        if(!alloc_inode_metadata_block(&(inode->blocks[12 + 2]), fs, inode, index, NULL, 0));
        read_disk_block(fs, inode->blocks[12 + 2], (void*)tmp);
        if(!alloc_inode_metadata_block(&(tmp[e]), fs, inode, index, (void*)tmp, inode->blocks[12 + 2]));
        unsigned int temp = tmp[e];
        read_disk_block(fs, tmp[e], (void*)tmp);
        if(!alloc_inode_metadata_block(&(tmp[f]), fs, inode, index, (void*)tmp, temp));
        temp = tmp[f];
        read_disk_block(fs, tmp[f], (void*)tmp);
        tmp[g] = disk_block;
        write_disk_block(fs, temp, (void*)tmp);
        goto exit;
    }
exit:
    del(tmp);
}

void alloc_inode_block(EXT2_t *fs, inode_base_t *inode, uint32_t index, uint32_t block) {
    uint32_t ret = ext2_alloc_block(fs);
    set_disk_block_number(fs, inode, index, block, ret);
    inode->n_sectors = (block + 1) * (fs->block_size / 512);
    write_inode_metadata(fs, inode, index);
}

void free_inode_block(EXT2_t *fs, inode_base_t *inode, uint32_t index, uint32_t block) {
    uint32_t ret = get_disk_block_number(fs, inode, block);
    ext2_free_block(fs, ret);
    set_disk_block_number(fs, inode, index, ret, 0);
    write_inode_metadata(fs, inode, index);
}

uint32_t alloc_inode(EXT2_t *fs) {
    uint32_t *buf = NULL;
    new(buf, fs->block_size, uint32_t);
    // Read the inode bitmap, find free inode, return its index
    for(uint32_t i = 0; i < fs->n_groups; i++) {
        if(!fs->BGD[i].n_unalloc_inodes)
            continue;

        uint32_t bitmap_block = fs->BGD[i].inode_bitmap_addr;
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
                    fs->BGD[i].n_unalloc_inodes--;
                    write_bgd(fs);
                    return i * fs->inodes_per_group + j * 32 + k;
                }
            }
        }
    }
exit:
    return (uint32_t)-1;
}

/*
 * Free an inode from inode bitmap
 * */
void free_inode(EXT2_t *fs, uint32_t inode) {
    uint32_t *buf = NULL;
    new(buf, fs->block_size, uint32_t);
    // Which group it belongs to ?
    uint32_t group_idx = inode / fs->inodes_per_group;
    // Which sub_bitmap it belongs to ?
    uint32_t sub_bitmap_idx = (inode - (fs->inodes_per_group * group_idx)) / 4;
    // Index in sub_bitmap ?
    uint32_t idx = (inode - (fs->inodes_per_group * group_idx)) % 4;

    uint32_t bitmap_block = fs->BGD[group_idx].inode_bitmap_addr;
    read_disk_block(fs, bitmap_block, (void*)buf);

    // Mask out that inode and write back the bitmap
    uint32_t mask = ~(0x1 << idx);
    buf[sub_bitmap_idx] = buf[sub_bitmap_idx] & mask;

    write_disk_block(fs, bitmap_block, (void*)buf);

    // update free_inodes
    fs->BGD[group_idx].n_unalloc_inodes++;
    write_bgd(fs);
exit:
}