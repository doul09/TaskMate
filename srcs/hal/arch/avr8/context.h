/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file context.h
 * @brief context header declarations.
 *
 */

#ifndef AVR8_CONTEXT_H
#define AVR8_CONTEXT_H

/* ============================================================================
 * Includes
 * ========================================================================== */

#include <avr/io.h>

#include "hal/arch/avr8/arch_define.h" // Get stack_word_t

/* ============================================================================
 * Public definitions
 * ========================================================================== */

#define AVR8_CONTEXT_SAVE  \
	"push r0 \n\t"         \
	"in r0, __SREG__ \n\t" \
	"push r0 \n\t"         \
	"clr r1 \n\t"          \
	"push r1 \n\t"         \
	"push r2 \n\t"         \
	"push r3 \n\t"         \
	"push r4 \n\t"         \
	"push r5 \n\t"         \
	"push r6 \n\t"         \
	"push r7 \n\t"         \
	"push r8 \n\t"         \
	"push r9 \n\t"         \
	"push r10 \n\t"        \
	"push r11 \n\t"        \
	"push r12 \n\t"        \
	"push r13 \n\t"        \
	"push r14 \n\t"        \
	"push r15 \n\t"        \
	"push r16 \n\t"        \
	"push r17 \n\t"        \
	"push r18 \n\t"        \
	"push r19 \n\t"        \
	"push r20 \n\t"        \
	"push r21 \n\t"        \
	"push r22 \n\t"        \
	"push r23 \n\t"        \
	"push r24 \n\t"        \
	"push r25 \n\t"        \
	"push r26 \n\t"        \
	"push r27 \n\t"        \
	"push r28 \n\t"        \
	"push r29 \n\t"        \
	"push r30 \n\t"        \
	"push r31 \n\t"

#define AVR8_CONTEXT_RESTORE \
	"pop r31 \n\t"           \
	"pop r30 \n\t"           \
	"pop r29 \n\t"           \
	"pop r28 \n\t"           \
	"pop r27 \n\t"           \
	"pop r26 \n\t"           \
	"pop r25 \n\t"           \
	"pop r24 \n\t"           \
	"pop r23 \n\t"           \
	"pop r22 \n\t"           \
	"pop r21 \n\t"           \
	"pop r20 \n\t"           \
	"pop r19 \n\t"           \
	"pop r18 \n\t"           \
	"pop r17 \n\t"           \
	"pop r16 \n\t"           \
	"pop r15 \n\t"           \
	"pop r14 \n\t"           \
	"pop r13 \n\t"           \
	"pop r12 \n\t"           \
	"pop r11 \n\t"           \
	"pop r10 \n\t"           \
	"pop r9 \n\t"            \
	"pop r8 \n\t"            \
	"pop r7 \n\t"            \
	"pop r6 \n\t"            \
	"pop r5 \n\t"            \
	"pop r4 \n\t"            \
	"pop r3 \n\t"            \
	"pop r2 \n\t"            \
	"pop r1 \n\t"            \
	"pop r0 \n\t"            \
	"out __SREG__, r0 \n\t"  \
	"pop r0	\n\t"

/* ============================================================================
 * Public API
 * ========================================================================== */

static inline __attribute__((always_inline)) void hal_contextRestore(void)
{
	asm volatile(AVR8_CONTEXT_RESTORE);
}

void hal_threadContextInit(void (*func)(void), hal_stack_word_t **stack_pointer,
						   hal_stack_word_t *stack_top);

#endif // AVR8_CONTEXT_H
