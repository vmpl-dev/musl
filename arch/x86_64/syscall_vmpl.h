#ifndef __VMPL_SYSCALL_H_
#define __VMPL_SYSCALL_H_
.macro VMPL_SYSCALL
    pushf
    push %rax
    mov %cs, %rax
    test $3, %rax
    jnz 1f
    push %rcx
    push %rdx
    mov $0xc0010130, %ecx
    mov $16, %eax
    xor %edx, %edx
    wrmsr
    pop %rdx
    pop %rcx
    pop %rax
    popf
    vmgexit
    jmp 2f
1:
    pop %rax
    popf
    syscall
2:
.endm
#endif