#ifndef __VMPL_SYSCALL_H_
#define __VMPL_SYSCALL_H_
.macro VMPL_SYSCALL
    push   %r12
    pushf
    pop    %r12
    pushf
    push %rax
    mov %cs, %ax
    test $3, %al
    jnz 11f
    cli
    push   %rcx
    push   %rdx
    push   %rbp
    mov $0xd8,%eax
    mov %gs:(%eax),%rbp
    test %rbp,%rbp
    jz 10f
    mov    $0x1,%edx        /* protocol_version */
    mov    $0x80000018,%ecx /* sw_exit_code */
    movl   $0x0,0xffc(%rbp) /* ghcb->ghcb_usage (0) */
    mov    %dx,0xffa(%rbp)  /* ghcb->protocol_version (1) */
    movabs $0x1c000000000000,%rdx
    mov    %rcx,0x390(%rbp) /* ghcb->sw_exit_code */
    or     %rdx,0x3f8(%rbp) /* ghcb->valid_bitmap */
    movq   $0x0,0x398(%rbp) /* ghcb->sw_exit_info_1 */
    movq   $0x0,0x3a0(%rbp) /* ghcb->sw_exit_info_2 */
    pop    %rbp
    pop    %rdx
    pop    %rcx
    pop    %rax
    popf
    vmgexit
    pop %r12
    test   $0x200,%r12
    jz 22f
    sti
    jmp 22f
10:
    mov $0xc0010130, %ecx
    mov $0x16, %eax
    xor %edx, %edx
    wrmsr
    pop %rbp
    pop %rdx
    pop %rcx
    pop %rax
    popf
    vmgexit
    pop %r12
    test $0x200, %r12
    jz 22f
    sti
    jmp 22f
11:
    pop %rax
    popf
    pop %r12
    syscall
22:
.endm
#endif