#ifndef __VMPL_SYSCALL_H_
#define __VMPL_SYSCALL_H_
.macro VMPL_SYSCALL
    pushf
    push %rax
    mov %cs, %ax
    test $3, %al
    jnz 11f
    push   %rcx
    push   %rdx
    mov $0xd8,%eax
    mov %gs:(%eax),%rax
    test %rax,%rax
    jz 10f
    mov    $0x1,%edx        /* protocol_version */
    mov    $0x80000018,%ecx /* sw_exit_code */
    movl   $0x0,0xffc(%rax) /* ghcb->ghcb_usage (0) */
    mov    %dx,0xffa(%rax)  /* ghcb->protocol_version (1) */
    movabs $0x1c000000000000,%rdx
    mov    %rcx,0x390(%rax) /* ghcb->sw_exit_code */
    or     %rdx,0x3f8(%rax) /* ghcb->valid_bitmap */
    movq   $0x0,0x398(%rax) /* ghcb->sw_exit_info_1 */
    movq   $0x0,0x3a0(%rax) /* ghcb->sw_exit_info_2 */
    pop    %rdx
    pop    %rcx
    pop    %rax
    popf
    vmgexit
    jmp 22f
10:
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