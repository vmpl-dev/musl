#ifndef __VMPL_SYSCALL_H_
#define __VMPL_SYSCALL_H_
.macro VMPL_SYSCALL
    pushf
    push %rax
    mov %cs, %ax
    test $3, %al
    jnz 11f
    push %rcx
    push %rdx
    mov $0xc0010130, %ecx
    mov $0x16, %eax
    xor %edx, %edx
    wrmsr
    pop %rdx
    pop %rcx
    pop %rax
    popf
    vmgexit
    jmp 22f
11:
    pop %rax
    popf
    syscall
22:
.endm
#endif