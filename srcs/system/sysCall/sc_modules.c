/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file sc_modules.c
 * @brief Module and system syscall implementation.
 */

#include "sc_modules.h"

#include "hal/public/atomic.h"
#include "interfaces/tm_macros.h"
#include "interfaces/tm_modules.h"
#include "interfaces/tm_runLevel.h"
#include "system/sysCore/modules.h"
#include "system/sysCore/tm_scheduler.h"
#include "tm_libc/tm_string.h"

static mod_thread_item_t *sc_threadGetPointer(const char *name);

void sc_threadSetSTC(uint16_t count)
{
	hal_atomic_state_t state = hal_atomicStart();
	mod_threadSetSTC(count);
	hal_atomicEnd(state);
}

uint16_t sc_threadGetSTC(void)
{
	hal_atomic_state_t state = hal_atomicStart();
	uint16_t timer = mod_threadGetSTC();
	hal_atomicEnd(state);
	return timer;
}

uint16_t sc_threadGetCount(void) { return TM_MOD_THREAD_COUNT; }

bool sc_threadGetInfo(uint16_t id, const tm_string_t **name, uint8_t *run_level)
{
	if( (id >= TM_MOD_THREAD_COUNT) || (name == 0) || (run_level == 0) ) { return false; }

	hal_atomic_state_t state = hal_atomicStart();
	mod_thread_item_t *thread = mod_threadGetPointer((uint8_t)id);
	*name = thread->name;
	*run_level = RL_GET_RUN_LEVEL(thread->status);
	hal_atomicEnd(state);

	return *name != 0;
}

bool sc_threadStart(const char *name, uint8_t initial_run_level)
{
	mod_thread_item_t *thread = sc_threadGetPointer(name);
	if( thread == 0 ) { return false; }

	hal_atomic_state_t state = hal_atomicStart();

	if( thread->saved_run_level == RL_RUN_NONE ) { thread->saved_run_level = initial_run_level; }
	else
	{
		thread->status &= (uint8_t)~RL_LEVEL_MASK;
		thread->status |= thread->saved_run_level;
	}

	hal_atomicEnd(state);
	return true;
}

bool sc_threadStop(const char *name)
{
	mod_thread_item_t *thread = sc_threadGetPointer(name);
	if( thread == 0 ) { return false; }

	hal_atomic_state_t state = hal_atomicStart();
	uint8_t current_run_level = RL_GET_RUN_LEVEL(thread->status);

	thread->saved_run_level = current_run_level;
	thread->status &= (uint8_t)~RL_LEVEL_MASK;

	hal_atomicEnd(state);
	return true;
}

void sc_coopYield(void)
{
	hal_atomic_state_t state = hal_atomicStart();
	mod_thread_item_t *thread = mod_threadGetPointer(mod_threadGetCurrent());
	TM_SETBIT(thread->status, TM_MOD_THREAD_YIELDED);
	tm_schedulerCoop();
	hal_atomicEnd(state);
	while( TM_GETBIT(thread->status, TM_MOD_THREAD_YIELDED) );
}

static mod_thread_item_t *sc_threadGetPointer(const char *name)
{
	if( name == 0 ) { return 0; }

	for( uint8_t i = 0; i < TM_MOD_THREAD_COUNT; i++ )
	{
		mod_thread_item_t *thread = mod_threadGetPointer(i);
		if( (thread->name != 0) &&
			tm_strncmp(*thread->name, TM_STR_RAM(name), TM_MOD_NAME_SIZE_MAX) == 0 )
		{
			return thread;
		}
	}

	return 0;
}
