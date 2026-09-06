/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file timerSTC.c
 * @brief timer stc implementation.
 *
 */

/* =============================================================================
 * Declarations - Include
 * ===========================================================================*/

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>

#include "interfaces/drv_timerSTC.h"
#include "interfaces/tm_define.h"
#include "interfaces/tm_macros.h"
#include "interfaces/tm_modules.h"
#include "interfaces/tm_runLevel.h"

/* -----------------------------------------------
 * Constants
 * ---------------------------------------------*/

const uint16_t hal_timerSTC_OVERFLOW_COUNT =
	625; // Interrupt every 10ms (10.10^-3 x 16.10^6 )/256 = 625

/* -----------------------------------------------
 * Private variables
 * ---------------------------------------------*/

static hal_timerSTCCallback_t stc_callback = NULL;
static hal_driver_status_t timer_stc_status;
static err_codes_t timer_stc_last_error = ERR_NO_ERROR;

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

/* -----------------------------------------------
 * Driver state
 * ---------------------------------------------*/

static hal_driver_state_t timerSTCSetError(err_codes_t error)
{
	timer_stc_last_error = error;
	return DRV_STATE_ERROR;
}

static hal_driver_state_t hal_timerSTCGetStatus(void)
{
	if( TM_GETBIT(timer_stc_status, DRV_BIT_DEAD) != 0 )
	{
		timer_stc_last_error = ERR_HAL_DRIVER_DEAD;
		return DRV_STATE_DEAD;
	}
	if( TM_GETBIT(timer_stc_status, DRV_BIT_ERROR) != 0 )
	{
		return timerSTCSetError(ERR_HAL_DRIVER_INVALID_STATE);
	}
	if( TM_GETBIT(timer_stc_status, DRV_BIT_INIT) == 0 )
	{
		if( TM_GETBIT(timer_stc_status, DRV_BIT_START) == 0 ) { return DRV_STATE_OFF; }
		return timerSTCSetError(ERR_HAL_DRIVER_INVALID_STATE);
	}
	if( TM_GETBIT(timer_stc_status, DRV_BIT_START) == 0 ) { return DRV_STATE_INITIALIZED; }
	return DRV_STATE_RUNNING;
}

/* -----------------------------------------------
 * Callback configuration
 * ---------------------------------------------*/

hal_driver_state_t hal_timerSTCSetCallback(hal_timerSTCCallback_t func_ptr)
{
	if( func_ptr == NULL ) { return timerSTCSetError(ERR_NULL_POINTER); }
	stc_callback = func_ptr;
	return hal_timerSTCGetStatus();
}

/* -----------------------------------------------
 * Driver life cycle
 * ---------------------------------------------*/

static hal_driver_state_t hal_timerSTCInit(void)
{
	if( TM_GETBIT(timer_stc_status, DRV_BIT_DEAD) != 0 )
	{
		return timerSTCSetError(ERR_HAL_DRIVER_DEAD);
	}

	// Set up timer3 for RTC
	TM_WRITEBIT(TCCR3B, WGM32, CS32); // CTC mode, prescaler 256
	OCR3A = hal_timerSTC_OVERFLOW_COUNT;

	TM_SETBIT(timer_stc_status, DRV_BIT_INIT);
	timer_stc_last_error = ERR_NO_ERROR;
	return DRV_STATE_INITIALIZED;
}

static hal_driver_state_t hal_timerSTCStart(void)
{
	if( TM_GETBIT(timer_stc_status, DRV_BIT_DEAD) != 0 )
	{

		return timerSTCSetError(ERR_HAL_DRIVER_DEAD);
	}
	if( TM_GETBIT(timer_stc_status, DRV_BIT_INIT) == 0 )
	{
		return timerSTCSetError(ERR_HAL_DRIVER_NOT_INITIALIZED);
	}

	// Start by enabling the interrupt
	TM_SETBIT(TIMSK3, OCIE3A);

	TM_SETBIT(timer_stc_status, DRV_BIT_START);
	return DRV_STATE_RUNNING;
}

static hal_driver_state_t hal_timerSTCStop(void)
{
	// Stop by disabling the interrupt
	TM_CLEARBIT(TIMSK3, OCIE3A);

	TM_CLEARBIT(timer_stc_status, DRV_BIT_START);
	return hal_timerSTCGetStatus();
}

/* -----------------------------------------------
 * Timer interrupt
 * ---------------------------------------------*/

ISR(TIMER3_COMPA_vect)
{
	// Software time counter callback
	if( stc_callback != NULL ) { stc_callback(); }
}

/* -----------------------------------------------
 * Driver control
 * ---------------------------------------------*/

hal_driver_state_t hal_timerSTCControl(hal_driver_control_t command,
									   hal_driver_control_data_t *data)
{
	switch( command )
	{
		case DRV_CTRL_INIT:
			return hal_timerSTCInit();
		case DRV_CTRL_START:
			return hal_timerSTCStart();
		case DRV_CTRL_STOP:
			return hal_timerSTCStop();
		case DRV_CTRL_RLSET:
			if( data == 0 ) { return timerSTCSetError(ERR_NULL_POINTER); }
			if( data->run_level >= RL_LEVEL_COUNT )
			{
				return timerSTCSetError(ERR_HAL_DRIVER_INVALID_VALUE);
			}
			timer_stc_status &= (hal_driver_status_t)~RL_LEVEL_MASK;
			timer_stc_status |= data->run_level;
			return hal_timerSTCGetStatus();
		case DRV_CTRL_RLGET:
			if( data == 0 ) { return timerSTCSetError(ERR_NULL_POINTER); }
			data->run_level = timer_stc_status & RL_LEVEL_MASK;
			return hal_timerSTCGetStatus();
		case DRV_CTRL_SETBIT:
			if( data == 0 ) { return timerSTCSetError(ERR_NULL_POINTER); }
			if( (data->status_bit < DRV_BIT_INIT) || (data->status_bit > DRV_BIT_DEAD) )
			{
				return timerSTCSetError(ERR_HAL_DRIVER_INVALID_VALUE);
			}
			TM_SETBIT(timer_stc_status, data->status_bit);
			return hal_timerSTCGetStatus();
		case DRV_CTRL_CLEARBIT:
			if( data == 0 ) { return timerSTCSetError(ERR_NULL_POINTER); }
			if( (data->status_bit < DRV_BIT_INIT) || (data->status_bit > DRV_BIT_DEAD) )
			{
				return timerSTCSetError(ERR_HAL_DRIVER_INVALID_VALUE);
			}
			TM_CLEARBIT(timer_stc_status, data->status_bit);
			return hal_timerSTCGetStatus();
		case DRV_CTRL_GETBIT:
			if( data == 0 ) { return timerSTCSetError(ERR_NULL_POINTER); }
			if( (data->status_bit < DRV_BIT_INIT) || (data->status_bit > DRV_BIT_DEAD) )
			{
				return timerSTCSetError(ERR_HAL_DRIVER_INVALID_VALUE);
			}
			data->bit_value = TM_GETBIT(timer_stc_status, data->status_bit) != 0;
			return hal_timerSTCGetStatus();
		case DRV_CTRL_GETSTATUS:
			return hal_timerSTCGetStatus();
		case DRV_CTRL_GETLASTERROR:
			if( data == 0 ) { return timerSTCSetError(ERR_NULL_POINTER); }
			data->error = timer_stc_last_error;
			return hal_timerSTCGetStatus();
		default:

			return timerSTCSetError(ERR_HAL_DRIVER_INVALID_CONTROL);
	}
}
