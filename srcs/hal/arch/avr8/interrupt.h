/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file interrupt.h
 * @brief interrupt header declarations.
 *
 */

#ifndef AVR8_INTERRUPT_H
#define AVR8_INTERRUPT_H

/* ============================================================================
 * Includes
 * ========================================================================== */

#include <avr/io.h>

/* ============================================================================
 * Public API
 * ========================================================================== */

static inline __attribute__((always_inline)) void hal_returnFromInterrupt(void)
{
	asm volatile("reti \n\t");
}

static inline __attribute__((always_inline)) void hal_setGlobalInterrupt(void)
{
	asm volatile("sei \n\t");
}

static inline __attribute__((always_inline)) void hal_clearGlobalInterrupt(void)
{
	asm volatile("cli \n\t");
}

#endif // AVR8_INTERRUPT_H
