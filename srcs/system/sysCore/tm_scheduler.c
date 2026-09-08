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
#include "interfaces/drv_timerSched.h"
#include "interfaces/tm_macros.h"
#include "interfaces/tm_modules.h"
#include "interfaces/tm_runLevel.h"
#include "system/sysCore/modules.h"
#include "system/sysCore/modules_list.h"
#include "tm_libc/tm_string.h"

/* -----------------------------------------------
 * Private variables
 * ---------------------------------------------*/

static volatile uint8_t scheduler_run_level = RL_RUN_CORE;

/* -----------------------------------------------
 * Private function prototypes
 * ---------------------------------------------*/

static hal_timerSchedCallback_func_t tm_schedulerRR;
static mod_thread_item_t *tm_schedulerSelectNext(uint8_t current);

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

/* -----------------------------------------------
 * Scheduler life cycle
 * ---------------------------------------------*/

void tm_schedulerInit(void)
{
	scheduler_run_level = RL_RUN_CORE;
	hal_timerSchedControl(DRV_CTRL_INIT, 0);
	hal_timerSchedSetCallback(tm_schedulerRR);
}

void tm_schedulerStart(void)
{
	hal_timerSchedControl(DRV_CTRL_START, 0);

	mod_thread_item_t *mod = mod_threadGetPointer(0);

	hal_contextRestore(&mod->context);
	hal_setGlobalInterrupt();
	hal_returnFromInterrupt();
}

/* -----------------------------------------------
 * Run level
 * ---------------------------------------------*/

bool tm_schedulerRunLevelSet(uint8_t run_level)
{
	if( (run_level < RL_RUN_CORE) || (run_level >= RL_LEVEL_COUNT) ||
		(run_level < scheduler_run_level) )
	{
		return false;
	}

	scheduler_run_level = run_level;
	return true;
}

uint8_t tm_schedulerRunLevelGet(void) { return scheduler_run_level; }

/* -----------------------------------------------
 * Cooperative trigger
 * ---------------------------------------------*/

void tm_schedulerCoop(void) { hal_timerSchedLoad(); }

/* -----------------------------------------------
 * Round-robin policy
 * ---------------------------------------------*/

static hal_context_t *tm_schedulerRR(hal_context_t *context)
{
	mod_thread_item_t *thread;

	// Save the current thread context
	thread = mod_threadGetPointer(mod_threadGetCurrent());
	thread->context = *context;

	// Canary check
	if( thread->canary_low != TM_MOD_CANARY ) { panic(TM_STR("canary low 1")); }
	if( thread->canary_high != TM_MOD_CANARY ) { panic(TM_STR("canary high 1")); }

	// Switch threads
	thread = tm_schedulerSelectNext(mod_threadGetCurrent());

	// Canary check
	if( thread->canary_low != TM_MOD_CANARY ) { panic(TM_STR("canary low 2")); }
	if( thread->canary_high != TM_MOD_CANARY ) { panic(TM_STR("canary high 2")); }

	TM_CLEARBIT(thread->status, THREAD_BIT_YIELDED);
	return &thread->context;
}

static mod_thread_item_t *tm_schedulerSelectNext(uint8_t current)
{
	uint8_t active_run_level = scheduler_run_level;

	for( uint8_t count = 0; count < TM_MOD_THREAD_COUNT; count++ )
	{
		if( ++current == TM_MOD_THREAD_COUNT ) { current = 0; }

		mod_thread_item_t *thread = mod_threadGetPointer(current);
		uint8_t thread_run_level = RL_GET_RUN_LEVEL(thread->status);
		if( (thread_run_level != RL_RUN_NONE) && (thread_run_level <= active_run_level) )
		{
			mod_threadSetCurrent(current);
			return thread;
		}
	}

	panic(TM_STR("no runnable thread"));
}
