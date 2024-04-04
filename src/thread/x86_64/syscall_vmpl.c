#include "syscall_arch.h"

struct syscall_args_t {
    long a0, a1, a2, a3, a4, a5;
} __attribute__((packed));

long vmpl_syscall(long sys_nr, struct syscall_args_t *args) {
    __syscall6(sys_nr, args->a0, args->a1, args->a2, args->a3, args->a4, args->a5);
}