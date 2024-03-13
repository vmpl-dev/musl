/* Copyright 2011-2012 Nicholas J. Kain, licensed under standard MIT license */
.include "syscall_vmpl.h"
.text
.global __unmapself
.type   __unmapself,@function
__unmapself:
	movl $11,%eax   /* SYS_munmap */
	VMPL_SYSCALL    /* munmap(arg2,arg3) */
	xor %rdi,%rdi   /* exit() args: always return success */
	movl $60,%eax   /* SYS_exit */
	VMPL_SYSCALL    /* exit(0) */
