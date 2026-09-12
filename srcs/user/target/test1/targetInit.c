/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file targetInit.c
 * @brief test1 init implementation.
 *
 */

/* =============================================================================
 * Declarations - Include
 * ===========================================================================*/

#include "targetInit.h"

#include <avr/io.h>
#include <stdbool.h>

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

void targetInit(void){}

void targetWireSignal(hal_signal_t *table, gpio_signal_t signal)
{
	// Set default values for outputs
	table[signal].pin.mode = GPIO_PIN_MODE_OUTPUT_PP;
	table[signal].pin.pull = GPIO_PIN_PULL_NONE;
	table[signal].active_high = true;

	if( signal == GPIO_SIGNAL_INBOARD_LED )
	{
		table[signal].pin.port = PORT_B;
		table[signal].pin.number = PB7;
		return;
	}

	if( signal == GPIO_SIGNAL_TASK1_LED )
	{
		table[signal].pin.port = PORT_A;
		table[signal].pin.number = PA0;
		return;
	}

	if( signal == GPIO_SIGNAL_TASK2_LED )
	{
		table[signal].pin.port = PORT_A;
		table[signal].pin.number = PA1;
		return;
	}

	// Set default values for inputs
}
