.include "syscall_vmpl.h"
.global vfork
.type vfork,@function
vfork:
	pop %rdx
	mov $58,%eax
	VMPL_SYSCALL
	push %rdx
	mov %rax,%rdi
	.hidden __syscall_ret
	jmp __syscall_ret
