/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file rtc_ZS042.c
 * @brief rtc zs042 implementation.
 *
 */

/* =============================================================================
 * Declarations - Include
 * ===========================================================================*/

#include "interfaces/drv_rtc.h"

#include "interfaces/drv_i2c.h"
#include "interfaces/tm_macros.h"
#include "interfaces/tm_modules.h"
#include "interfaces/tm_runLevel.h"

// NOLINTBEGIN
// NOLINT(readability-magic-numbers)

/* -----------------------------------------------
 * Constants
 * ---------------------------------------------*/

#define ZS042_I2C_ADDR 0x68

/* -----------------------------------------------
 * Private variables
 * ---------------------------------------------*/

static uint8_t buf[7];
static hal_driver_status_t rtc_status;
static err_codes_t rtc_last_error = ERR_NO_ERROR;

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

/* -----------------------------------------------
 * Time conversion
 * ---------------------------------------------*/

static uint8_t bcdToBin(uint8_t bcd) { return (uint8_t)((bcd >> 4) * 10u) + (bcd & 0x0Fu); }
static uint8_t binToBcd(uint8_t val) { return (uint8_t)((val / 10u) << 4) | (val % 10u); }

/* -----------------------------------------------
 * Driver lifecycle
 * ---------------------------------------------*/

static hal_driver_state_t rtcSetError(err_codes_t error)
{
	rtc_last_error = error;
	return DRV_STATE_ERROR;
}

static hal_driver_state_t hal_rtcGetStatus(void)
{
	if( TM_GETBIT(rtc_status, DRV_BIT_DEAD) != 0 )
	{
		rtc_last_error = ERR_HAL_DRIVER_DEAD;
		return DRV_STATE_DEAD;
	}
	if( TM_GETBIT(rtc_status, DRV_BIT_ERROR) != 0 )
	{
		return rtcSetError(ERR_HAL_DRIVER_INVALID_STATE);
	}
	if( hal_i2cControl(DRV_CTRL_GETSTATUS, 0) != DRV_STATE_RUNNING )
	{
		return rtcSetError(ERR_HAL_DRIVER_DEPENDENCY);
	}
	if( TM_GETBIT(rtc_status, DRV_BIT_INIT) == 0 )
	{
		if( TM_GETBIT(rtc_status, DRV_BIT_START) == 0 ) { return DRV_STATE_OFF; }
		return rtcSetError(ERR_HAL_DRIVER_INVALID_STATE);
	}
	if( TM_GETBIT(rtc_status, DRV_BIT_START) == 0 ) { return DRV_STATE_INITIALIZED; }
	return DRV_STATE_RUNNING;
}

static hal_driver_state_t rtcRequireRunning(void)
{
	hal_driver_state_t state = hal_rtcGetStatus();
	if( (state == DRV_STATE_OFF) || (state == DRV_STATE_INITIALIZED) )
	{
		return rtcSetError(ERR_HAL_DRIVER_NOT_RUNNING);
	}
	return state;
}

static hal_driver_state_t hal_rtcInit(void)
{
	if( TM_GETBIT(rtc_status, DRV_BIT_DEAD) != 0 ) { return rtcSetError(ERR_HAL_DRIVER_DEAD); }
	if( hal_i2cControl(DRV_CTRL_GETSTATUS, 0) != DRV_STATE_RUNNING )
	{

		return rtcSetError(ERR_HAL_DRIVER_DEPENDENCY);
	}
	TM_SETBIT(rtc_status, DRV_BIT_INIT);
	rtc_last_error = ERR_NO_ERROR;
	return DRV_STATE_INITIALIZED;
}

static hal_driver_state_t hal_rtcStart(void)
{
	if( TM_GETBIT(rtc_status, DRV_BIT_DEAD) != 0 ) { return rtcSetError(ERR_HAL_DRIVER_DEAD); }
	if( TM_GETBIT(rtc_status, DRV_BIT_INIT) == 0 )
	{
		return rtcSetError(ERR_HAL_DRIVER_NOT_INITIALIZED);
	}

	TM_SETBIT(rtc_status, DRV_BIT_START);
	return DRV_STATE_RUNNING;
}

static hal_driver_state_t hal_rtcStop(void)
{
	TM_CLEARBIT(rtc_status, DRV_BIT_START);
	return hal_rtcGetStatus();
}

/* -----------------------------------------------
 * RTC operations
 * ---------------------------------------------*/

hal_driver_state_t hal_rtcRead(hal_rtc_time_t *time)
{
	hal_driver_state_t state = rtcRequireRunning();
	if( state != DRV_STATE_RUNNING ) { return state; }
	if( time == 0 ) { return rtcSetError(ERR_NULL_POINTER); }
	if( hal_i2cCommStart(ZS042_I2C_ADDR, HAL_I2C_WRITE) == DRV_STATE_ERROR )
	{
		return rtcSetError(ERR_HAL_DRIVER_DEPENDENCY);
	}
	if( hal_i2cWrite(0x00) == DRV_STATE_ERROR ) { return rtcSetError(ERR_HAL_DRIVER_DEPENDENCY); }

	if( hal_i2cCommStart(ZS042_I2C_ADDR, HAL_I2C_READ) == DRV_STATE_ERROR )
	{
		return rtcSetError(ERR_HAL_DRIVER_DEPENDENCY);
	}

	for( uint8_t i = 0; i < 6; i++ )
	{
		if( hal_i2cRead(&buf[i], HAL_I2C_ACK) == DRV_STATE_ERROR )
		{
			return rtcSetError(ERR_HAL_DRIVER_DEPENDENCY);
		}
	}
	if( hal_i2cRead(&buf[6], HAL_I2C_NACK) == DRV_STATE_ERROR )
	{
		return rtcSetError(ERR_HAL_DRIVER_DEPENDENCY);
	}

	if( hal_i2cCommStop() == DRV_STATE_ERROR ) { return rtcSetError(ERR_HAL_DRIVER_DEPENDENCY); }

	time->seconds = bcdToBin(buf[0] & 0x7F); // bit 7 = Clock Halt, ignored
	time->minutes = bcdToBin(buf[1]);
	time->hours = bcdToBin(buf[2] & 0x3F); // 24 hour
	time->weekday = bcdToBin(buf[3]);
	time->day = bcdToBin(buf[4]);
	time->month = bcdToBin(buf[5] & 0x1F); // bit 7 = century
	time->year = bcdToBin(buf[6]);

	return DRV_STATE_RUNNING;
}

hal_driver_state_t hal_rtcWrite(const hal_rtc_time_t *time)
{
	hal_driver_state_t state = rtcRequireRunning();
	if( state != DRV_STATE_RUNNING ) { return state; }
	if( time == 0 ) { return rtcSetError(ERR_NULL_POINTER); }
	if( (time->seconds > 59) || (time->minutes > 59) || (time->hours > 23) || (time->weekday < 1) ||
		(time->weekday > 7) || (time->day < 1) || (time->day > 31) || (time->month < 1) ||
		(time->month > 12) || (time->year > 99) )
	{
		return rtcSetError(ERR_HAL_RTC_TIME_OUT_OF_RANGE);
	}
	buf[0] = binToBcd(time->seconds & 0x7F); // bit 7 = 0, clock ON
	buf[1] = binToBcd(time->minutes);
	buf[2] = binToBcd(time->hours) & 0x3F; // mode 24h
	buf[3] = binToBcd(time->weekday);
	buf[4] = binToBcd(time->day);
	buf[5] = binToBcd(time->month & 0x1F);
	buf[6] = binToBcd(time->year);

	if( hal_i2cCommStart(ZS042_I2C_ADDR, HAL_I2C_WRITE) == DRV_STATE_ERROR )
	{
		return rtcSetError(ERR_HAL_DRIVER_DEPENDENCY);
	}
	if( hal_i2cWrite(0x00) == DRV_STATE_ERROR ) { return rtcSetError(ERR_HAL_DRIVER_DEPENDENCY); }
	for( uint8_t i = 0; i < 7; i++ )
	{
		if( hal_i2cWrite(buf[i]) == DRV_STATE_ERROR )
		{
			return rtcSetError(ERR_HAL_DRIVER_DEPENDENCY);
		}
	}
	if( hal_i2cCommStop() == DRV_STATE_ERROR ) { return rtcSetError(ERR_HAL_DRIVER_DEPENDENCY); }

	return DRV_STATE_RUNNING;
}

/* -----------------------------------------------
 * Driver control
 * ---------------------------------------------*/

hal_driver_state_t hal_rtcControl(hal_driver_control_t command, hal_driver_control_data_t *data)
{
	switch( command )
	{
		case DRV_CTRL_INIT:
			return hal_rtcInit();
		case DRV_CTRL_START:
			return hal_rtcStart();
		case DRV_CTRL_STOP:
			return hal_rtcStop();
		case DRV_CTRL_RLSET:
			if( data == 0 ) { return rtcSetError(ERR_NULL_POINTER); }
			if( data->run_level >= RL_LEVEL_COUNT )
			{
				return rtcSetError(ERR_HAL_DRIVER_INVALID_VALUE);
			}
			rtc_status &= (hal_driver_status_t)~RL_LEVEL_MASK;
			rtc_status |= data->run_level;
			return hal_rtcGetStatus();
		case DRV_CTRL_RLGET:
			if( data == 0 ) { return rtcSetError(ERR_NULL_POINTER); }
			data->run_level = rtc_status & RL_LEVEL_MASK;
			return hal_rtcGetStatus();
		case DRV_CTRL_SETBIT:
			if( data == 0 ) { return rtcSetError(ERR_NULL_POINTER); }
			if( (data->status_bit < DRV_BIT_INIT) || (data->status_bit > DRV_BIT_DEAD) )
			{
				return rtcSetError(ERR_HAL_DRIVER_INVALID_VALUE);
			}
			TM_SETBIT(rtc_status, data->status_bit);
			return hal_rtcGetStatus();
		case DRV_CTRL_CLEARBIT:
			if( data == 0 ) { return rtcSetError(ERR_NULL_POINTER); }
			if( (data->status_bit < DRV_BIT_INIT) || (data->status_bit > DRV_BIT_DEAD) )
			{
				return rtcSetError(ERR_HAL_DRIVER_INVALID_VALUE);
			}
			TM_CLEARBIT(rtc_status, data->status_bit);
			return hal_rtcGetStatus();
		case DRV_CTRL_GETBIT:
			if( data == 0 ) { return rtcSetError(ERR_NULL_POINTER); }
			if( (data->status_bit < DRV_BIT_INIT) || (data->status_bit > DRV_BIT_DEAD) )
			{
				return rtcSetError(ERR_HAL_DRIVER_INVALID_VALUE);
			}
			data->bit_value = TM_GETBIT(rtc_status, data->status_bit) != 0;
			return hal_rtcGetStatus();
		case DRV_CTRL_GETSTATUS:
			return hal_rtcGetStatus();
		case DRV_CTRL_GETLASTERROR:
			if( data == 0 ) { return rtcSetError(ERR_NULL_POINTER); }
			data->error = rtc_last_error;
			return hal_rtcGetStatus();
		default:

			return rtcSetError(ERR_HAL_DRIVER_INVALID_CONTROL);
	}
}

// NOLINTEND
