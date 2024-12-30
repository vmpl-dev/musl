#ifndef _SYSCALL_VMPL_H_
#define _SYSCALL_VMPL_H_

#ifdef __ASSEMBLER__
#   define VMPL_SYSCALL callq *vmpl_syscall@GOTPCREL(%rip)
#endif

#endif
