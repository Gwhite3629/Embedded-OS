#include <stdlib.h>
#include <fs/fat32.h>
#include <fs/file.h>
#include <drivers/sd.h>

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
					sector = (uint16_t)(*fs->current_access);
					ret = remove_chain(dj.fs, cl, 0);
					if (ret == E_NOERR) {
						ret = move_access(fs, sector);
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
					cluster = get_entry(fp->fs, cluster);
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
        				if (sd_read((char *)fp->buf, fp->sector, 1) != E_NOERR)
                            ret = E_FINT;
                    }
                }
            }
        }
    }
    if (ret != E_NOERR) {
        fp->fs = 0;	/* Invalidate file object on error */
    }

	return ret;
}	

/* Close an open file object */
err_t f_close (FILE* fp)
{
    err_t ret;
    FS *fs;

    ret = f_sync(fp);
    if (ret == E_NOERR) {
        ret = validate(fp->fs, &fs);
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

    ret = validate(fp->fs, &fs);
    if (ret == E_NOERR) {
        if (fp->flags & FA_MODIFIED) {
            if (fp->flags & FA_DIRTY) {
                if (sd_write((char *)fp->buf, fp->sector, 1) != E_NOERR)
                    return ret;
                fp->flags &= (uint8_t)~FA_DIRTY;
            }
            tm = get_fattime();
			ret = move_access(fs, fp->dir_sector);
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

/* Read data from the file */
err_t f_read (FILE* fp, void* buff, unsigned int btr, unsigned int* br)
{
    err_t ret;
    FS *fs;
    uint32_t cluster = 0;
    uint16_t sector = 0;
    size_t rem = 0;
    uint32_t cnt = 0, cc = 0, csector = 0;
    uint8_t *rbuff = (uint8_t *)buff;

    *br = 0;
    ret = validate(fp->fs, &fs);
    if (ret != E_NOERR || (ret = (err_t)fp->err) != E_NOERR) return ret;
    if (!(fp->flags & FA_READ)) return ret;
    rem = fp->fs->objsize - fp->fptr;
    if (btr > rem) btr = (uint32_t)rem;

    for (; btr > 0; btr -= cnt, *br += cnt, rbuff += cnt, fp->fptr += cnt) {
        if (fp->fptr % SECTOR_SIZE == 0) {
            csector = (uint32_t)(fp->fptr / SECTOR_SIZE & (fs->csize - 1));
            if (csector == 0) {
                cluster = fp->fs->sclust;
            } else {
                cluster = get_entry(fp->fs, fp->cluster);
            }
            if (cluster < 2) return E_FINT;
            if (cluster == 0xFFFFFFFF) return E_DISKERR;
            fp->cluster = cluster;
        }
        sector = c2s(fs, fp->cluster);
        if (sector == 0) return E_FINT;
        sector += csector;
        cc = btr / SECTOR_SIZE;
        if (cc > 0) {
            if (csector + cc > fs->csize) {
                cc = fs->csize - csector;
            }
            if (sd_read((char *)rbuff, sector, cc) != E_NOERR) return E_FINT;
            if ((fp->flags & FA_DIRTY) && fp->sector - sector < cc) {
                memcpy(rbuff + ((fp->sector - sector) * SECTOR_SIZE), fp->buf,
                SECTOR_SIZE);
            }
            cnt = SECTOR_SIZE * cc;
            continue;
        }
        if (fp->sector != sector) {
            if (fp->flags & FA_DIRTY) {
                if (sd_write((char *)fp->buf, fp->sector, 1) != E_NOERR) return E_FINT;
                fp->flags &= (uint8_t)~FA_DIRTY;
            }
            if (sd_read((char *)fp->buf, sector, 1) != E_NOERR) return E_DISKERR;
        }
        fp->sector = sector;
    }
    cnt = SECTOR_SIZE - (uint32_t)fp->fptr % SECTOR_SIZE;
    if (cnt > btr) cnt = btr;
    memcpy(rbuff, fp->buf + fp->fptr % SECTOR_SIZE, cnt);
exit:
    return E_NOERR;
}

/* Write data to the file */
err_t f_write (FILE* fp, const void* buff, unsigned int btw, unsigned int* bw)
{
    err_t ret;
    FS *fs;
    uint32_t cluster = 0;
    uint16_t sector = 0;
    uint32_t cnt = 0, cc = 0, csector = 0;
    const uint8_t *wbuff = (const uint8_t *)buff;

    *bw = 0;
    ret = validate(fp->fs, &fs);
    if (ret != E_NOERR || (ret = (err_t)fp->err) != E_NOERR) return ret;
    if (!(fp->flags & FA_WRITE)) return E_DENIED;
    if ((uint32_t)(fp->fptr + btw) < (uint32_t)fp->fptr) {
        btw = (uint32_t)(0xFFFFFFFF - (uint32_t)fp->fptr);
    }

    for (; btw > 0; btw -= cnt, *bw += cnt, wbuff += cnt, fp->fptr += cnt,
        fp->fs->objsize = (fp->fptr > fp->fs->objsize) ? fp->fptr :
        fp->fs->objsize) {
        if (fp->fptr % SECTOR_SIZE == 0) {
            csector = (uint32_t)(fp->fptr / SECTOR_SIZE) & (fs->csize - 1);
            if (csector == 0) {
                if (fp->fptr == 0) {
                    cluster = fp->fs->sclust;
                    if (cluster == 0) {
                        cluster = create_chain(fp->fs, 0);
                    }
                } else {
                    cluster = create_chain(fp->fs, fp->cluster);
                }
                if (cluster == 0) break;
                if (cluster == 1) return E_FINT;
                if (cluster == 0xFFFFFFFF) return E_DISKERR;
                fp->cluster = cluster;
                if (fp->fs->sclust == 0) fp->fs->sclust = cluster;
            }
            if (fp->flags & FA_DIRTY) {
                if (sd_write((char *)fp->buf, fp->sector, 1) != E_NOERR) return E_DISKERR;
                fp->flags &= (uint8_t)~FA_DIRTY;
            }
            sector = c2s(fs, fp->cluster);
            if (sector == 0) return E_FINT;
            sector += csector;
            cc = btw / SECTOR_SIZE;
            if (cc > 0) {
                if (csector + cc > fs->csize) {
                    cc = fs->csize - csector;
                }
                if (sd_write((char *)wbuff, sector, cc) != E_NOERR) return E_DISKERR;
                if (fp->sector - sector < cc) {
                    memcpy(fp->buf, wbuff + ((fp->sector - sector) *
                    SECTOR_SIZE), SECTOR_SIZE);
                    fp->flags &= (uint8_t)~FA_DIRTY;
                }
                cnt = SECTOR_SIZE * cc;
                continue;
            }
            if (fp->sector != sector &&
                fp->fptr < fp->fs->objsize &&
                sd_read((char *)fp->buf, sector, 1) != E_NOERR) {
                return E_DISKERR;
            }
            fp->sector = sector;
        }
        cnt = SECTOR_SIZE - (uint32_t)fp->fptr % SECTOR_SIZE;
        if (cnt > btw) cnt = btw;
        memcpy(fp->buf + fp->fptr % SECTOR_SIZE, wbuff, cnt);
        fp->flags |= FA_DIRTY;
    }

    fp->flags |= FA_MODIFIED;

    return E_NOERR;
}

/* Get file status */
err_t f_stat (const char* path, FILEINFO* fno)
{
    err_t ret;
    DIR dj;
    
    ret = mount_volume(&path, &dj.fs, 0);
    if (ret == E_NOERR) {
        ret = dir_follow_path(&dj, path);
        if (ret == E_NOERR) {
            if (dj.fn[11] & 0x80) {
                ret = E_INVALID_F;
            } else {
                if (fno) dir_getfileinfo(&dj, fno);
            }
        }
    }

    return E_NOERR;
}

/* Get number of free clusters on the drive */
err_t f_getfree (const char* path, uint32_t* nclst, FS** fatfs)
{
    err_t ret;
    FS *fs;
    uint32_t nfree = 0, cluster = 0, stat = 0;
    uint16_t sector = 0;
    uint32_t i = 0;

    ret = mount_volume(&path, &fs, 0);
    if (ret == E_NOERR) {
        *fatfs = fs;
        if (fs->nfc <= fs->n_entries - 2) {
            *nclst = fs->nfc;
        } else {
            nfree = 0;
            cluster = fs->n_entries;
            sector = fs->fbase;
            i = 0;
            do {
                if (i == 0) {
                    ret = move_access(fs, sector++);
                    if (ret != E_NOERR) break;
                }
                if ((load_full(fs->current_access + i) & 0x0FFFFFFF) == 0)
                    nfree++;
                i += 4;
                i %= SECTOR_SIZE;
            } while (--cluster);
        }
        if (ret == E_NOERR) {
            *nclst = nfree;
            fs->nfc = nfree;
            fs->fsiflag |= 1;
        }
    }

    return ret;
}

/* Delete an existing file or directory */
err_t f_unlink (const char* path)
{
    err_t ret;
    FS *fs;
    DIR dj, sdj;
    uint32_t dcluster = 0;

    ret = mount_volume(&path, &fs, FA_WRITE);
    if (ret == E_NOERR) {
        dj.fs = fs;
        ret = dir_follow_path(&dj, path);
        if (ret == E_NOERR && (dj.fn[11] & 0x20)) {
            ret = E_INVALID_F;
        }
        if (ret == E_NOERR) {
            if (dj.fn[11] & 0x80) {
                ret = E_INVALID_F;
            } else {
                if (dj.fs->attr & A_RDONLY) {
                    ret = E_DENIED;
                }
            }
            if (ret == E_NOERR) {
                dcluster = load_cluster(fs, dj.dir);
                if (dj.fs->attr & A_DIR) {
                    sdj.fs = fs;
                    sdj.fs->sclust = dcluster;
                    ret = dir_set_idx(&sdj, 0);
                    if (ret == E_NOERR) {
                        ret = dir_read(&sdj, 0);
                        if (ret == E_NOERR) ret = E_DENIED;
                        if (ret == E_NOFILE) ret = E_NOERR;
                    }
                }
            }
            if (ret == E_NOERR) {
                ret = dir_remove(&dj);
                if (ret == E_NOERR && dcluster != 0) {
                    ret = remove_chain(dj.fs, dcluster, 0);
                }
                if (ret == E_NOERR) ret = sync_fs(fs);
            }
        }
    }

    return ret;
}

/* Rename/Move a file or directory */
err_t f_rename (const char *path_old, const char *path_new)
{
    err_t ret;
    FS *fs = NULL;
    DIR djo, djn;
    uint8_t buf[32], *dir;
    uint16_t sector = 0;

    get_ldnumber(&path_new);
    ret = dir_follow_path(&djo, path_old);
    if (ret == E_NOERR && (djo.fn[11] & (0x80 | 0x20))) ret = E_INVALID_F;
    if (ret == E_NOERR) {
        memcpy(buf, djo.dir, 32);
        memcpy(&djn, &djo, sizeof(DIR));
        ret = dir_follow_path(&djn, path_new);
        if (ret == E_NOERR) {
            ret = (djn.fs->sclust == djo.fs->sclust && djn.dptr == djo.dptr) ?
            E_NOFILE : E_FDENIED;
        }
        if (ret == E_NOFILE) {
            ret = dir_register(&djn);
            if (ret == E_NOERR) {
                dir = djn.dir;
                memcpy(dir + 13, buf + 13, 32 - 13);
                dir[11] = buf[11];
                if (!(dir[11] & A_DIR)) dir[11] |= A_ARCHIVE;
                fs->dflag = 1;
                if ((dir[11] & A_DIR) && djo.fs->sclust != djn.fs->sclust) {
                    sector = c2s(fs, load_cluster(fs, dir));
                    if (sector != 0) {
                        ret = E_FINT;
                    } else {
                        ret = move_access(fs, sector);
                        dir = fs->current_access + 32 * 1;
                        if (ret == E_NOERR && dir[1] == '.') {
                            store_cluster(fs, dir, djn.fs->sclust);
                            fs->dflag = 1;
                        }
                    }
                }
            }
        }

        if (ret == E_NOERR) {
            ret = dir_remove(&djo);
            if (ret == E_NOERR) {
                ret = sync_fs(fs);
            }
        }
    }
    
    return ret;
}

/* Create a sub directory */
err_t f_mkdir (const char* path)
{
    err_t ret;
    FS *fs = NULL;
    FS *Sfs = NULL;
    DIR dj;
    uint32_t dcl = 0, pcl = 0, tm = 0;

    ret = mount_volume(&path, &fs, FA_WRITE);
    if (ret == E_NOERR) {
        dj.fs = fs;
        ret = dir_follow_path(&dj, path);
        if (ret == E_NOERR) ret = E_EXIST;
        if (dj.fn[11] & 0x20) {
            ret = E_INVALID_F;
        }
        if (ret == E_NOFILE) {
            Sfs = fs;
            dcl = create_chain(Sfs, 0);
            ret = E_NOERR;
            if (dcl == 0) ret = E_FDENIED;
            if (dcl == 1) ret = E_FINT;
            if (dcl == 0xFFFFFFFF) ret = E_DISKERR;
            tm = get_fattime();
            if (ret == E_NOERR) {
                ret = dir_clear(fs, dcl);
                if (ret == E_NOERR) {
                    memset(fs->current_access + 0, ' ', 11);
                    fs->current_access[0] = '.';
                    fs->current_access[11] = A_DIR;
                    store_full(fs->current_access + 22, tm);
                    store_cluster(fs, fs->current_access, dcl);
                    memcpy(fs->current_access + 32, fs->current_access, 32);
                    fs->current_access[32 + 1] = '.';
                    pcl = dj.fs->sclust;
                    store_cluster(fs, fs->current_access + 32, pcl);
                    fs->dflag = 1;
                    ret = dir_register(&dj);
                }
            }
            if (ret == E_NOERR) {
                store_full(dj.dir + 22, tm);
                store_cluster(fs, dj.dir, dcl);
                dj.dir[11] = A_DIR;
                fs->dflag = 1;
            }
            if (ret == E_NOERR) {
                ret = sync_fs(fs);
            }
        } else {
            remove_chain(Sfs, dcl, 0);
        }
    }

    return ret;
}

/* Open a directory */
err_t f_opendir (DIR* dp, const char* path)
{
    err_t ret;
    FS *fs;

    if (!dp) return E_INVALID;

    ret = mount_volume(&path, &fs, 0);
    if (ret == E_NOERR) {
        dp->fs = fs;
        ret = dir_follow_path(dp, path);
        if (ret == E_NOERR) {
            if (!(dp->fn[11] & 0x80)) {
                if (dp->fs->attr & A_DIR) {
                    dp->fs->sclust = load_cluster(fs, dp->dir);
                } else {
                    ret = E_NOPATH;
                }
            }
            if (ret == E_NOERR) {
                dp->fs->id = fs->id;
                ret = dir_set_idx(dp, 0);
            }
        }
        if (ret == E_NOFILE) ret = E_NOPATH;
    }
    if (ret != E_NOERR) dp->fs = 0;

    return ret;
}

/* Close an open directory */
err_t f_closedir (DIR* dp)
{
    err_t ret;
    FS *fs;

    ret = validate(dp->fs, &fs);
    if (ret == E_NOERR) {
        dp->fs = 0;
    }

    return ret;
}

/* Read a directory item */
err_t f_readdir (DIR* dp, FILEINFO* fno)
{
    err_t ret;
    FS *fs;

    ret = validate(dp->fs, &fs);
    if (ret == E_NOERR) {
        if (!fno) {
            ret = dir_set_idx(dp, 0);
        } else {
            ret = dir_read(dp, 0);
            if (ret == E_NOFILE) ret = E_NOERR;
            if (ret == E_NOERR) {
                dir_getfileinfo(dp, fno);
                ret = dir_next(dp, 0);
                if (ret == E_NOFILE) ret = E_NOERR;
            }
        }
    }

    return ret;
}

err_t f_lseek (FILE* fp, size_t ofs);								/* Move file pointer of the file object */
err_t f_truncate (FILE* fp);										/* Truncate the file */
err_t f_findfirst (DIR* dp, FILEINFO* fno, const char* path, const char* pattern);	/* Find first file */
err_t f_findnext (DIR* dp, FILEINFO* fno);							/* Find next file */
err_t f_chmod (const char* path, uint8_t attr, uint8_t mask);			/* Change attribute of a file/dir */
err_t f_utime (const char* path, const FILEINFO* fno);			/* Change timestamp of a file/dir */
err_t f_chdir (const char* path);								/* Change current directory */
err_t f_chdrive (const char* path);								/* Change current drive */
err_t f_getcwd (char* buff, unsigned int len);							/* Get current directory */
err_t f_getlabel (const char* path, char* label, uint32_t* vsn);	/* Get volume label */
err_t f_setlabel (const char* label);							/* Set volume label */
err_t f_forward (FILE* fp, unsigned int(*func)(const uint8_t*,unsigned int), unsigned int btf, unsigned int* bf);	/* Forward data to the stream */
err_t f_expand (FILE* fp, size_t fsz, uint8_t opt);					/* Allocate a contiguous block to the file */
err_t f_mkfs (const char* path, const MKFS_PARM* opt, void* work, unsigned int len);	/* Create a FAT volume */
err_t f_fdisk (uint8_t pdrv, const uint16_t ptbl[], void* work);		/* Divide a physical drive into some partitions */
err_t f_setcp (uint32_t cp);											/* Set current code page */
int f_puts (const char* str, FILE* cp);								/* Put a string to the file */
int f_printf (FILE* fp, const char* str, ...);						/* Put a formatted string to the file */

int f_putc (char c, FILE* fp)
{
	int ret = E_NOERR;

    unsigned int nw;

    char *buf[2] = {c, '\0'};

    ret = f_write(fp, buf,1,nw);

    return ret;
}

char* f_gets (char* buff, int len, FILE* fp)
{
    int nc = 0;
	char *p = buff;
	uint8_t s[4];
	unsigned int rc;
	uint32_t dc;
    unsigned int ct;

	while (nc < len) {
        f_read(fp, s, 1, &rc);		/* Get a code unit */
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
            f_read(fp, s, ct, &rc);	/* Get trailing bytes */
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
		f_read(fp, s, 1, &rc);	/* Get a byte */
		if (rc != 1) break;		/* EOF? */
		dc = s[0];
		if (dc == '\r') continue;
		*p++ = (char)dc; nc++;
		if (dc == '\n') break;
	}

	*p = 0;		/* Terminate the string */
	return nc ? buff : 0;	/* When no data read due to EOF or error, return with error. */
}