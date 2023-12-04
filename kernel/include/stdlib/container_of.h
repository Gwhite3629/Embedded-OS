#ifndef _CONTAINER_H_
#define _CONTAINER_H_

#define container_of(ptr, type, member) ({ \
    void *mptr = (void *)(ptr); \
    ((type *)(mptr - __builtin_offsetof(type, member))); })

#endif // _CONTAINER_H_