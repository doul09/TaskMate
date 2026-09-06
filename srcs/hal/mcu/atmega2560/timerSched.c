/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file timerSched.c
 * @brief timer sched implementation.
 *
 */

/* =============================================================================
 * Declarations - Include
 * ===========================================================================*/

#include "interfaces/drv_timerSched.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>

#include "hal/arch/avr8/context.h"
#include "hal/arch/avr8/interrupt.h"
#include "hal/arch/avr8/stack.h"
#include "interfaces/tm_define.h"
#include "interfaces/tm_macros.h"
#include "interfaces/tm_modules.h"
#include "interfaces/tm_runLevel.h"

/* -----------------------------------------------
 * Constants
 * ---------------------------------------------*/

const uint16_t TIMER1_OVERFLOW_COUNT = 2000; // Interrupt every 1ms (1.10^-3 x 16.10^6 )/8 = 2000

/* -----------------------------------------------
 * Private variables
 * ---------------------------------------------*/

static hal_timerSchedCallback_ptr_t sched_callback = NULL;
static hal_driver_status_t timer_sched_status;
static err_codes_t timer_sched_last_error = ERR_NO_ERROR;

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

/* -----------------------------------------------
 * Driver state
 * ---------------------------------------------*/

static hal_driver_state_t timerSchedSetError(err_codes_t error)
{
	timer_sched_last_error = error;
	return DRV_STATE_ERROR;
}

static hal_driver_state_t hal_timerSchedGetStatus(void)
{
	if( TM_GETBIT(timer_sched_status, DRV_BIT_DEAD) != 0 )
	{
		timer_sched_last_error = ERR_HAL_DRIVER_DEAD;
		return DRV_STATE_DEAD;
	}
	if( TM_GETBIT(timer_sched_status, DRV_BIT_ERROR) != 0 )
	{
		return timerSchedSetError(ERR_HAL_DRIVER_INVALID_STATE);
	}
	if( TM_GETBIT(timer_sched_status, DRV_BIT_INIT) == 0 )
	{
		if( TM_GETBIT(timer_sched_status, DRV_BIT_START) == 0 ) { return DRV_STATE_OFF; }
		return timerSchedSetError(ERR_HAL_DRIVER_INVALID_STATE);
	}
	if( TM_GETBIT(timer_sched_status, DRV_BIT_START) == 0 ) { return DRV_STATE_INITIALIZED; }
	return DRV_STATE_RUNNING;
}

static hal_driver_state_t timerSchedRequireRunning(void)
{
	hal_driver_state_t state = hal_timerSchedGetStatus();
	if( (state == DRV_STATE_OFF) || (state == DRV_STATE_INITIALIZED) )
	{
		return timerSchedSetError(ERR_HAL_DRIVER_NOT_RUNNING);
	}
	return state;
}

/* -----------------------------------------------
 * Callback and scheduler trigger
 * ---------------------------------------------*/

hal_driver_state_t hal_timerSchedSetCallback(hal_timerSchedCallback_ptr_t func_ptr)
{
	if( func_ptr == NULL ) { return timerSchedSetError(ERR_NULL_POINTER); }
	sched_callback = func_ptr;
	return hal_timerSchedGetStatus();
}

hal_driver_state_t hal_timerSchedLoad(void)
{
	hal_driver_state_t state = timerSchedRequireRunning();
	if( state != DRV_STATE_RUNNING ) { return state; }
#define LOAD_GUARD 4
	const uint16_t LOAD = TIMER1_OVERFLOW_COUNT - LOAD_GUARD;

	TCNT1 = LOAD;
	return DRV_STATE_RUNNING;
}

/* -----------------------------------------------
 * Driver lifecycle
 * ---------------------------------------------*/

static hal_driver_state_t hal_timerSchedInit(void)
{
	if( TM_GETBIT(timer_sched_status, DRV_BIT_DEAD) != 0 )
	{
		return timerSchedSetError(ERR_HAL_DRIVER_DEAD);
	}
	// Set up timer1 interrupt for scheduler
	TM_SETBIT(TCCR1B, WGM12); // CTC mode
	OCR1A = TIMER1_OVERFLOW_COUNT;
	TM_SETBIT(TIMSK1, OCIE1A); // output compare interrupt enable

	TM_SETBIT(timer_sched_status, DRV_BIT_INIT);
	timer_sched_last_error = ERR_NO_ERROR;
	return DRV_STATE_INITIALIZED;
}

#define TIMER_SCHED_START                              \
	"lds r24, %0\n\t"                                  \
	"ori r24, %1\n\t"                                  \
	"sts  %0, r24\n\t" : : "M"(_SFR_MEM_ADDR(TCCR1B)), \
						   "n"((uint8_t)(1u << CS11))  \
		: "r24"

static hal_driver_state_t hal_timerSchedStart(void)
{
	if( TM_GETBIT(timer_sched_status, DRV_BIT_DEAD) != 0 )
	{
		return timerSchedSetError(ERR_HAL_DRIVER_DEAD);
	}
	if( TM_GETBIT(timer_sched_status, DRV_BIT_INIT) == 0 )
	{

		return timerSchedSetError(ERR_HAL_DRIVER_NOT_INITIALIZED);
	}

	asm volatile(TIMER_SCHED_START);
	TM_SETBIT(timer_sched_status, DRV_BIT_START);
	return DRV_STATE_RUNNING;
}

#define TIMER_SCHED_STOP                              \
	"lds r24, %0\n\t"                                 \
	"andi r24, %1\n\t"                                \
	"sts  %0, r24\n\t"                                \
	"sts %2,r1 \n\t"                                  \
	"sts %3,r1 \n\t" : : "M"(_SFR_MEM_ADDR(TCCR1B)),  \
						 "n"((uint8_t)~(1u << CS11)), \
						 "M"(_SFR_MEM_ADDR(TCNT1H)),  \
						 "M"(_SFR_MEM_ADDR(TCNT1L))   \
		: "r24"

static hal_driver_state_t hal_timerSchedStop(void)
{
	asm volatile(TIMER_SCHED_STOP);
	TM_CLEARBIT(timer_sched_status, DRV_BIT_START);
	return hal_timerSchedGetStatus();
}

/* -----------------------------------------------
 * Context-switch interrupt
 * ---------------------------------------------*/

#define TM_SCHED_CALLBACK                        \
	"in r24, 0x3d \n\t"                          \
	"in r25, 0x3e \n\t"                          \
	"lds r30, %0 \n\t"                           \
	"lds r31, %0+1 \n\t"                         \
	"movw r28,r24 \n\t"                          \
	"sbiw r30,0x00 \n\t"                         \
	"breq .+4 \n\t"                              \
	"eicall \n\t"                                \
	"out 0x3e, r25 \n\t"                         \
	"out 0x3d, r24 \n\t" : : "m"(sched_callback) \
		: "r24", "r25", "r30", "r31", "r28", "r29"

ISR(TIMER1_COMPA_vect, ISR_NAKED)
{
	asm volatile(AVR8_CONTEXT_SAVE);
	asm volatile(TIMER_SCHED_STOP);

	asm volatile(TM_SCHED_CALLBACK);

	asm volatile(TIMER_SCHED_START);
	asm volatile(AVR8_CONTEXT_RESTORE);
	asm volatile("reti \n\t");
}

/* -----------------------------------------------
 * Driver control
 * ---------------------------------------------*/

hal_driver_state_t hal_timerSchedControl(hal_driver_control_t command,
										 hal_driver_control_data_t *data)
{
	switch( command )
	{
		case DRV_CTRL_INIT:
			return hal_timerSchedInit();
		case DRV_CTRL_START:
			return hal_timerSchedStart();
		case DRV_CTRL_STOP:
			return hal_timerSchedStop();
		case DRV_CTRL_RLSET:
			if( data == 0 ) { return timerSchedSetError(ERR_NULL_POINTER); }
			if( data->run_level >= RL_LEVEL_COUNT )
			{
				return timerSchedSetError(ERR_HAL_DRIVER_INVALID_VALUE);
			}
			timer_sched_status &= (hal_driver_status_t)~RL_LEVEL_MASK;
			timer_sched_status |= data->run_level;
			return hal_timerSchedGetStatus();
		case DRV_CTRL_RLGET:
			if( data == 0 ) { return timerSchedSetError(ERR_NULL_POINTER); }
			data->run_level = timer_sched_status & RL_LEVEL_MASK;
			return hal_timerSchedGetStatus();
		case DRV_CTRL_SETBIT:
			if( data == 0 ) { return timerSchedSetError(ERR_NULL_POINTER); }
			if( (data->status_bit < DRV_BIT_INIT) || (data->status_bit > DRV_BIT_DEAD) )
			{
				return timerSchedSetError(ERR_HAL_DRIVER_INVALID_VALUE);
			}
			TM_SETBIT(timer_sched_status, data->status_bit);
			return hal_timerSchedGetStatus();
		case DRV_CTRL_CLEARBIT:
			if( data == 0 ) { return timerSchedSetError(ERR_NULL_POINTER); }
			if( (data->status_bit < DRV_BIT_INIT) || (data->status_bit > DRV_BIT_DEAD) )
			{
				return timerSchedSetError(ERR_HAL_DRIVER_INVALID_VALUE);
			}
			TM_CLEARBIT(timer_sched_status, data->status_bit);
			return hal_timerSchedGetStatus();
		case DRV_CTRL_GETBIT:
			if( data == 0 ) { return timerSchedSetError(ERR_NULL_POINTER); }
			if( (data->status_bit < DRV_BIT_INIT) || (data->status_bit > DRV_BIT_DEAD) )
			{
				return timerSchedSetError(ERR_HAL_DRIVER_INVALID_VALUE);
			}
			data->bit_value = TM_GETBIT(timer_sched_status, data->status_bit) != 0;
			return hal_timerSchedGetStatus();
		case DRV_CTRL_GETSTATUS:
			return hal_timerSchedGetStatus();
		case DRV_CTRL_GETLASTERROR:
			if( data == 0 ) { return timerSchedSetError(ERR_NULL_POINTER); }
			data->error = timer_sched_last_error;
			return hal_timerSchedGetStatus();
		default:
			return timerSchedSetError(ERR_HAL_DRIVER_INVALID_CONTROL);
	}
}
