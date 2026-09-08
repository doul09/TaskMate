/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file modules.h
 * @brief modules header declarations.
 */

#ifndef SYSCORE_MODULES_H
#define SYSCORE_MODULES_H

/* ============================================================================
 * Includes
 * ========================================================================== */

#include <stdint.h>

#include "hal/public/define.h" // Get architecture-specific types
#include "interfaces/tm_modules.h"
#include "interfaces/tm_string.h"

/* ============================================================================
 * Public definitions
 * ========================================================================== */

#define TM_MOD_CANARY 0xa5a5

typedef struct
{
	const tm_string_t *name;
	volatile uint8_t status;
	uint8_t saved_run_level;

	void (*main)(void);

	volatile uint16_t software_time_counter;

	hal_context_t context;

	uint16_t canary_low;
	hal_stack_word_t stack[TM_MOD_THREAD_STACK_SIZE];
	uint16_t canary_high;

} mod_thread_item_t;

// Driver
typedef struct
{
	const tm_string_t *name;
	uint8_t address;

	hal_driver_state_t (*control)(hal_driver_control_t, hal_driver_control_data_t *);

} mod_driver_item_t;

/* ============================================================================
 * Public API
 * ========================================================================== */

void mod_threadSetCurrent(uint8_t n);
uint8_t mod_threadGetCurrent(void);

void mod_threadSetSTC(uint16_t count);
uint16_t mod_threadGetSTC(void);

mod_driver_item_t *mod_driverGetPointer(uint8_t id);
mod_thread_item_t *mod_threadGetPointer(uint8_t id);

void mod_threadsAlloc(void);
void mod_driversAlloc(void);

#endif // SYSCORE_MODULES_H
