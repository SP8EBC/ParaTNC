#ifndef MACROS_H_
#define MACROS_H_


#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))

/**
 * Perform an integer division rounding the result to the nearest int value.
 * \note \a divisor should preferibly be a costant, otherwise this macro generates
 * 2 division. Also divisor is evaluated twice.
 */
#define DIV_ROUND(dividend, divisor)  (((dividend) + (divisor) / 2) / (divisor))

#ifndef BV
	/** Convert a bit value to a binary flag. */
	#define BV(x)  (1<<(x))
#endif

#define pgm_read8(a)     (*(const uint8_t  *)(a))
#define pgm_read16(a)    (*(const uint16_t *)(a))
#define pgm_read32(a)    (*(const uint32_t *)(a))


//IRQ

#define IRQ_PRIO_DISABLED	0x40

		#define IRQ_DISABLE						\
		({								\
			register uint32_t reg = IRQ_PRIO_DISABLED;		\
			asm volatile (						\
				"msr basepri, %0"				\
				: : "r"(reg) : "memory", "cc");			\
		})

#define IRQ_ENABLE						\
({								\
	register uint32_t reg = IRQ_PRIO_ENABLED;		\
	asm volatile (						\
		"msr basepri, %0"				\
		: : "r"(reg) : "memory", "cc");			\
})

#define CPU_READ_FLAGS()					\
({								\
	register uint32_t reg;				\
	asm volatile (						\
		"mrs %0, basepri"				\
		 : "=r"(reg) : : "memory", "cc");		\
	reg;							\
})

#define IRQ_SAVE_DISABLE(x)					\
({								\
	x = CPU_READ_FLAGS();					\
	IRQ_DISABLE;						\
})

#define IRQ_RESTORE(x)						\
({								\
	asm volatile (						\
		"msr basepri, %0"				\
		: : "r"(x) : "memory", "cc");			\
})


#define IRQ_ENABLED() ((CPU_READ_FLAGS() & 0xc0) != 0xc0)

/**
 * Execute \a CODE atomically with respect to interrupts.
 *
 * \see IRQ_SAVE_DISABLE IRQ_RESTORE
 */
#define ATOMIC(CODE) \
	do { \
		uint32_t __flags; \
		IRQ_SAVE_DISABLE(__flags); \
		CODE; \
		IRQ_RESTORE(__flags); \
	} while (0)

#endif /* MACROS_H_ */
