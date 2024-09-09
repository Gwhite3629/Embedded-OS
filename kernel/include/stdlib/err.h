#ifndef _ERR_H_
#define _ERR_H_

#include "types.h"

#define NULL ((void *) 0)

#define NLIST ((DLL *) 0)

typedef int err_t;

#define E_NOERR     0x1000  // No error
#define E_PANIC     0x0001  // Panic signal for commisioner
#define E_BADARG    0x0002  // Bad argument
#define E_NOSPACE   0x0003  // No space

// Port errors
#define E_NOPORT    0x0004  // No available port
#define E_BUSYPORT  0x0005  // Port busy
#define E_FAILPORT  0x0006  // Port failure
#define E_NOWRPORT  0x0007  // No write on port

// Device errors
#define E_NODEV     0x000C  // No available device
#define E_BUSYDEV   0x000D  // Device busy
#define E_FAILDEV   0x000E  // Device failure
#define E_NOWRDEV   0x000F  // No write on device

// Process errors
#define E_NOSTART   0x0010  // Cannot start process

// Memory errors
#define E_NOMEM     0x0014  // No available memory
#define E_NOHEAP    0x0015  // Couldn't create heap
#define E_NOPAGE    0x0016  // Couldn't fetch user pages

// UART errors
#define E_OERR      0x0033  // UART Overrun error
#define E_BERR      0x0032  // UART Break error
#define E_PERR      0x0031  // UART Parity error
#define E_FERR      0x0030  // UART Framing error

// FAT32 errors
#define E_BADC      0x0100  // Bad cluster
#define E_FSINT     0x0101  // Internal error
#define E_FSNOF     0x0102  // No file in FS
#define E_NOFREE    0x0103  // No free clusters
#define E_BADNAME   0x0104  // Name doesn't work
#define E_NOPATH    0x0105  // Couldn't find path
#define E_NODRIVE   0x0106  // Invalid drive
#define E_DENIED    0x0107  // Denied access
#define E_EXIST     0x0108  // File exists, name conflict
#define E_INVALID   0x0109  // Invalid object

// SD errors
#define E_NOT_READY 0x0200
#define E_WRONLY    0x0201
#define E_DISKERR   0x0202
#define E_NOFS      0x0203
#define E_TIMEOUT   0x0204
#define E_NOCARD    0x0206

// File errors
#define E_NOFILE    0x0300  // No available file
#define E_BUSYFILE  0x0301  // File busy
#define E_FAILFILE  0x0302  // File failure
#define E_NORWFILE  0x0303  // No write on file
#define E_FINT      0x0304  // Internal error
#define E_INVALID_F 0x0305  // Invalid file
#define E_FDENIED   0x0306  // Denied access

// Timer errors
#define E_NXIO      0x0400


#endif  // _ERR_H_
