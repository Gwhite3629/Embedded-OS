#ifndef _STATUS_H_
#define _STATUS_H_

#define COMM   -1       // Is the commissioner

#define INVALID 0       // Process is invalid

#define IDLE    1       // In queue, pre-insert

#define SLEEP   2       // Process is waiting for interrupt, don't insert

#define RUNNING 3       // On the processor

#define WAITING 4       // Waiting in queue

#define DESTROY 5       // Marked for destroy, in queue

#define STALE   6       // Stale is the first thing marked when destroying

#endif  // _STATUS_H_