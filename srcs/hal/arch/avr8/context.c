/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file context.c
 * @brief context implementation.
 *
 */

/* =============================================================================
 * Declarations - Include
 * ===========================================================================*/

#include "context.h"

#include <avr/interrupt.h>
#include <avr/io.h>

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

// NOLINTBEGIN
// NOLINT(readability-magic-numbers)

void hal_threadContextInit(void (*func)(void), hal_stack_word_t **stack_pointer,
						   hal_stack_word_t *stack_top)
{
	// Stack initialisation
	hal_stack_word_t *sp = stack_top;

	*(sp--) = (uint8_t)((uintptr_t)func & 0xFF); // PCL;
	*(sp--) = (uint8_t)(((uintptr_t)func >> 8u) & 0xFF); // PCH
	*(sp--) = 0x00; // PCHH is always 0 if the flash code size is below 128 kB
	*(sp--) = 0x00; // R0
	*(sp--) = SREG;

	// Registers R1-R31
	for( uint8_t i = 1; i < AVR8_REGISTER_COUNT; i++ ) { *(sp--) = 0x00; }

	*stack_pointer = sp;
}
// NOLINTEND
