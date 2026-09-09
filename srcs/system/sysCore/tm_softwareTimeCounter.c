/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file tm_softwareTimeCounter.c
 * @brief tm software time counter implementation.
 *
 */

/* =============================================================================
 * Declarations - Include
 * ===========================================================================*/

#include "tm_softwareTimeCounter.h"

#include "interfaces/drv_timerSTC.h"
#include "system/sysCore/modules.h"

/* -----------------------------------------------
 * Private function prototypes
 * ---------------------------------------------*/

static void tm_softwareTimeCounter(void);

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

void tm_softwareTimeCounterInit(void) { hal_timerSTCSetCallback(tm_softwareTimeCounter); }

void tm_softwareTimeCounter(void)
{
	// Decrement the RTC
	for( uint8_t i = 0; i < MOD_THREAD_COUNT; i++ )
	{
		mod_thread_item_t *mod = mod_threadGetPointer(i);
		if( mod->software_time_counter > 0 ) { mod->software_time_counter--; }
	}
}
