#ifndef __VMPL_SYSCALL_H_
#define __VMPL_SYSCALL_H_
.macro VMPL_SYSCALL
    pushf
    sub $168UL, %rsp
    mov %rax, 120(%rsp)
    mov %rdi, 112(%rsp)
    mov %rsi, 104(%rsp)
    mov %rdx, 96(%rsp)
    mov %rcx, 88(%rsp)
    mov %r8, 72(%rsp)
    mov %r9, 64(%rsp)
    mov %r10, 56(%rsp)
    mov %r11, 48(%rsp)
    mov %rsp, %rdi
    call vmpl_syscall
    mov 112(%rsp), %rdi
    mov 104(%rsp), %rsi
    mov 96(%rsp), %rdx
    mov 88(%rsp), %rcx
    mov 72(%rsp), %r8
    mov 64(%rsp), %r9
    mov 56(%rsp), %r10
    mov 48(%rsp), %r11
    add $168UL, %rsp
    popf
.endm
#endif