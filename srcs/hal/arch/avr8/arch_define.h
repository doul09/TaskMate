/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file arch_define.h
 * @brief arch define header declarations.
 *
 */

#ifndef AVR8_ARCH_DEFINE_H
#define AVR8_ARCH_DEFINE_H

/* ============================================================================
 * Includes
 * ========================================================================== */

#include <stdint.h>

/* ============================================================================
 * Public definitions
 * ========================================================================== */

/* -----------------------------------------------
 * Architecture constants
 * ---------------------------------------------*/

#define AVR8_REGISTER_COUNT 32 // from R0 to R31

/* -----------------------------------------------
 * Architecture types
 * ---------------------------------------------*/

typedef uint8_t hal_stack_word_t;
typedef uint8_t hal_atomic_state_t;

typedef struct hal_context
{
	hal_stack_word_t *stack_pointer;
} hal_context_t;

typedef struct
{
	volatile uint8_t *ddr;
	volatile uint8_t *port;
	volatile uint8_t *pin;
} hal_port_t;

#endif // AVR8_ARCH_DEFINE_H
