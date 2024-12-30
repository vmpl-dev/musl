#define _GNU_SOURCE

#include "syscall_arch.h"

long vmpl_syscall_shim(long a1, long a2, long a3, long a4, long a5,
                       long a6, long n) {
  return __syscall6(n, a1, a2, a3, a4, a5, a6);
}
