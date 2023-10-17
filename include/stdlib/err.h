#ifndef _ERR_H_
#define _ERR_H_

#include "types.h"

#define NULL ((void *) 0)

#define NLIST ((DLL *) 0)

#define err_t int32_t

err_t error;

#define E_NOERR     0x1000  // No error
#define E_PANIC     0x0001  // Panic signal for commisioner
#define E_BADARG    0x0002  // Bad argument
#define E_NOSPACE   0x0003  // No space

// Port errors
#define E_NOPORT    0x0004  // No available port
#define E_BUSYPORT  0x0005  // Port busy
#define E_FAILPORT  0x0006  // Port failure
#define E_NOWRPORT  0x0007  // No write on port

// File errors
#define E_NOFILE    0x0008  // No available file
#define E_BUSYFILE  0x0009  // File busy
#define E_FAILFILE  0x000A  // File failure
#define E_NORWFILE  0x000B  // No write on file

// Device errors
#define E_NODEV     0x000C  // No available device
#define E_BUSYDEV   0x000D  // Device busy
#define E_FAILDEV   0x000E  // Device failure
#define E_NOWRDEV   0x000F  // No write on device

// Process errors
#define E_NOSTART   0x0010  // Cannot start process

// Memory errors
#define E_NOMEM     0x0014  // No available memory

// UART errors
#define E_OERR      0x0033  // UART Overrun error
#define E_BERR      0x0032  // UART Break error
#define E_PERR      0x0031  // UART Parity error
#define E_FERR      0x0030  // UART Framing error

// FAT32 errors
#define E_BADC      0x0100  // Bad cluster
#define E_FSINT     0x0101

#endif  // _ERR_H_
