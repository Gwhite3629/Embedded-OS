#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include "../stdlib/err.h"
#include "../stdlib/types.h"

// Serial
#define SYS_SREAD     0x0000    // Serial read
#define SYS_SWRITE      0x0001  // Serial write
#define SYS_SOPEN       0x0002  // Serial open
#define SYS_SCLOSE      0x0003  // Serial close

// Ports, pins and IO
#define SYS_PREAD     0x0004  // Port read
#define SYS_PWRITE    0x0005  // Port write
#define SYS_POPEN     0x0006  // Port open
#define SYS_PCLOSE    0x0007  // Port close

// File, breaks to filesystem
#define SYS_FREAD     0x0008  // File read
#define SYS_FWRITE    0x0009  // File write
#define SYS_FOPEN     0x000A  // File open
#define SYS_FCLOSE    0x000B  // File close

// Device, generic module interface
#define SYS_DREAD     0x000C  // Device read
#define SYS_DWRITE    0x000D  // Device write
#define SYS_DOPEN     0x000E  // Device open
#define SYS_DCLOSE    0x000F  // Device close

// Processes
#define SYS_PSTART    0x0010  // Process start
#define SYS_SLEEP     0x0011  // Process sleep
#define SYS_PINT      0x0012  // Process interrupt
#define SYS_PEXIT     0x0013  // Process exit
#define SYS_GETPID    0x0014  // Process Get PID

// Memory
#define SYS_MMAP      0x0014  // Used in malloc
#define SYS_MUNMAP    0x0015  // Used in malloc
#define SYS_MALLOC    0x0016  // Used in malloc
#define SYS_PMALLOC   0x0017  // Process malloc
#define SYS_FREE      0x0018  // Free from heap

// TIME
#define SYS_TIME      0x0100  // Get time

err_t swi_handler_c(uint64_t r0, uint64_t r1, uint64_t r2, uint64_t r3);

#endif  // _SYSCALL_H_