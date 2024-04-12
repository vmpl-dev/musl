#define _GNU_SOURCE 1
#include <sys/mman.h>
#include "libc.h"
#include "syscall.h"

int __pkey_mprotect(void *addr, size_t len, int prot, int pkey) {
    int ret;

    // EINVAL pkey or prot is invalid.
    if (pkey < 0 || pkey > 15 || prot < 0 || prot > (PROT_READ | PROT_WRITE | PROT_EXEC)) {
        errno = EINVAL;
        return -1;
    }

    return syscall(SYS_pkey_mprotect, addr, len, prot, pkey);
}

weak_alias(__pkey_mprotect, pkey_mprotect);