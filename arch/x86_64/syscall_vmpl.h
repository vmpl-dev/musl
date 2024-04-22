#ifndef __VMPL_SYSCALL_H_
#define __VMPL_SYSCALL_H_
.macro VMPL_SYSCALL
    pushfq
    push %rcx
    push %r11
    push %r9
    push %r8
    push %r10
    push %rdx
    push %rsi
    push %rdi
    mov %rax, %rdi
    mov %rsp, %rsi
    call vmpl_syscall
    pop %rdi
    pop %rsi
    pop %rdx
    pop %r10
    pop %r8
    pop %r9
    pop %r11
    pop %rcx
    popfq
.endm
#endif