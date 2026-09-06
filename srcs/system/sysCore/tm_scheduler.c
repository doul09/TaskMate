/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file tm_scheduler.c
 * @brief tm scheduler implementation.
 *
 */

/* =============================================================================
 * Declarations - Include
 * ===========================================================================*/

#include "tm_scheduler.h"

#include <stdint.h>

#include "hal/public/context.h"
#include "hal/public/interrupt.h"
#include "hal/public/panic.h"
#include "hal/public/stack.h"
#include "interfaces/drv_timerSched.h"
#include "interfaces/tm_macros.h"
#include "interfaces/tm_modules.h"
#include "interfaces/tm_runLevel.h"
#include "system/sysCore/modules.h"
#include "system/sysCore/modules_list.h"
#include "tm_libc/tm_string.h"

/* -----------------------------------------------
 * Private function prototypes
 * ---------------------------------------------*/

static hal_timerSchedCallback_func_t tm_schedulerRR;
static mod_thread_item_t *tm_schedulerSelectNext(uint8_t current);

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

/* -----------------------------------------------
 * Scheduler lifecycle
 * ---------------------------------------------*/

void tm_schedulerInit(void)
{
	hal_timerSchedControl(DRV_CTRL_INIT, 0);
	hal_timerSchedSetCallback(tm_schedulerRR);
}

void tm_schedulerStart(void)
{
	hal_timerSchedControl(DRV_CTRL_START, 0);

	mod_thread_item_t *mod = mod_threadGetPointer(0);

	hal_setStackPointer(mod->stack_pointer);
	hal_contextRestore();
	hal_setGlobalInterrupt();
	hal_returnFromInterrupt();
}

/* -----------------------------------------------
 * Cooperative trigger
 * ---------------------------------------------*/

void tm_schedulerCoop(void) { hal_timerSchedLoad(); }

/* -----------------------------------------------
 * Round-robin policy
 * ---------------------------------------------*/

static void *tm_schedulerRR(void *stack_pointer)
{
	mod_thread_item_t *thread;

	// Save the current thread context
	thread = mod_threadGetPointer(mod_threadGetCurrent());
	thread->stack_pointer = stack_pointer;

	// Canary check
	if( thread->canary_low != TM_MOD_CANARY ) { panic(TM_STR("canary low 1")); }
	if( thread->canary_high != TM_MOD_CANARY ) { panic(TM_STR("canary high 1")); }

	// Switch threads
	thread = tm_schedulerSelectNext(mod_threadGetCurrent());

	// Canary check
	if( thread->canary_low != TM_MOD_CANARY ) { panic(TM_STR("canary low 2")); }
	if( thread->canary_high != TM_MOD_CANARY ) { panic(TM_STR("canary high 2")); }

	TM_CLEARBIT(thread->status, THREAD_BIT_YIELDED);
	return thread->stack_pointer;
}

static mod_thread_item_t *tm_schedulerSelectNext(uint8_t current)
{
	for( uint8_t count = 0; count < TM_MOD_THREAD_COUNT; count++ )
	{
		if( ++current == TM_MOD_THREAD_COUNT ) { current = 0; }

		mod_thread_item_t *thread = mod_threadGetPointer(current);
		if( RL_GET_RUN_LEVEL(thread->status) != RL_RUN_NONE )
		{
			mod_threadSetCurrent(current);
			return thread;
		}
	}

	panic(TM_STR("no runnable thread"));
}
