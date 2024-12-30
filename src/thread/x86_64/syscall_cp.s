.include "syscall_vmpl.h"
.text
.global __cp_begin
.hidden __cp_begin
.global __cp_end
.hidden __cp_end
.global __cp_cancel
.hidden __cp_cancel
.hidden __cancel
.global __syscall_cp_asm
.hidden __syscall_cp_asm
.type   __syscall_cp_asm,@function
.extern vmpl_syscall
__syscall_cp_asm:

__cp_begin:
	mov (%rdi),%eax
	test %eax,%eax
	jnz __cp_cancel
	mov %rdi,%r11		/* save rdi[arg0] to r11[tmp] */
	mov %rsi,%rax		/* save rsi[arg1] to rax[sys_nr] */
	mov %rdx,%rdi		/* save rdx[arg2] to rdi[arg0] */
	mov %rcx,%rsi		/* save rcx[arg3] to rsi[arg1] */
	mov %r8,%rdx		/* save r8[arg4] to rdx[arg2] */
	mov %r9,%r10		/* save r9[arg5] to r10[arg3] */
	mov 8(%rsp),%r8		/* save [arg6] to r8[arg4] */
	mov 16(%rsp),%r9	/* save [arg7] to r9[arg5] */
	mov %r11,8(%rsp)	/* restore r11[tmp] to [arg6] */
	callq vmpl_syscall
__cp_end:
	ret
__cp_cancel:
	jmp __cancel
