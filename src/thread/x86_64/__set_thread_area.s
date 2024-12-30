/* Copyright 2011-2012 Nicholas J. Kain, licensed under standard MIT license */
.include "syscall_vmpl.h"
.text
.global __set_thread_area
.hidden __set_thread_area
.type __set_thread_area,@function
.extern vmpl_syscall
__set_thread_area:
	mov %rdi,%rsi           /* shift for syscall */
	movl $0x1002,%edi       /* SET_FS register */
	movl $158,%eax          /* set fs segment to */
	callq vmpl_syscall       /* arch_prctl(SET_FS, arg)*/
	ret
