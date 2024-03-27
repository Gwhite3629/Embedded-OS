#ifndef _CHECKS_H_
#define _CHECK_H_

#define VALID(check, err) \
    do { \
        if ((check) == NULL) { \
            ret = err; \
            goto exit; \
        } \
    } while (0);

#define HANDLE_ERR(check, err) \
    do { \
        if ((check) != 0) { \
            ret = err; \
            goto exit; \
        } \
    } while(0);

#define CHECK(F) \
    do { \
        ret = F; \
        if (ret != 0) \
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