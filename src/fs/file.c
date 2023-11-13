#include "../../include/stdlib.h"
#include "../../include/fs/fat32.h"
#include "../../include/fs/file.h"
#include "../../include/drivers/sd.h"

extern lock_t Mutex;

extern FS *FatFs[1];

/* Mount/Unmount a logical drive */
err_t f_mount (FS* fs, const char* path, uint8_t opt)
{
    FS *cfs;
    int vol;
    err_t ret;
    const char *rp = path;

    vol = get_ldnumber(&rp);
    if (vol < 0)
        return E_NODRIVE;
    cfs = FatFs[vol];

    if (cfs) {
        FatFs[vol] = 0;
        mutex_delete();
        cfs->is_mounted = 0;
    }

    if (fs) {
        fs->pdrv = (uint8_t)vol;
        fs->ldrv = (uint8_t)vol;
        if (!mutex_create()) return E_FINT;
        fs->is_mounted = 0;
        FatFs[vol] = fs;
    }

    if (opt == 0) return E_NOERR;

    ret = mount_volume(&path, &fs, 0);
    return ret;
}

/* Open or create a file */
err_t f_open (FILE* fp, const char* path, uint8_t mode)
{
    err_t ret;
    DIR dj;
    FS *fs;
    uint32_t cl, bcs, cluster, tm;
    uint16_t sector;
    size_t ofs;

    if (!fp) return E_FAILFILE;

    mode &= FA_READ | FA_WRITE | FA_CREATE_ALWAYS | FA_CREATE_NEW | FA_OPEN_ALWAYS | FA_OPEN_APPEND;
    ret = mount_volume(&path, &fs, mode);
    if (ret == E_NOERR) {
        dj.fs = fs;
        ret = dir_follow_path(&dj, path);
        if (ret == E_NOERR) {
            if (dj.fn[11] & 0x80) {
                ret = E_INVALID_F;
            }
        }
        if (mode & (FA_CREATE_ALWAYS | FA_OPEN_ALWAYS | FA_CREATE_NEW)) {
            if (ret != E_NOERR) {
                if (ret == E_NOFILE) {
                    ret = dir_register(&dj);
                }
                mode |= FA_CREATE_ALWAYS;
            } else {
                if (dj.fs->attr & (A_RDONLY | A_DIR)) {
                    ret = E_FDENIED;
                } else {
                    if (mode & FA_CREATE_NEW) ret = E_FDENIED;
                }
            }
            if (ret == E_NOERR && (mode & FA_CREATE_ALWAYS)) {
                /* Set directory entry initial state */
				tm = get_fattime();					/* Set created time */
				store_full(dj.dir + DIR_CrtTime, tm);
				store_full(dj.dir + DIR_ModTime, tm);
				cl = load_cluster(fs, dj.dir);			/* Get current cluster chain */
				dj.dir[DIR_Attr] = A_ARCHIVE;			/* Reset attribute */
				store_cluster(fs, dj.dir, 0);			/* Reset file allocation info */
				store_full(dj.dir + DIR_FileSize, 0);
				fs->dflag = 1;
				if (cl != 0) {						/* Remove the cluster chain if exist */
					sector = fs->current_access;
					ret = remove_chain(&dj.fs, cl, 0);
					if (ret == E_NOERR) {
						ret = move_acess(fs, sector);
						fs->lac = cl - 1;		/* Reuse the cluster hole */
					}
				}
            }
        } else {
            if (ret == E_NOERR) {					/* Is the object exsiting? */
				if (dj.fs->attr & A_DIR) {		/* File open against a directory */
					ret = E_NOFILE;
				} else {
					if ((mode & FA_WRITE) && (dj.fs->attr & A_RDONLY)) { /* Write mode open against R/O file */
						ret = E_FDENIED;
					}
				}
			}
        }
        if (ret == E_NOERR) {
            if (mode & FA_CREATE_ALWAYS) mode |= FA_MODIFIED;	/* Set file change flag if created or overwritten */
			fp->dir_sector = fs->current_sector;			/* Pointer to the directory entry */
			fp->dir_ptr = dj.dir;
        }
        if (ret == E_NOERR) {
            fp->fs->sclust = load_cluster(fs, dj.dir);					/* Get object allocation info */
			fp->fs->objsize = load_full(dj.dir + DIR_FileSize);
			fp->fs = fs;	/* Validate the file object */
			fp->fs->id = fs->id;
			fp->flags = mode;	/* Set file access mode */
			fp->err = 0;		/* Clear error flag */
			fp->sector = 0;		/* Invalidate current data sector */
			fp->fptr = 0;		/* Set file pointer top of the file */
            memset(fp->buf, 0, sizeof fp->buf);	/* Clear sector buffer */
            if ((mode & FA_SEEKEND) && fp->fs->objsize > 0) {	/* Seek to end of file if FA_OPEN_APPEND is specified */
				fp->fptr = fp->fs->objsize;			/* Offset to seek */
				bcs = (uint32_t)fs->csize * SECTOR_SIZE;	/* Cluster size in byte */
				cluster = fp->fs->sclust;				/* Follow the cluster chain */
				for (ofs = fp->fs->objsize; ret == E_NOERR && ofs > bcs; ofs -= bcs) {
					cluster = get_entry(&fp->fs, cluster);
					if (cluster <= 1) ret = E_FINT;
					if (cluster == 0xFFFFFFFF) ret = E_DISKERR;
				}
				fp->cluster = cluster;
				if (ret == E_NOERR && ofs % SECTOR_SIZE) {	/* Fill sector buffer if not on the sector boundary */
					sector = c2s(fs, cluster);
					if (sector == 0) {
						ret = E_FINT;
					} else {
						fp->sector = sector + (uint32_t)(ofs / SECTOR_SIZE);
        				if (sd_read(fp->buf, fp->sector, 1) != E_NOERR)
                            ret = E_FINT;
                    }
                }
            }
        }
    }
    if (ret != E_NOERR)
        fp->fs = 0;	/* Invalidate file object on error */

	return ret;
}	

/* Close an open file object */
err_t f_close (FILE* fp)
{
    err_t ret;
    FS *fs;

    ret = f_sync(fp);
    if (ret == E_NOERR) {
        ret = validate(&fp->fs, &fs);
        if (ret == E_NOERR) {
            fp->fs = 0;
            mutex_give();
        }
    }
    return ret;
}

/* Flush cached data of the writing file */
err_t f_sync (FILE* fp)
{
    err_t ret;
    FS *fs;
    uint32_t tm;
    uint8_t *dir;

    ret = validate(&fp->fs, &fs);
    if (ret == E_NOERR) {
        if (fp->flags & FA_MODIFIED) {
            if (fp->flags * FA_DIRTY) {
                if (sd_write(fp->buf, fp->sector, 1) != E_NOERR)
                    return ret;
                fp->flags &= (uint8_t)~FA_DIRTY;
            }
            tm = get_fattime();
			ret = move_window(fs, fp->dir_sector);
			if (ret == E_NOERR) {
				dir = fp->dir_ptr;
				dir[DIR_Attr] |= A_ARCHIVE;						/* Set archive attribute to indicate that the file has been changed */
				store_cluster(fp->fs, dir, fp->fs->sclust);		/* Update file allocation information  */
				store_full(dir + DIR_FileSize, (uint32_t)fp->fs->objsize);	/* Update file size */
				store_full(dir + DIR_ModTime, tm);				/* Update modified time */
				store_half(dir + DIR_LstAccDate, 0);
				fs->dflag = 1;
				ret = sync_fs(fs);					/* Restore it to the directory */
				fp->flags &= (uint8_t)~FA_MODIFIED;
			}
        }
    }

    return ret;
}

err_t f_read (FILE* fp, void* buff, unsigned int btr, unsigned int* br);			/* Read data from the file */
err_t f_write (FILE* fp, const void* buff, unsigned int btw, unsigned int* bw);	/* Write data to the file */
err_t f_lseek (FILE* fp, size_t ofs);								/* Move file pointer of the file object */
err_t f_truncate (FILE* fp);										/* Truncate the file */
err_t f_opendir (DIR* dp, const char* path);						/* Open a directory */
err_t f_closedir (DIR* dp);										/* Close an open directory */
err_t f_readdir (DIR* dp, FILEINFO* fno);							/* Read a directory item */
err_t f_findfirst (DIR* dp, FILEINFO* fno, const char* path, const char* pattern);	/* Find first file */
err_t f_findnext (DIR* dp, FILEINFO* fno);							/* Find next file */
err_t f_mkdir (const char* path);								/* Create a sub directory */
err_t f_unlink (const char* path);								/* Delete an existing file or directory */
err_t f_rename (const char* path_old, const char* path_new);	/* Rename/Move a file or directory */
err_t f_stat (const char* path, FILEINFO* fno);					/* Get file status */
err_t f_chmod (const char* path, uint8_t attr, uint8_t mask);			/* Change attribute of a file/dir */
err_t f_utime (const char* path, const FILEINFO* fno);			/* Change timestamp of a file/dir */
err_t f_chdir (const char* path);								/* Change current directory */
err_t f_chdrive (const char* path);								/* Change current drive */
err_t f_getcwd (char* buff, unsigned int len);							/* Get current directory */
err_t f_getfree (const char* path, uint32_t* nclst, FS** fatfs);	/* Get number of free clusters on the drive */
err_t f_getlabel (const char* path, char* label, uint32_t* vsn);	/* Get volume label */
err_t f_setlabel (const char* label);							/* Set volume label */
err_t f_forward (FILE* fp, unsigned int(*func)(const uint8_t*,unsigned int), unsigned int btf, unsigned int* bf);	/* Forward data to the stream */
err_t f_expand (FILE* fp, size_t fsz, uint8_t opt);					/* Allocate a contiguous block to the file */
err_t f_mkfs (const char* path, const MKFS_PARM* opt, void* work, unsigned int len);	/* Create a FAT volume */
err_t f_fdisk (uint8_t pdrv, const uint16_t ptbl[], void* work);		/* Divide a physical drive into some partitions */
err_t f_setcp (uint32_t cp);											/* Set current code page */
int f_putc (char c, FILE* fp);										/* Put a character to the file */
int f_puts (const char* str, FILE* cp);								/* Put a string to the file */
int f_printf (FILE* fp, const char* str, ...);						/* Put a formatted string to the file */
char* f_gets (char* buff, int len, FILE* fp);