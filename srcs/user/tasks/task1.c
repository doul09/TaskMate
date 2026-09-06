/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file task1.c
 * @brief task1 implementation.
 *
 */

/* =============================================================================
 * Declarations - Include
 * ===========================================================================*/

#include "task1.h"

#include "system/sysCall/sc_gpio.h"
#include "system/sysCall/sc_modules.h"
#include "tm_libc/tm_stdio.h"

/* -----------------------------------------------
 * Task state
 * ---------------------------------------------*/

uint8_t task1_msg_channel;

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

void task1(void)
{
	while( 1 )
	{

		sc_gpio_signalToggle(GPIO_SIGNAL_TASK1_LED);

		sc_threadSetSTC(50);
		while( sc_threadGetSTC() > 0 );
	}
}
