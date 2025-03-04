#ifndef _CHECKS_H_
#define _CHECKS_H_

#include "err.h"

#define VALID(check, err) \
    do { \
        if ((check) == NULL) { \
            ret = err; \
            goto exit; \
        } \
    } while (0);

#define HANDLE_ERR(check, err) \
    do { \
        if ((check) != E_NOERR) { \
            ret = err; \
            goto exit; \
        } \
    } while(0);

#define CHECK(F) \
    do { \
        ret = F; \
        if (ret != E_NOERR) \
            goto exit; \
    } while(0);

#define HANDLE(cond, err) \
    do { \
        if (cond) { \
            ret = err; \
            goto exit; \
        } \
    } while(0);

#endif // _CHECKS_H_
