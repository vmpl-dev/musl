#define _GNU_SOURCE 1
#include <sys/mman.h>
#include <errno.h>
#include "libc.h"
#include "syscall.h"

int __pkey_alloc(unsigned int flags, unsigned int access_rights)
{
    int ret;

    // EINVAL pkey, flags, or access_rights is invalid.
    if (flags != 0 || access_rights > (PKEY_DISABLE_ACCESS | PKEY_DISABLE_WRITE)) {
        errno = EINVAL;
        return -1;
    }

    return syscall(SYS_pkey_alloc, flags, access_rights);
}

weak_alias(__pkey_alloc, pkey_alloc);