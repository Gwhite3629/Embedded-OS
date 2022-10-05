#ifndef _STATUS_H_
#define _STATUS_H_

#define COMM -1     // Is the commissioner

#define IDLE 0      // In queue, pre-insert

#define SLEEP 1     // Process is waiting for interrupt, don't insert

#define RUNNING 2   // On the processor

#define WAITING 3   // Waiting in queue

#define DESTROY 4   // Marked for destroy, in queue

#define STALE 5     // Stale is the first thing market when destroying

#endif  // _STATUS_H_