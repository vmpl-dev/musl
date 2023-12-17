#define __SYSCALL_LL_E(x) (x)
#define __SYSCALL_LL_O(x) (x)

#define MSR_PROTOCOL 1
#ifdef MSR_PROTOCOL
#define GHCB_MSR 0xc0010130

/// 0xfff
#define GHCB_MSR_INFO_MASK 0xfff

#define GHCB_MSR_INFO(x) ((x) & GHCB_MSR_INFO_MASK)

#define GHCB_MSR_DATA(x) ((x) & ~GHCB_MSR_INFO_MASK)

/* GHCB Run at VMPL Request/Response */
/// 0x16
#define GHCB_MSR_VMPL_REQ 0x016
#define GHCB_MSR_VMPL_REQ_LEVEL(x) ((x) | GHCB_MSR_VMPL_REQ)
/// 0x17
#define GHCB_MSR_VMPL_RES 0x017
#define GHCB_MSR_VMPL_RESP_VAL(v) (v >> 32)

// Read MSR
static inline unsigned long __rdmsr(unsigned int msr) {
    unsigned int lo;
    unsigned int hi;

    __asm__ __volatile__("rdmsr"
                         : "=a"(lo), "=d"(hi)
                         : "c"(msr)
                         : "memory");

    return ((unsigned long)hi << 32) | lo;
}

// Write to MSR a given value
static inline void __wrmsr(unsigned int msr, unsigned long value) {
    unsigned int lo = value;
    unsigned int hi = value >> 32;

    __asm__ __volatile__("wrmsr"
                         :
                         : "c"(msr), "a"(lo), "d"(hi)
                         : "memory");
}

#define __syscall_wrapper(__vmgexit, __syscall)            \
	do                                                     \
	{                                                      \
		unsigned short cs;                                 \
		__asm__ __volatile__("movw %%cs, %0" : "=r"(cs));  \
		if ((cs & 0x3) == 0)                               \
		{                                                  \
			__wrmsr(GHCB_MSR, GHCB_MSR_VMPL_REQ_LEVEL(0)); \
			__asm__ __vmgexit;                             \
		}                                                  \
		else                                               \
		{                                                  \
			__asm__ __syscall;                             \
		}                                                  \
	} while (0)
#elif GHCB_PROTOCOL
struct ghcb_save_area {
	u8 reserved_0x0[912];
	u64 sw_exit_code;
	u64 sw_exit_info_1;
	u64 sw_exit_info_2;
	u64 sw_scratch;
	u8 reserved_0x3b0[64];
	u8 valid_bitmap[16];
	u64 x87_state_gpa;
} __packed;

#define GHCB_SHARED_BUF_SIZE	2032

struct ghcb {
	struct ghcb_save_area save;
	u8 reserved_save[2048 - sizeof(struct ghcb_save_area)];

	u8 shared_buffer[GHCB_SHARED_BUF_SIZE];

	u8 reserved_0xff0[10];
	u16 protocol_version;	/* negotiated SEV-ES/GHCB protocol version */
	u32 ghcb_usage;
} __packed;

/* GHCB Accessor functions */

#define GHCB_BITMAP_IDX(field)							\
	(offsetof(struct ghcb_save_area, field) / sizeof(u64))

#define DEFINE_GHCB_ACCESSORS(field)						\
	static __always_inline bool ghcb_##field##_is_valid(const struct ghcb *ghcb) \
	{									\
		return test_bit(GHCB_BITMAP_IDX(field),				\
				(unsigned long *)&ghcb->save.valid_bitmap);	\
	}									\
										\
	static __always_inline u64 ghcb_get_##field(struct ghcb *ghcb)		\
	{									\
		return ghcb->save.field;					\
	}									\
										\
	static __always_inline u64 ghcb_get_##field##_if_valid(struct ghcb *ghcb) \
	{									\
		return ghcb_##field##_is_valid(ghcb) ? ghcb->save.field : 0;	\
	}									\
										\
	static __always_inline void ghcb_set_##field(struct ghcb *ghcb, u64 value) \
	{									\
		__set_bit(GHCB_BITMAP_IDX(field),				\
			  (unsigned long *)&ghcb->save.valid_bitmap);		\
		ghcb->save.field = value;					\
	}

DEFINE_GHCB_ACCESSORS(sw_exit_code)
DEFINE_GHCB_ACCESSORS(sw_exit_info_1)
DEFINE_GHCB_ACCESSORS(sw_exit_info_2)

/**
 * lower_32_bits - return bits 0-31 of a number
 * @n: the number we're accessing
 */
#define lower_32_bits(n) ((u32)((n) & 0xffffffff))

extern struct ghcb *ghcb;

#define __syscall_wrapper(__vmgexit, __syscall)             \
	do                                                      \
	{                                                       \
		unsigned short cs;                                  \
		__asm__ __volatile__("movw %%cs, %0" : "=r"(cs));   \
		if ((cs & 0x3) == 0)                                \
		{                                                   \
			uint64_t sw_exit_info_1;                        \
			ghcb->protocol_version = GHCB_PROTOCOL_MIN;     \
			ghcb->ghcb_usage = GHCB_DEFAULT_USAGE;          \
			ghcb_set_sw_exit_code(ghcb, code);              \
			ghcb_set_sw_exit_info_1(ghcb, info1);           \
			ghcb_set_sw_exit_info_2(ghcb, info2);           \
			__asm__ __vmgexit;                              \
			sw_exit_info_1 = ghcb_get_sw_exit_info_1(ghcb); \
			if (!ghcb_sw_exit_info_1_is_valid(ghcb))        \
				return -EINVAL;                             \
			if (lower_32_bits(sw_exit_info_1) != 0)         \
				return -EINVAL;                             \
		}                                                   \
		else                                                \
		{                                                   \
			__asm__ __syscall;                              \
		}                                                   \
	} while (0);
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
