#include "../../include/fs/fat32.h"
#include "../../include/drivers/sd.h"
#include "../../include/stdlib.h"

static lock_t Mutex;

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