#include <fs/ext2/inode.h>
#include <fs/ext2/file.h>
#include <fs/ext2/ext2.h>
#include <fs/ext2/fstypes.h>
#include <trace/strace.h>

#include <drivers/sd.h>

#include <memory/malloc.h>

void read_inode_metadata(inode_base_t *inode, uint32_t index) {
    push_trace("void read_inode_meta(inode_base_t*,uint32_t)","read_inode_meta",inode,index,0,0);
    // Which group the inode lives in
    uint32_t group = (index - 1) / fs->inodes_per_group;
    // The block that points to a table of all inodes
    uint32_t inode_table_block = fs->BGD[group].inode_table_addr;
 
    printk("n_dir: %d\n",fs->BGD[group].n_dir);

    uint32_t idx = (index - 1) % fs->inodes_per_group;
    uint32_t block_offset = (idx * fs->superblock->inode_size) / fs->block_size;
    uint32_t offset_in_block = (idx * fs->superblock->inode_size) % fs->block_size;
    uint32_t block_num = inode_table_block + block_offset;

    uint32_t n_sectors = (fs->superblock->inode_size / 512) + 1;

    printk(CYAN("READING INODE:     %d\n"), index);
    printk(CYAN("group:             %d\n"), group);
    printk(CYAN("table_block:       %d\n"), inode_table_block);
    printk(CYAN("idx:               %d\n"), idx);
    printk(CYAN("block_offset:      %d\n"), block_offset);
    printk(CYAN("offset_in_block:   %d\n"), offset_in_block);
    printk(CYAN("block_num:         %d\n"), block_num);
    printk(CYAN("n_sectors:         %d\n"), n_sectors);

    uint16_t *block_buf = NULL;
    if (n_sectors > 0) {
        new(block_buf, n_sectors * 512 / 2, uint16_t);
    
        fs->dev->read((uint8_t *)block_buf, n_sectors, fs->start_block + block_num * 8);

        memcpy(inode, &block_buf[offset_in_block / 2], fs->superblock->inode_size);
    }

exit:
    if (block_buf) {
        printk(YELLOW("Culling block_buf\n"));
        del(block_buf);
    }
    pop_trace();
}

void write_inode_metadata(inode_base_t *inode, uint32_t index) {
    push_trace("void write_inode_meta(inode_base_t*,uint32_t)","write_inode_meta",inode,index,0,0);
    // Which group the inode lives in
    uint32_t group = (index - 1) / fs->inodes_per_group;
    // The block that points to a table of all inodes
    uint32_t inode_table_block = fs->BGD[group].inode_table_addr;
  
    uint32_t idx = (index - 1) % fs->inodes_per_group;
    uint32_t block_offset = (index * fs->superblock->inode_size) / fs->block_size;
    uint32_t offset_in_block = (index * fs->superblock->inode_size) % fs->block_size;
    uint32_t block_num = inode_table_block + block_offset;

    uint32_t n_sectors = fs->superblock->inode_size / (512 + 1);

    uint16_t *block_buf = NULL;
    new(block_buf, n_sectors * 512 / 2, uint16_t);

    fs->dev->read((uint8_t *)block_buf, n_sectors, fs->start_block + block_num * 8);
    
    memcpy(&block_buf[offset_in_block / 2], inode, fs->superblock->inode_size);

    fs->dev->write((uint8_t *)block_buf, n_sectors, fs->start_block + block_num * 8);

exit:
    pop_trace();
    del(block_buf);
}

uint32_t read_inode_filedata(inode_base_t *inode, uint32_t offset, uint32_t size, char *buf) {
    push_trace("uint32_t read_inode_file(inode_base_t*,uint32_t,uint32_t,char*)","read_inode_file",inode,offset,size,buf);
    uint32_t end_offset = (inode->size_lower >= offset + size) ? (offset + size) : (inode->size_lower);
    // Convert the offset/size to some starting/end iblock numbers
    uint32_t start_block = offset / fs->block_size;
    uint32_t end_block = end_offset / fs->block_size;
    // What's the offset into the start block
    uint32_t start_off = offset % fs->block_size;
    // How much bytes to read for the end block
    uint32_t end_size = end_offset - end_block * fs->block_size;

    printk("start_off: %d, end_off: %d\n", start_off, end_size);

    uint32_t i = start_block;
    uint32_t cur_off = 0;
    while(i <= end_block) {
        uint32_t left = 0, right = fs->block_size - 1;
        char *block_buf = read_inode_block(inode, i);
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
    pop_trace();
    return end_offset - offset;
}

void write_inode_filedata(inode_base_t *inode, uint32_t index, uint32_t offset, uint32_t size, char *buf) {
    push_trace("void write_inode_file(inode_base_t*,uint32_t,uint32_t,uint32_t)","write_inode_file",inode,index,offset,size);
    if(offset + size > inode->size_lower) {
        inode->size_lower = offset + size;
        write_inode_metadata(inode, index);
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
        char *block_buf = read_inode_block(inode, i);

        if(i == start_block)
            left = start_off;
        if(i == end_block)
            right = end_size - 1;
        memcpy(block_buf + left, buf + cur_off, (right - left + 1));
        cur_off = cur_off + (right - left + 1);
        write_inode_block(inode, i, block_buf);
        del(block_buf);
        i++;
    }
exit:
    pop_trace();
}

char * read_inode_block(inode_base_t *inode, uint32_t iblock) {
    push_trace("char *read_inode_block(inode_base_t*,uint32_t)","read_inode_block",inode,iblock,0,0);
    char *buf = NULL;
    new(buf, fs->block_size, char);
    // Get the actual disk block number of the iblock
    uint32_t disk_block = get_disk_block_number(inode, iblock);
    // Then just read a disk block
    read_disk_block(disk_block, buf);
exit:
    pop_trace();
    return buf;
}

void write_inode_block(inode_base_t *inode, uint32_t iblock, char *buf) {
    push_trace("void write_inode_block(inode_base_t,uint32_t,char*)","write_inode_block",inode,iblock,buf,0);
    // Get the actual disk block number of the iblock
    uint32_t disk_block = get_disk_block_number(inode, iblock);
    // Then just write a disk block
    write_disk_block(disk_block, buf);
    pop_trace();
}
void read_disk_block(uint32_t block, char *buf) {
    push_trace("void read_disk_block(uint32_t,char*)","read_disk_block",block,buf,0,0);
    // Simply call the hard disk/floppy/whatever driver to read two consecutive sectors
    fs->dev->read((uint8_t *)buf, fs->block_size / 512, fs->start_block + 8 * block);
    pop_trace();
}

void write_disk_block(uint32_t block, char * buf) {
    push_trace("void write_disk_block(uint32_t,char*)","write_disk_block",block,buf,0,0);
    // Simply call the hard disk/floppy/whatever driver to write two consecutive sectors
    fs->dev->write((uint8_t *)buf, fs->block_size / 512, fs->start_block + 8 * block);
    pop_trace();
}

int alloc_inode_metadata_block(uint32_t *block_ptr, inode_base_t *inode, uint32_t index, char *buffer, unsigned int block_overwrite) {
    if(!(*block_ptr)) {
        unsigned int block_no = ext2_alloc_block();
        if(!block_no) return 0;
        *block_ptr = block_no;
        if(buffer)
            write_disk_block(block_overwrite, (void*)buffer);
        else
            write_inode_metadata(inode, index);
        return 1;
    }
    return 0;
}

uint32_t get_disk_block_number(inode_base_t *inode, uint32_t inode_block) {
    unsigned int p = fs->block_size / 4, ret = E_NOERR;
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
        read_disk_block(inode->blocks[12], (void*)tmp);
        ret = tmp[a];
        goto exit;
    }
    c = b - p * p;
    if(c < 0) {
        c = b / p;
        d = b - c * p;
        read_disk_block(inode->blocks[12 + 1], (void*)tmp);
        read_disk_block(tmp[c], (void*)tmp);
        ret = tmp[d];
        goto exit;
    }
    d = c - p * p * p;
    if(d < 0) {
        e = c / (p * p);
        f = (c - e * p * p) / p;
        g = (c - e * p * p - f * p);
        read_disk_block(inode->blocks[12 + 2], (void*)tmp);
        read_disk_block(tmp[e], (void*)tmp);
        read_disk_block(tmp[f], (void*)tmp);
        ret = tmp[g];
        goto exit;
    }

exit:
    cull(global_heap, tmp);
    return ret;
}

void set_disk_block_number(inode_base_t *inode, uint32_t index, uint32_t inode_block, uint32_t disk_block) {
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
        if(!alloc_inode_metadata_block((inode->blocks + 12), inode, index, NULL, 0));
        read_disk_block(inode->blocks[12], (void*)tmp);
        ((unsigned int*)tmp)[a] = disk_block;
        write_disk_block(inode->blocks[12], (void*)tmp);
        tmp[a] = disk_block;
        goto exit;
    }
    c = b - p * p;
    if(c <= 0) {
        c = b / p;
        d = b - c * p;
        if(!alloc_inode_metadata_block((inode->blocks + 12 + 1), inode, index, NULL, 0));
        read_disk_block(inode->blocks[12 + 1], (void*)tmp);
        if(!alloc_inode_metadata_block(&(tmp[c]), inode, index, (void*)tmp, inode->blocks[12 + 1]));
        unsigned int temp = tmp[c];
        read_disk_block(temp, (void*)tmp);
        tmp[d] = disk_block;
        write_disk_block(temp, (void*)tmp);
        goto exit;
    }
    d = c - p * p * p;
    if(d <= 0) {
        e = c / (p * p);
        f = (c - e * p * p) / p;
        g = (c - e * p * p - f * p);
        if(!alloc_inode_metadata_block((inode->blocks + 12 + 2), inode, index, NULL, 0));
        read_disk_block(inode->blocks[12 + 2], (void*)tmp);
        if(!alloc_inode_metadata_block(&(tmp[e]), inode, index, (void*)tmp, inode->blocks[12 + 2]));
        unsigned int temp = tmp[e];
        read_disk_block(tmp[e], (void*)tmp);
        if(!alloc_inode_metadata_block(&(tmp[f]), inode, index, (void*)tmp, temp));
        temp = tmp[f];
        read_disk_block(tmp[f], (void*)tmp);
        tmp[g] = disk_block;
        write_disk_block(temp, (void*)tmp);
        goto exit;
    }
exit:
    del(tmp);
}

void alloc_inode_block(inode_base_t *inode, uint32_t index, uint32_t block) {
    uint32_t ret = ext2_alloc_block();
    set_disk_block_number(inode, index, block, ret);
    inode->n_sectors = (block + 1) * (fs->block_size / 512);
    write_inode_metadata(inode, index);
}

void free_inode_block(inode_base_t *inode, uint32_t index, uint32_t block) {
    uint32_t ret = get_disk_block_number(inode, block);
    ext2_free_block(ret);
    set_disk_block_number(inode, index, ret, 0);
    write_inode_metadata(inode, index);
}

uint32_t alloc_inode(void) {
    uint32_t *buf = NULL;
    new(buf, fs->block_size, uint32_t);
    // Read the inode bitmap, find free inode, return its index
    for(uint32_t i = 0; i < fs->n_groups; i++) {
        if(!fs->BGD[i].n_unalloc_inodes)
            continue;

        uint32_t bitmap_block = fs->BGD[i].inode_bitmap_addr;
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
                    fs->BGD[i].n_unalloc_inodes--;
                    write_bgd();
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
void free_inode(uint32_t inode) {
    uint32_t *buf = NULL;
    new(buf, fs->block_size, uint32_t);
    // Which group it belongs to ?
    uint32_t group_idx = inode / fs->inodes_per_group;
    // Which sub_bitmap it belongs to ?
    uint32_t sub_bitmap_idx = (inode - (fs->inodes_per_group * group_idx)) / 4;
    // Index in sub_bitmap ?
    uint32_t idx = (inode - (fs->inodes_per_group * group_idx)) % 4;

    uint32_t bitmap_block = fs->BGD[group_idx].inode_bitmap_addr;
    read_disk_block(bitmap_block, (void*)buf);

    // Mask out that inode and write back the bitmap
    uint32_t mask = ~(0x1 << idx);
    buf[sub_bitmap_idx] = buf[sub_bitmap_idx] & mask;

    write_disk_block(bitmap_block, (void*)buf);

    // update free_inodes
    fs->BGD[group_idx].n_unalloc_inodes++;
    write_bgd();
exit:
}
