#define __SYSCALL_LL_E(x) (x)
#define __SYSCALL_LL_O(x) (x)

#include "ghcb.h"

#define X86_EFLAGS_IF 0x200
#define GHCB		216
#define VMPL 		0ULL

#define percpu(var, offset)              \
	__asm__ volatile("mov %%gs:(%1), %0" \
					 : "=r"(var)         \
					 : "r"(offset));

static inline int irqs_disabled_flags(unsigned long flags)
{
	return !(flags & X86_EFLAGS_IF);
}

static inline void local_irq_disable(void)
{
	__asm__ __volatile__("cli" : : : "memory");
}

static inline void local_irq_enable(void)
{
	__asm__ __volatile__("sti" : : : "memory");
}

static inline unsigned long local_irq_save()
{
	unsigned long flags;
	__asm__ __volatile__("pushfq; popq %0; cli" : "=r"(flags) : : "memory");
	return flags;
}

static inline unsigned long local_irq_restore(unsigned long flags)
{
	unsigned short cs;
	__asm__ __volatile__("movw %%cs, %0" : "=r"(cs));
	if ((cs & 0x3) == 0)
	{
		if (!irqs_disabled_flags(flags))
			local_irq_enable();
	}
}

#define GHCB_PROTOCOL_SWITCH 1
#ifdef GHCB_PROTOCOL_COMPLETE
#define __msr_protocol(__vmgexit)                       \
	do                                                  \
	{                                                   \
		unsigned long val, resp;                        \
		val = __rdmsr(GHCB_MSR);                        \
		__wrmsr(GHCB_MSR, GHCB_MSR_VMPL_REQ_LEVEL(0));  \
		__asm__ __vmgexit;                              \
		resp = __rdmsr(GHCB_MSR);                       \
		__wrmsr(GHCB_MSR, val);                         \
		if (GHCB_RESP_CODE(resp) != GHCB_MSR_VMPL_RESP) \
			ret = -ENOSYS;                              \
		if (GHCB_MSR_VMPL_RESP_VAL(resp) != 0)          \
			ret = -ENOSYS;                              \
	} while (0)

#define __ghcb_protocol(__vmgexit)                             \
	do                                                         \
	{                                                          \
		uint64_t sw_exit_info_1;                               \
		ghcb->protocol_version = GHCB_PROTOCOL_MIN;            \
		ghcb->ghcb_usage = GHCB_DEFAULT_USAGE;                 \
		ghcb_set_sw_exit_code(ghcb, SVM_VMGEXIT_SNP_RUN_VMPL); \
		ghcb_set_sw_exit_info_1(ghcb, VMPL);                   \
		ghcb_set_sw_exit_info_2(ghcb, 0ULL);                   \
		__asm__ __vmgexit;                                     \
		sw_exit_info_1 = ghcb_get_sw_exit_info_1(ghcb);        \
		if (!ghcb_sw_exit_info_1_is_valid(ghcb))               \
			return -ENOSYS;                                    \
		if (lower_32_bits(sw_exit_info_1) != 0)                \
			return -ENOSYS;                                    \
	} while (0)
#else
#define __msr_protocol(__vmgexit)                      \
	do                                                 \
	{                                                  \
		__wrmsr(GHCB_MSR, GHCB_MSR_VMPL_REQ_LEVEL(0)); \
		__asm__ __vmgexit;                             \
	} while (0)

#define __ghcb_protocol(__vmgexit)                             \
	do                                                         \
	{                                                          \
		ghcb->protocol_version = GHCB_PROTOCOL_MIN;            \
		ghcb->ghcb_usage = GHCB_DEFAULT_USAGE;                 \
		ghcb_set_sw_exit_code(ghcb, SVM_VMGEXIT_SNP_RUN_VMPL); \
		ghcb_set_sw_exit_info_1(ghcb, VMPL);                   \
		ghcb_set_sw_exit_info_2(ghcb, 0ULL);                   \
		__asm__ __vmgexit;                                     \
	} while (0)
#endif

#ifdef GHCB_PROTOCOL_SWITCH // GHCB_PROTOCOL_SWITCH
#define __syscall_wrapper(__vmgexit, __syscall)           \
	do                                                    \
	{                                                     \
		unsigned short cs;                                \
		__asm__ __volatile__("movw %%cs, %0" : "=r"(cs)); \
		if ((cs & 0x3) == 0)                              \
		{                                                 \
			unsigned long flags;                          \
			flags = local_irq_save();                     \
			struct ghcb *ghcb;                            \
			percpu(ghcb, GHCB);                           \
			if (ghcb)                                     \
			{                                             \
				__ghcb_protocol(__vmgexit);               \
			}                                             \
			else                                          \
			{                                             \
				__msr_protocol(__vmgexit);                \
			}                                             \
			local_irq_restore(flags);                     \
		}                                                 \
		else                                              \
		{                                                 \
			__asm__ __syscall;                            \
		}                                                 \
	} while (0)
#elif defined(GHCB_PROTOCOL) // GHCB_PROTOCOL
#define __syscall_wrapper(__vmgexit, __syscall)           \
	do                                                    \
	{                                                     \
		unsigned short cs;                                \
		__asm__ __volatile__("movw %%cs, %0" : "=r"(cs)); \
		if ((cs & 0x3) == 0)                              \
		{                                                 \
			unsigned long flags;                          \
			flags = local_irq_save();                     \
			struct ghcb *ghcb;                            \
			percpu(ghcb, GHCB);                           \
			__ghcb_protocol(__vmgexit);                   \
			local_irq_restore(flags);                     \
		}                                                 \
		else                                              \
		{                                                 \
			__asm__ __syscall;                            \
		}                                                 \
	} while (0)
#else // MSR_PROTOCOL
#define __syscall_wrapper(__vmgexit, __syscall)           \
	do                                                    \
	{                                                     \
		unsigned short cs;                                \
		__asm__ __volatile__("movw %%cs, %0" : "=r"(cs)); \
		if ((cs & 0x3) == 0)                              \
		{                                                 \
			unsigned long flags;                          \
			flags = local_irq_save();                     \
			__msr_protocol(__vmgexit);                    \
			local_irq_restore(flags);                     \
		}                                                 \
		else                                              \
		{                                                 \
			__asm__ __syscall;                            \
		}                                                 \
	} while (0)
#endif

#if __GCC__ >= 12
#define vmgexit "vmgexit"
#else
#define vmgexit "rep; vmmcall"
#endif

static __inline long __syscall0(long n)
{
	unsigned long ret;
	__syscall_wrapper(__volatile__(vmgexit : "=a"(ret) : "a"(n) : "rcx", "r11", "memory"),
					 __volatile__("syscall" : "=a"(ret) : "a"(n) : "rcx", "r11", "memory"));
	return ret;
}

static __inline long __syscall1(long n, long a1)
{
	unsigned long ret;
	__syscall_wrapper(__volatile__(vmgexit : "=a"(ret) : "a"(n), "D"(a1) : "rcx", "r11", "memory"),
					 __volatile__("syscall" : "=a"(ret) : "a"(n), "D"(a1) : "rcx", "r11", "memory"));
	return ret;
}

static __inline long __syscall2(long n, long a1, long a2)
{
	unsigned long ret;
	__syscall_wrapper(__volatile__(vmgexit : "=a"(ret) : "a"(n), "D"(a1), "S"(a2)
								   : "rcx", "r11", "memory"),
					  __volatile__("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2)
								   : "rcx", "r11", "memory"));
	return ret;
}

static __inline long __syscall3(long n, long a1, long a2, long a3)
{
	unsigned long ret;
	__syscall_wrapper(__volatile__(vmgexit : "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
														   "d"(a3) : "rcx", "r11", "memory"),
					  __volatile__("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
														   "d"(a3) : "rcx", "r11", "memory"));
	return ret;
}

static __inline long __syscall4(long n, long a1, long a2, long a3, long a4)
{
	unsigned long ret;
	register long r10 __asm__("r10") = a4;
	__syscall_wrapper(__volatile__(vmgexit : "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
														   "d"(a3), "r"(r10) : "rcx", "r11", "memory"),
					  __volatile__("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
														   "d"(a3), "r"(r10) : "rcx", "r11", "memory"));
	return ret;
}

static __inline long __syscall5(long n, long a1, long a2, long a3, long a4, long a5)
{
	unsigned long ret;
	register long r10 __asm__("r10") = a4;
	register long r8 __asm__("r8") = a5;
	__syscall_wrapper(__volatile__(vmgexit : "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
														   "d"(a3), "r"(r10), "r"(r8) : "rcx", "r11", "memory"),
					  __volatile__("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
														   "d"(a3), "r"(r10), "r"(r8) : "rcx", "r11", "memory"));
	return ret;
}

static __inline long __syscall6(long n, long a1, long a2, long a3, long a4, long a5, long a6)
{
	unsigned long ret;
	register long r10 __asm__("r10") = a4;
	register long r8 __asm__("r8") = a5;
	register long r9 __asm__("r9") = a6;
	__syscall_wrapper(__volatile__(vmgexit : "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
														   "d"(a3), "r"(r10), "r"(r8), "r"(r9) : "rcx", "r11", "memory"),
					  __volatile__("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
														   "d"(a3), "r"(r10), "r"(r8), "r"(r9) : "rcx", "r11", "memory"));
	return ret;
}

#define VDSO_USEFUL
#define VDSO_CGT_SYM "__vdso_clock_gettime"
#define VDSO_CGT_VER "LINUX_2.6"
#define VDSO_GETCPU_SYM "__vdso_getcpu"
#define VDSO_GETCPU_VER "LINUX_2.6"

#define IPC_64 0
