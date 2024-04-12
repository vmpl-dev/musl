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

    return syscall(SYS_pkey_free, pkey);
}

weak_alias(__pkey_free, pkey_free);