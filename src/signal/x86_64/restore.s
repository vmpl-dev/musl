	nop
.include "syscall_vmpl.h"
.global __restore_rt
.hidden __restore_rt
.type __restore_rt,@function
.extern vmpl_syscall
__restore_rt:
	mov $15, %rax
	callq vmpl_syscall
.size __restore_rt,.-__restore_rt
