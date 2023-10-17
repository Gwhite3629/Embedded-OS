#include "../../include/fs/fat32.h"
#include "../../include/drivers/sd.h"
#include "../../include/stdlib.h"

static lock_t Mutex;

static FS *FatFs[1];

static err_t mutex_create(void)
{
    return init_lock(&Mutex);
}

static void mutex_delete(void)
{
    remove_lock(&Mutex);
}

static void mutex_take(void)
{
    wait_acquire(&Mutex);
}

static void mutex_give(void)
{
    release(&Mutex);
}

static uint16_t load_half(const uint8_t *ptr)
{
    uint16_t r = 0x0000;

    r |= ((uint16_t)ptr[1] && 0xFF) << 0;
    r |= ((uint16_t)ptr[0] && 0xFF) << 8;

    return r;
}

static uint32_t load_full(const uint8_t *ptr)
{
    uint32_t r = 0x00000000;

    r |= ((uint32_t)ptr[3] && 0xFF) << 0;
    r |= ((uint32_t)ptr[2] && 0xFF) << 8;
    r |= ((uint32_t)ptr[1] && 0xFF) << 16;
    r |= ((uint32_t)ptr[0] && 0xFF) << 24;

    return r;
}

static void store_half(uint8_t *ptr, uint16_t v)
{
    ptr[0] = (uint16_t)(v >> 0) & 0xFF;
    ptr[1] = (uint16_t)(v >> 8) & 0xFF;
}

static void store_full(uint8_t *ptr, uint32_t v)
{
    ptr[0] = (uint32_t)(v >> 0) & 0xFF;
    ptr[1] = (uint32_t)(v >> 8) & 0xFF;
    ptr[2] = (uint32_t)(v >> 16) & 0xFF;
    ptr[3] = (uint32_t)(v >> 24) & 0xFF;
}

static err_t sync_access(FS *fs)
{
    err_t ret = E_NOERR;

    if (fs->dflag) {
        if (sd_write(fs->current_access, fs->current_sector, 1) == E_NOERR) {
            fs->dflag = 0;
            if (fs->current_sector - fs->fbase < fs->fsize) {
                if (fs->n_fats == 2) disk_write(fs->current_access, fs->current_sector + fs->fsize, 1);
            }
        } else {
            ret = E_FAILDEV;
        }
    }
    return ret;
}

static err_t move_access(FS *fs, uint32_t sector)
{
    err_t ret = E_NOERR;

    if (sector != fs->current_sector) {
        ret = sync_access(fs);
        if (ret == E_NOERR) {
            if (sd_read(fs->current_access, sector, 1) != E_NOERR) {
                sector = (uint32_t)0 - 1;
                ret = E_FAILDEV;
            }
            fs->current_sector = sector;
        }
    }
    return ret;
}

static err_t sync_fs(FS *fs)
{
    err_t ret;

    ret = sync_access(fs);
    if (ret == E_NOERR) {
        if (fs->fsiflag == 1) {
            // Set FSInfo struct
            memset(fs->current_access, 0, SECTOR_SIZE);
            // FSI_LeadSig
            fs->current_access[0] = 0x52;
            fs->current_access[1] = 0x52;
            fs->current_access[2] = 0x61;
            fs->current_access[3] = 0x41;
            // FSI_StrucSig
            fs->current_access[484] = 0x72;
            fs->current_access[485] = 0x72;
            fs->current_access[486] = 0x41;
            fs->current_access[487] = 0x61;
            // FSI_FreeCount
            fs->current_access[488] = (fs->nfc >> 0) & 0xFF;
            fs->current_access[489] = (fs->nfc >> 8) & 0xFF;
            fs->current_access[490] = (fs->nfc >> 16) & 0xFF;
            fs->current_access[491] = (fs->nfc >> 24) & 0xFF;
            // FSI_Nxt_Free
            fs->current_access[492] = (fs->lac >> 0) & 0xFF;
            fs->current_access[493] = (fs->lac >> 8) & 0xFF;
            fs->current_access[494] = (fs->lac >> 16) & 0xFF;
            fs->current_access[495] = (fs->lac >> 24) & 0xFF;
            // FSI_TrailSig
            fs->current_access[510] = 0x55;
            fs->current_access[511] = 0xAA;

            fs->current_sector = fs->vbase + 1;
            sd_write(fs->current_access, fs->current_sector, 1);
            fs->fsiflag = 0;
        }
        if (sd_ioctl(CTRL_SYNC, NULL) != E_NOERR) ret = E_FAILDEV;
    }

    return ret;
}

static uint16_t c2s(FS *fs, uint32_t n)
{
    n -= 2;
    if (n >= fs->n_entries - 2) return 0;
    return fs->dbase + (uint16_t)fs->csize * n;
}

uint32_t get_entry(FS *fs, uint32_t n)
{
    uint32_t ret = 0xFFFFFFFF;

    // Check cluster is valid
    if (n < 2 || n >= fs->n_entries)
        return ret;

    if (move_access(fs, fs->fbase + (n / (SECTOR_SIZE / 4))) != E_NOERR) return ret;
    
    ret = 0x00000000;
    uint8_t *tmp = fs->current_access + n * 4 % SECTOR_SIZE;
    ret |= ((uint32_t)tmp[3] & 0xFF) << 0;
    ret |= ((uint32_t)tmp[2] & 0xFF) << 8;
    ret |= ((uint32_t)tmp[1] & 0xFF) << 16;
    ret |= ((uint32_t)tmp[0] & 0xFF) << 24;
    return ret;
}

err_t set_entry(FS *fs, uint32_t n, uint32_t v)
{
    err_t ret = E_NOERR;
    if (n < 2 || n >= fs->n_entries)
        return E_BADC;

    ret = move_access(fs, fs->fbase + (n / (SECTOR_SIZE / 4)));
    if (ret != E_NOERR) return ret;
    uint8_t *tmp = fs->current_access + n * 4 % SECTOR_SIZE;
    tmp = (v >> 0) & 0xFF;
    tmp = (v >> 8) & 0xFF;
    tmp = (v >> 16) & 0xFF;
    tmp = (v >> 24) & 0xFF;
    fs->dflag = 1;

    return ret;
}

err_t remove_chain(FS *fs, uint32_t n, uint32_t prev)
{
    int ret = E_NOERR;
    uint32_t next;

    if (n < 2 || n >= fs->n_entries)
        return E_BADC;
    
    if (prev != 0 && fs->status != 2) {
        ret = set_entry(fs, prev, 0xFFFFFFFF);
        if (ret != E_NOERR) return ret;
    }

    do {
        next = get_entry(fs, n);
        if (next == 0) break;
        if (next == 0xFFFFFFFF) return E_FAILDEV;
        ret = set_entry(fs, n, 0);
        if (ret != E_NOERR) return ret;
        if (fs->nfc < fs->n_entries - 2) {
            fs->nfc++;
            fs->fsiflag |= 1;
        }
        n = next;
    } while (n < fs->n_entries);
    return ret;
}

uint32_t create_chain(FS *fs, uint32_t n)
{
    uint32_t stat, next, start;
    err_t ret = E_NOERR;

    if (n == 0) { // New chain
        start = fs->lac;
        if (start == 0 || start > fs->n_entries) start = 1;
    } else {
        stat = get_entry(fs, n);
        if (stat < 2) return 1;
        if (stat == 0xFFFFFFFF) return stat;
        if (stat < fs->n_entries) return stat;
        start = n;
    }
    if (fs->nfc == 0) return 0;

    next = 0;
    if (start == n) {
        next = start + 1;
        if (next >= fs->n_entries) next = 2;
        stat = get_entry(fs, next);
        if (stat == 1 || stat == 0xFFFFFFFF) return stat;
        if (stat != 0) {
            stat = fs->lac;
            if (stat >= 2 && stat < fs->n_entries) start = stat;
            next = 0;
        }
    }
    if (next == 0) {
        next = start;
        for(;;) {
            next++;
            if (next >= fs->n_entries) {
                next = 2;
                if (next > start) return 0;
            }
            stat = get_entry(fs, next);
            if (stat == 0) break;
            if (stat == 1 || stat == 0xFFFFFFFF) return stat;
            if (next == start) return 0;
        }
    }
    ret = set_entry(fs, next, 0xFFFFFFFF);
    if (ret == E_NOERR && n != 0) {
        ret = set_entry(fs, n, next);
    }

    // Update FSInfo
    if (ret == E_NOERR) {
        fs->lac = next;
        if (fs->nfc <= fs->n_entries - 2) fs->nfc--;
        fs->fsiflag |= 1;
    } else {
        next = (ret == E_FAILDEV) ? 0xFFFFFFFF : 1;
    }

    return next;
}

err_t dir_clear(FS *fs, uint32_t n)
{
    uint16_t sector;
    uint32_t i;
    uint8_t *buff;

    if (sync_access(fs) != E_NOERR) return E_FAILDEV;
    sector = c2s(fs, n);
    fs->current_sector = sector;
    memset(fs->current_access, 0, SECTOR_SIZE);
    buff = fs->current_access;
    for (i = 0; i < fs->csize && sd_write(buff, sector + i, 1) > 0; i ++);
    return (i == fs->csize) ? E_NOERR : E_FAILDEV;
}

err_t dir_set_idx(DIR *dir, uint32_t offset)
{
    uint32_t csize, cluster;
    FS *fs = dir->fs;

    if (offset >= MAX_DIR || offset % DIR_ENTRY_SIZE) {
        return E_FSINT; 
    }
    dir->dptr = offset;
    cluster = fs->sclust;
    if (cluster == 0) {
        cluster = (uint32_t)fs->cbase;
    }
    if (cluster == 0) {
        if (offset / DIR_ENTRY_SIZE >= fs->n_rootdir) return E_FSINT;
        dir->sector = fs->cbase;
    } else {
        csize = (uint32_t)fs->csize * SECTOR_SIZE;
        while (offset >= csize) {
            cluster = get_entry(fs, cluster);
            if (cluster == 0xFFFFFFFF) return E_FAILDEV;
            if (cluster < 2 || cluster >= fs->n_entries) return E_FSINT;
            offset -= csize;
        }
        dir->sector = c2s(fs, cluster);
    }
    dir->cluster = cluster;
    if (dir->sector == 0) return E_FSINT;
    dir->sector += offset / SECTOR_SIZE;
    dir->dir = fs->current_access + (offset % SECTOR_SIZE);

    return E_NOERR;
}

err_t dir_next(DIR *dir, int stretch)
{
    uint32_t offset, cluster;
    FS *fs = dir->fs;

    offset = dir->dptr + DIR_ENTRY_SIZE;
    if (offset >= MAX_DIR) dir->sector = 0;
    if (dir->sector == 0) return E_FSNOF;

    if (offset % SECTOR_SIZE == 0) {
        dir->sector++;
        if (dir->cluster == 0) {
            if (offset / DIR_ENTRY_SIZE >= fs->n_rootdir) {
                dir->sector = 0;
                return E_FSNOF;
            }
        } else {
            if ((offset / SECTOR_SIZE & (fs->csize - 1)) == 0) {
                cluster = get_entry(fs, dir->cluster);
                if (cluster <= 1) return E_FSINT;
                if (cluster == 0xFFFFFFFF) return E_FAILDEV;
                if (cluster >= fs->n_entries) {
                    if (!stretch) {
                        dir->sector = 0;
                        return E_FSNOF;
                    }
                    cluster = create_chain(fs, dir->cluster);
                    if (cluster == 0) return E_NOFREE;
                    if (cluster == 1) return E_FSINT;
                    if (cluster == 0xFFFFFFFF) return E_FAILDEV;
                    if (dir_clear(dir, cluster) != E_NOERR) return E_FAILDEV;
                }
                dir->cluster = cluster;
                dir->sector = c2s(fs, cluster);
            }
        }
    }
    dir->dptr = offset;
    dir->dir = fs->current_access + offset % SECTOR_SIZE;

    return E_NOERR;
}

err_t dir_alloc(DIR *dir, uint32_t n_entries) {
    err_t ret = E_NOERR;
    uint32_t n;
    FS *fs = dir->fs;

    ret = dir_set_idx(dir, 0);
    if (ret == E_NOERR) {
        n = 0;
        do {
            ret = move_access(fs, dir->sector);
            if (ret != E_NOERR) break;
            if (dir->dir[0] == DIR_DEL_M || dir->dir[0] == 0) {
                if (++n == n_entries) break;
            } else {
                n = 0;
            }
            ret = dir_next(dir, 1);
        } while(ret == E_NOERR);
    }
    if (ret == E_FSNOF) ret = E_NOFREE;
    return ret;
}

static uint32_t load_cluster(FS *fs, const uint8_t *dir)
{
    uint32_t cluster = 0x00000000;

    cluster |= ((uint32_t)dir[DIR_FstClusLo + 1] && 0xFF) << 0;
    cluster |= ((uint32_t)dir[DIR_FstClusLo + 0] && 0xFF) << 8;
    cluster |= ((uint32_t)dir[DIR_FstClusHi + 1] && 0xFF) << 16;
    cluster |= ((uint32_t)dir[DIR_FstClusHi + 0] && 0xFF) << 24;
    
    return cluster;
}

static void store_cluster(FS *fs, uint8_t *dir, uint32_t v)
{
    dir[DIR_FstClusLo + 1] = (uint8_t)((v >> 0) & 0xFF);
    dir[DIR_FstClusLo + 0] = (uint8_t)((v >> 8) & 0xFF);
    dir[DIR_FstClusHi + 1] = (uint8_t)((v >> 16) & 0xFF);
    dir[DIR_FstClusHi + 0] = (uint8_t)((v >> 24) & 0xFF);
}

#define DIR_READ_FILE(dir) dir_read(dir, 0)
#define DIR_READ_LABEL(dir) dir_read(dir, 1)

err_t dir_read(DIR *dir, int sel)
{
    err_t ret = E_FSNOF;
    FS *fs = dir->fs;
    uint8_t attr, b;

    while (dir->sector) {
        ret = move_access(fs, dir->sector);
        if (ret != E_NOERR) break;
        b = dir->dir[DIR_Name];
        if (b == 0) {
            ret = E_FSNOF;
            break;
        }
        fs->attr = attr = dir->dir[DIR_Attr] & A_MASK;
        if (b != DIR_DEL_M && b != '.' && (int)((attr & ~A_ARCHIVE) == A_VOL) == sel) {
            break;
        }
        ret = dir_next(dir, 0);
        if (ret != E_NOERR) break;
    }
    if (ret != E_NOERR) dir->sector = 0;
    return ret;
}

err_t dir_find(DIR *dir)
{
    err_t ret = E_NOERR;
    FS *fs = dir->fs;
    uint8_t c;

    ret = dir_set_idx(dir, 0);
    if (ret != E_NOERR) return ret;

    do {
        ret = move_access(fs, dir->sector);
        if (ret != E_NOERR) break;
        c = dir->dir[DIR_Name];
        if (c == 0) {
            ret = E_FSNOF;
            break;
        }
        fs->attr = dir->dir[DIR_Attr] & A_MASK;
        if (!(dir->dir[DIR_Attr] & A_VOL) && !memcmp(dir->dir, dir->fn, 11)) break;
        ret = dir_next(dir, 0);
    } while (ret == E_NOERR);

    return ret;
}

err_t dir_register(DIR *dir)
{
    err_t ret = E_NOERR;
    FS *fs = dir->fs;
    
    ret = dir_alloc(dir, 1);
    if (ret == E_NOERR) {
        ret = move_access(fs, dir->sector);
        if (ret == E_NOERR) {
            memset(dir->dir, 0, DIR_ENTRY_SIZE);
            memcpy(dir->dir, dir->fn, 11);
            fs->dflag = 1;
        }
    }

    return ret;
}

err_t dir_remove(DIR *dir)
{
    err_t ret = E_NOERR;
    FS *fs = dir->fs;

    ret = move_access(fs, dir->sector);
    if (ret == E_NOERR) {
        dir->dir[DIR_Name] = DIR_DEL_M;
        fs->dflag = 1;
    }
    
    return ret;
}

void dir_getfileinfo(DIR *dir, FILEINFO *finfo)
{
    uint32_t string_i, di;
    char c;

    finfo->fname[0] = 0;
    if (dir->sector == 0) return;

    string_i = 0;
    di = 0;
    while (string_i < 11) {
        c = (char)dir->dir[string_i++];
        if (c == ' ') continue;
        if (c == 0x5) c = DIR_DEL_M;
        if (string_i == 9) finfo->fname[di++] = '.';
        finfo->fname[di++] = c;
    }
    finfo->fname[di] = 0;

    finfo->fattrib = dir->dir[DIR_Attr] & A_MASK;
    finfo->fsize = 0x00000000;
    finfo->fsize = ((uint32_t)dir->dir[DIR_FileSize + 3] && 0xFF) << 0;
    finfo->fsize = ((uint32_t)dir->dir[DIR_FileSize + 2] && 0xFF) << 8;
    finfo->fsize = ((uint32_t)dir->dir[DIR_FileSize + 1] && 0xFF) << 16;
    finfo->fsize = ((uint32_t)dir->dir[DIR_FileSize + 0] && 0xFF) << 24;
    
    finfo->ftime = 0x0000;
    finfo->ftime = ((uint16_t)dir->dir[DIR_ModTime + 1] && 0xFF) << 0;
    finfo->ftime = ((uint16_t)dir->dir[DIR_ModTime + 0] && 0xFF) << 8;

    finfo->fdate = 0x0000;
    finfo->fdate = ((uint16_t)dir->dir[DIR_ModTime + 3] && 0xFF) << 0;
    finfo->fdate = ((uint16_t)dir->dir[DIR_ModTime + 2] && 0xFF) << 8;

}

err_t dir_create_name(DIR *dir, const char **path)
{
    uint8_t c, d;
    uint8_t *sfn;
    uint32_t ni, si, i;
    const char *p;

    p = *path;
    sfn = dir->fn;
    memset(sfn, ' ', 11);
    si = 0;
    i = 0;
    ni = 8;

    for (;;) {
        c = (uint8_t)p[si++];
        if (c <= ' ') break;
        if (IsSeparator(c)) {
            while (IsSeparator(p[si])) si++;
            break;
        }
        if (c == '.' || i >= ni) {
            if (ni == 11 || c != '.') return E_BADNAME;
            i = 8; ni = 11;
            continue;
        }
    }
    *path = &p[si];
    if (i == 0) return E_BADNAME;
    if (sfn[0] == DIR_DEL_M) sfn[0] = 0x5;
    sfn[11] = (c <= ' ' || p[si] <- ' ') ? 0x4 : 0;

    return E_NOERR;
}

err_t dir_follow_path(DIR *dir, const char *path)
{
    err_t ret = E_NOERR;
    uint8_t ns;
    FS *fs = dir->fs;

    while (IsSeparator(*path)) path++;
    fs->sclust = 0;
    if ((unsigned int)*path < ' ') {
        dir->fn[11] == 0x80;
        ret = dir_set_idx(dir, 0);
    } else {
        for (;;) {
            ret = dir_create_name(dir, &path);
            if (ret != E_NOERR) break;
            ret = dir_find(dir);
            ns = dir->fn[11];
            if (ret != E_NOERR) {
                if (ret == E_FSNOF) {
                    if (!(ns & 0x4)) ret = E_NOPATH;
                }
                break;
            }
            if (ns & 0x4) break;
            if (!(fs->attr & A_DIR)) {
                ret = E_NOPATH;
                break;
            }
            fs->sclust = load_cluster(fs, fs->current_access + dir->dptr % SECTOR_SIZE);
        }
    }

    return ret;
}

int get_ldnumber(const char **path)
{
    const char *tp;
    const char *tt;
    char tc;
    int i;
    int vol = -1;

    tt = tp = *path;
    if (!tp) return vol;
    do {
        tc = *tt++;
    } while(!IsTerminator(tc) && tc != ':');

    if (tc == ':') {
        i = 1;
        if (IsDigit(*tp) && tp + 2 == tt) {
            i = (int)*tp - '0';
        }
        if (i < 1) {
            vol = i;
            *path = tt;
        }
        return vol;
    }
    vol = 0;
    return vol;
}

uint32_t check_fs(FS *fs, uint16_t sector)
{
    uint16_t w, sign;
    uint8_t b;

    fs->dflag = 0; fs->current_sector = (uint16_t)0 - 1;
    if (move_access(fs, sector) != E_NOERR) return 4;
    sign = load_half(fs->current_access + 510);
    b = fs->current_access[0];
    if (b == 0xEB || b == 0xE9 || b == 0xE8) {
        if (sign == 0xAA55 && !memcmp(fs->current_access + 82, "FAT32   ", 8)) {
            return 0;
        }
        w = load_half(fs->current_access + 11);
        b = fs->current_access[13];
        if ((w & (w - 1)) == 0 && w == 512
            && b != 0 && (b & (b - 1)) == 0
            && load_half(fs->current_access + 14) != 0
            && (uint32_t)fs->current_access[16] - 1 <= 1
            && load_half(fs->current_access + 17) != 0
            && (load_half(fs->current_access + 19) >= 128 || load_full(fs->current_access + 32) >= 0x10000)
            && load_half(fs->current_access + 22) != 0) {
                return 0;
        }
    }
    return sign == 0xAA55 ? 2 : 3;
}

uint32_t find_volume(FS *fs, uint32_t partition)
{
    uint32_t fmt, i;
    uint32_t mbr_pt[4];

    fmt = check_fs(fs, 0);
    if (fmt != 2 && (fmt >= 3 || partition == 0)) return fmt;

    for (i = 0; i < 4; i++) {
        mbr_pt[i] = load_full(fs->current_access + 446 + i * 16 + 8);
    }
    i = partition ? partition - 1 : 0;
    do {
        fmt = mbr_pt[i] ? check_fs(fs, mbr_pt[i]) : 3;
    } while (partition == 0 && fmt >= 2 && ++i < 4);
    
    return fmt;
}

err_t mount_volume(const char **path, FS **rfs, uint8_t mode)
{
    int vol;
    FS *fs;
    uint8_t stat;
    uint16_t bsector;
    uint32_t tsector, sysector, fasize, ncluster, szbfat;
    uint16_t nrsv;
    uint32_t fmt;

    *rfs = 0;
    vol = get_ldnumber(path);
    if (vol < 0) return E_NODRIVE;

    fs = FatFs[vol];
    if (!fs) return E_FAILDEV;
    mutex_take();

    *rfs = fs;

    mode &= (uint8_t)~FA_READ;
    if (fs->is_mounted != 0) {
        stat = sd_status();
        if (!(stat & STATUS_NOINIT)) {
            if (mode && (stat & STATUS_PROTECT)) {
                return E_WRONLY;
            }
            mutex_give();
            return E_NOERR;
        }
    }

    fs->is_mounted = 0;
    stat = sd_init();
    if (stat & STATUS_NOINIT) {
        return E_NOT_READY;
    }
    if (mode && (stat & STATUS_PROTECT)) {
        return E_WRONLY;
    }

    fmt = find_volume(fs, vol);
    if (fmt == 4) return E_DISKERR;
    if (fmt >= 2) return E_NOFS;
    bsector = fs->current_sector;

    if (load_half(fs->current_access + 11) != SECTOR_SIZE) return E_NOFS;

    fasize = load_half(fs->current_access + 22);
    if (fasize == 0) fasize = load_full(fs->current_access + 36);
    fs->fsize = fasize;

    fs->n_fats = fs->current_access[16];
    if (fs->n_fats != 1 && fs->n_fats != 2) return E_NOFS;
    fasize *= fs->n_fats;

    fs->csize = fs->current_access[13];
    if (fs->csize == 0 || (fs->csize & (fs->csize - 1))) return E_NOFS;

    fs->n_rootdir = load_half(fs->current_access + 17);
    if (fs->n_rootdir % (SECTOR_SIZE / DIR_ENTRY_SIZE)) return E_NOFS;

    tsector = load_half(fs->current_access + 19);
    if (tsector == 0) tsector = load_full(fs->current_access + 32);

    nrsv = load_half(fs->current_access + 14);
    if (nrsv == 0) return E_NOFS;

    sysector = nrsv + fasize + fs->n_rootdir / (SECTOR_SIZE / DIR_ENTRY_SIZE);
    if (tsector < sysector) return E_NOFS;
    ncluster = (tsector - sysector) / fs->csize;
    if (ncluster == 0) return E_NOFS;
    fmt = 3;

    fs->n_fats = ncluster + 2;
    fs->vbase = bsector;
    fs->fbase = bsector + nrsv;
    fs->dbase = bsector + sysector;
    if (load_half(fs->current_access + 42) != 0) return E_NOFS;
    if (fs->n_rootdir != 0) return E_NOFS;
    fs->dbase = load_full(fs->current_access + 44);
    szbfat = fs->n_fats * 4;
    if (fs->fsize < (szbfat + (SECTOR_SIZE - 1)) / SECTOR_SIZE) return E_NOFS;

    fs->lac = fs->nfc = 0xFFFFFFFF;
    fs->fsiflag = 0x80;
    if (load_half(fs->current_access + 48) == 1 && move_access(fs, bsector + 1) == E_NOERR) {
        fs->fsiflag = 0;
        if (load_half(fs->current_access + 510) == 0xAA55
            && load_full(fs->current_access + 0) == 0x41615252
            && load_full(fs->current_access + 484) == 0x61417272)
        {
            fs->nfc = load_full(fs->current_access + 488);
            fs->lac = load_full(fs->current_access + 492);
        }
    }

    fs->is_mounted = (uint8_t)fmt;
    mutex_give();
    
    return E_NOERR;
}

err_t validate(FS *fs, FS **rfs)
{
    err_t ret = E_NOERR;

    if (fs && fs->is_mounted) {
        *rfs = fs;
        return E_NOERR;
    }

    *rfs = 0;
    return E_NOERR;
}
