/* Copyright 2011-2012 Nicholas J. Kain, licensed under standard MIT license */
.include "syscall_vmpl.h"
.text
.global __unmapself
.type   __unmapself,@function
.extern vmpl_syscall
__unmapself:
	movl $11,%eax   /* SYS_munmap */
	callq vmpl_syscall /* munmap(arg2,arg3) */
	xor %rdi,%rdi   /* exit() args: always return success */
	movl $60,%eax   /* SYS_exit */
	callq vmpl_syscall /* exit(0) */
