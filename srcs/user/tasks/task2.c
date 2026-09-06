/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file task2.c
 * @brief task2 implementation.
 *
 */

/* =============================================================================
 * Declarations - Include
 * ===========================================================================*/

#include "task2.h"

#include "system/sysCall/sc_gpio.h"
#include "system/sysCall/sc_modules.h"
#include "tm_libc/tm_stdio.h"

/* -----------------------------------------------
 * Task state
 * ---------------------------------------------*/

uint8_t task2_msg_channel;

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

void task2(void)
{
	sc_threadSetInitialized();

	while( 1 )
	{
		sc_gpio_signalToggle(GPIO_SIGNAL_TASK2_LED);

		sc_threadSetSTC(50);
		while( sc_threadGetSTC() > 0 );
	}
}
