#ifndef _SYSCALL_H_
#define _SYSCALL_H_

// Ports, pins and IO
#define S_PREAD     0x0004  // Port read
#define S_PWRITE    0x0005  // Port write
#define S_POPEN     0x0006  // Port open
#define S_PCLOSE    0x0007  // Port close

// File, breaks to filesystem
#define S_FREAD     0x0008  // File read
#define S_FWRITE    0x0009  // File write
#define S_FOPEN     0x000A  // File open
#define S_FCLOSE    0x000B  // File close

// Device, generic module interface
#define S_DREAD     0x000C  // Device read
#define S_DWRITE    0x000D  // Device write
#define S_DOPEN     0x000E  // Device open
#define S_DCLOSE    0x000F  // Device close

// Processes
#define S_PSTART    0x0010  // Process start
#define S_SLEEP     0x0011  // Process sleep
#define S_PINT      0x0012  // Process interrupt
#define S_PEXIT     0x0013  // Process exit

// Memory
#define S_MMAP      0x0014  // Used in malloc
#define S_MUNMAP    0x0015  // Used in malloc
#define S_MALLOC    0x0016  // Used in malloc
#define S_PMALLOC   0x0017  // Process malloc

err_t call(uint32_t SCALL);

#endif  // _SYSCALL_H_