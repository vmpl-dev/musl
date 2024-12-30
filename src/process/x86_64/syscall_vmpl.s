#include "wrapper.h"
#include "syscall_vmpl.h"

.global vmpl_syscall
.type vmpl_syscall, @function
.extern vmpl_syscall_shim

vmpl_syscall:
    .cfi_startproc

    # 创建栈帧保存寄存器
    pushfq
    andq $~0x100, (%rsp)    # 清除 trap flag
    
    cld
    pushq %rbp
    pushq %rbx
    pushq %rdi
    pushq %rsi
    pushq %rdx
    pushq %rcx
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    pushq %rax

    movq %rsp, %rbp
    
    # 准备调用 vmpl_syscall_shim
    # 参数顺序: rdi(n), rsi(a1), rdx(a2), rcx(a3), r8(a4), r9(a5), rax(n)
    mov %r10, %rcx         # syscall to user-space calling convention
    andq $~0xF, %rsp       # 16字节对齐
    lea vmpl_syscall_shim(%rip), %rbx
    call *%rbx

ret:
    movq %rbp, %rsp
    addq $8, %rsp       # skip orig_rax
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rcx
    popq %rdx
    popq %rsi
    popq %rdi
    popq %rbx
    popq %rbp
    popfq
    retq

    .cfi_endproc
    .size vmpl_syscall,.-vmpl_syscall
