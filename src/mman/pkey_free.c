#define _GNU_SOURCE 1
#include <sys/mman.h>
#include "libc.h"
#include "syscall.h"

int __pkey_free(int pkey)
{
    int ret;

    // EINVAL pkey is invalid.
    if (pkey < 0 || pkey > 15) {
        errno = EINVAL;
        return -1;
    }

    ret = syscall(SYS_pkey_free, pkey);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }

    return ret;
}

weak_alias(__pkey_free, pkey_free);