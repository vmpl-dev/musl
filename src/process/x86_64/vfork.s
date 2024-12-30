.include "syscall_vmpl.h"
.global vfork
.type vfork,@function
.extern vmpl_syscall
vfork:
	pop %rdx
	mov $58,%eax
	callq vmpl_syscall
	push %rdx
	mov %rax,%rdi
	.hidden __syscall_ret
	jmp __syscall_ret
