/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file lcd_AMC2004.c
 * @brief lcd amc2004 implementation.
 *
 */

/* =============================================================================
 * Declarations - Include
 * ===========================================================================*/

#include <util/delay.h>

#include "hal/public/tmlibc.h"
#include "interfaces/drv_i2c.h"
#include "interfaces/drv_lcd.h"
#include "interfaces/tm_macros.h"
#include "interfaces/tm_modules.h"
#include "interfaces/tm_runLevel.h"

// NOLINTBEGIN
// NOLINT(readability-magic-numbers)

/* -----------------------------------------------
 * Private function prototypes
 * ---------------------------------------------*/

static hal_driver_state_t lcdAMC2004Clear(void);
static hal_driver_state_t lcdAMC2004SendCommand(uint8_t command);
static hal_driver_state_t lcdSetError(err_codes_t error);

/* -----------------------------------------------
 * Private variables
 * ---------------------------------------------*/

static hal_driver_status_t lcd_status;
static err_codes_t lcd_last_error = ERR_NO_ERROR;

/* -----------------------------------------------
 * Constants
 * ---------------------------------------------*/

#define LCDAMC2004_I2C_ADDR 0x3C // AiP31068L I2C address (Write mode)
#define LCDAMC2004_CMD 0x80 // Co=1 RS = 0, Write Command
#define LCDAMC2004_DATA 0x40 // Co=0 RS = 1, Write Data series
#define LCDAMC2004_RAW 4
#define LCDAMC2004_COL 20

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

/* -----------------------------------------------
 * Driver life cycle
 * ---------------------------------------------*/

static hal_driver_state_t lcdSetError(err_codes_t error)
{
	lcd_last_error = error;
	return DRV_STATE_ERROR;
}

static hal_driver_state_t hal_lcdGetStatus(void)
{
	if( TM_GETBIT(lcd_status, DRV_BIT_DEAD) != 0 )
	{
		lcd_last_error = ERR_HAL_DRIVER_DEAD;
		return DRV_STATE_DEAD;
	}
	if( TM_GETBIT(lcd_status, DRV_BIT_ERROR) != 0 )
	{
		return lcdSetError(ERR_HAL_DRIVER_INVALID_STATE);
	}
	if( hal_i2cControl(DRV_CTRL_GETSTATUS, 0) != DRV_STATE_RUNNING )
	{
		return lcdSetError(ERR_HAL_DRIVER_DEPENDENCY);
	}
	if( TM_GETBIT(lcd_status, DRV_BIT_INIT) == 0 )
	{
		if( TM_GETBIT(lcd_status, DRV_BIT_START) == 0 ) { return DRV_STATE_OFF; }
		return lcdSetError(ERR_HAL_DRIVER_INVALID_STATE);
	}
	if( TM_GETBIT(lcd_status, DRV_BIT_START) == 0 ) { return DRV_STATE_INITIALIZED; }
	return DRV_STATE_RUNNING;
}

static hal_driver_state_t lcdRequireRunning(void)
{
	hal_driver_state_t state = hal_lcdGetStatus();
	if( (state == DRV_STATE_OFF) || (state == DRV_STATE_INITIALIZED) )
	{
		return lcdSetError(ERR_HAL_DRIVER_NOT_RUNNING);
	}
	return state;
}

static hal_driver_state_t hal_lcdInit(void)
{
	if( TM_GETBIT(lcd_status, DRV_BIT_DEAD) != 0 ) { return lcdSetError(ERR_HAL_DRIVER_DEAD); }
	if( hal_i2cControl(DRV_CTRL_GETSTATUS, 0) != DRV_STATE_RUNNING )
	{
		return lcdSetError(ERR_HAL_DRIVER_DEPENDENCY);
	}

	_delay_ms(50); // Wait for LCD to power up

	if( lcdAMC2004SendCommand(0x38) == DRV_STATE_ERROR ) { return DRV_STATE_ERROR; }
	// Function Set: 8-bit mode, 2 lines, 5x8 dots
	_delay_us(110);
	if( lcdAMC2004SendCommand(0x0C) == DRV_STATE_ERROR ) { return DRV_STATE_ERROR; }
	// Display ON, Cursor OFF, Blink OFF
	_delay_us(110);
	if( lcdAMC2004SendCommand(0x01) == DRV_STATE_ERROR ) { return DRV_STATE_ERROR; }
	// Clear Display
	_delay_ms(11);
	if( lcdAMC2004SendCommand(0x06) == DRV_STATE_ERROR ) { return DRV_STATE_ERROR; }
	// Entry Mode: Cursor moves right, no shift
	_delay_us(110);

	TM_SETBIT(lcd_status, DRV_BIT_INIT);
	lcd_last_error = ERR_NO_ERROR;
	return DRV_STATE_INITIALIZED;
}

static hal_driver_state_t hal_lcdStart(void)
{
	if( TM_GETBIT(lcd_status, DRV_BIT_DEAD) != 0 ) { return lcdSetError(ERR_HAL_DRIVER_DEAD); }
	if( TM_GETBIT(lcd_status, DRV_BIT_INIT) == 0 )
	{

		return lcdSetError(ERR_HAL_DRIVER_NOT_INITIALIZED);
	}
	if( lcdAMC2004Clear() == DRV_STATE_ERROR ) { return DRV_STATE_ERROR; }

	TM_SETBIT(lcd_status, DRV_BIT_START);
	return DRV_STATE_RUNNING;
}

static hal_driver_state_t hal_lcdStop(void)
{
	// nothing to do.
	TM_CLEARBIT(lcd_status, DRV_BIT_START);
	return hal_lcdGetStatus();
}

/* -----------------------------------------------
 * LCD operations
 * ---------------------------------------------*/

static hal_driver_state_t lcdAMC2004SendCommand(uint8_t command)
{
	if( hal_i2cCommStart(LCDAMC2004_I2C_ADDR, HAL_I2C_WRITE) == DRV_STATE_ERROR )
	{
		return lcdSetError(ERR_HAL_DRIVER_DEPENDENCY);
	}
	if( hal_i2cWrite(LCDAMC2004_CMD) == DRV_STATE_ERROR )
	{
		return lcdSetError(ERR_HAL_DRIVER_DEPENDENCY);
	}
	if( hal_i2cWrite(command) == DRV_STATE_ERROR )
	{
		return lcdSetError(ERR_HAL_DRIVER_DEPENDENCY);
	}
	if( hal_i2cCommStop() == DRV_STATE_ERROR ) { return lcdSetError(ERR_HAL_DRIVER_DEPENDENCY); }
	_delay_us(200); // Small delay for LCD to process the command
	return DRV_STATE_RUNNING;
}

static hal_driver_state_t lcdAMC2004Clear(void)
{
	if( lcdAMC2004SendCommand(0x01) == DRV_STATE_ERROR ) { return DRV_STATE_ERROR; }
	_delay_ms(2);
	return DRV_STATE_RUNNING;
}

hal_driver_state_t hal_lcdClear(void)
{
	hal_driver_state_t state = lcdRequireRunning();
	if( state != DRV_STATE_RUNNING ) { return state; }
	return lcdAMC2004Clear();
}

hal_driver_state_t hal_lcdSetCursor(uint8_t row, uint8_t col)
{
	hal_driver_state_t state = lcdRequireRunning();
	if( state != DRV_STATE_RUNNING ) { return state; }
	if( (row >= LCDAMC2004_RAW) || (col >= LCDAMC2004_COL) )
	{
		return lcdSetError(ERR_HAL_LCD_CURSOR_OUT_OF_RANGE);
	}
	const uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
	return lcdAMC2004SendCommand((uint8_t)(0x80u | (col + row_offsets[row])));
}

hal_driver_state_t hal_lcdWriteString(tm_string_t str)
{
	uint8_t index = 0;

	hal_driver_state_t state = lcdRequireRunning();
	if( state != DRV_STATE_RUNNING ) { return state; }
	if( str.text == 0 ) { return lcdSetError(ERR_NULL_POINTER); }

	if( hal_i2cCommStart(LCDAMC2004_I2C_ADDR, HAL_I2C_WRITE) == DRV_STATE_ERROR )
	{
		return lcdSetError(ERR_HAL_DRIVER_DEPENDENCY);
	}
	if( hal_i2cWrite(LCDAMC2004_DATA) == DRV_STATE_ERROR )
	{
		return lcdSetError(ERR_HAL_DRIVER_DEPENDENCY);
	}

	while( index < TM_STRING_SIZE_MAX )
	{
		char str_char = hal_string_getChar(&str, index);
		if( str_char == 0 ) { break; }
		if( hal_i2cWrite((uint8_t)str_char) == DRV_STATE_ERROR )
		{
			return lcdSetError(ERR_HAL_DRIVER_DEPENDENCY);
		}
		index++;
	}
	if( hal_i2cCommStop() == DRV_STATE_ERROR ) { return lcdSetError(ERR_HAL_DRIVER_DEPENDENCY); }
	return DRV_STATE_RUNNING;
}

/* -----------------------------------------------
 * Driver control
 * ---------------------------------------------*/

hal_driver_state_t hal_lcdControl(hal_driver_control_t command, hal_driver_control_data_t *data)
{
	switch( command )
	{
		case DRV_CTRL_INIT:
			return hal_lcdInit();
		case DRV_CTRL_START:
			return hal_lcdStart();
		case DRV_CTRL_STOP:
			return hal_lcdStop();
		case DRV_CTRL_RLSET:
			if( data == 0 ) { return lcdSetError(ERR_NULL_POINTER); }
			if( data->run_level >= RL_LEVEL_COUNT )
			{
				return lcdSetError(ERR_HAL_DRIVER_INVALID_VALUE);
			}
			lcd_status &= (hal_driver_status_t)~RL_LEVEL_MASK;
			lcd_status |= data->run_level;
			return hal_lcdGetStatus();
		case DRV_CTRL_RLGET:
			if( data == 0 ) { return lcdSetError(ERR_NULL_POINTER); }
			data->run_level = lcd_status & RL_LEVEL_MASK;
			return hal_lcdGetStatus();
		case DRV_CTRL_SETBIT:
			if( data == 0 ) { return lcdSetError(ERR_NULL_POINTER); }
			if( (data->status_bit < DRV_BIT_INIT) || (data->status_bit > DRV_BIT_DEAD) )
			{
				return lcdSetError(ERR_HAL_DRIVER_INVALID_VALUE);
			}
			TM_SETBIT(lcd_status, data->status_bit);
			return hal_lcdGetStatus();
		case DRV_CTRL_CLEARBIT:
			if( data == 0 ) { return lcdSetError(ERR_NULL_POINTER); }
			if( (data->status_bit < DRV_BIT_INIT) || (data->status_bit > DRV_BIT_DEAD) )
			{
				return lcdSetError(ERR_HAL_DRIVER_INVALID_VALUE);
			}
			TM_CLEARBIT(lcd_status, data->status_bit);
			return hal_lcdGetStatus();
		case DRV_CTRL_GETBIT:
			if( data == 0 ) { return lcdSetError(ERR_NULL_POINTER); }
			if( (data->status_bit < DRV_BIT_INIT) || (data->status_bit > DRV_BIT_DEAD) )
			{
				return lcdSetError(ERR_HAL_DRIVER_INVALID_VALUE);
			}
			data->bit_value = TM_GETBIT(lcd_status, data->status_bit) != 0;
			return hal_lcdGetStatus();
		case DRV_CTRL_GETSTATUS:
			return hal_lcdGetStatus();
		case DRV_CTRL_GETLASTERROR:
			if( data == 0 ) { return lcdSetError(ERR_NULL_POINTER); }
			data->error = lcd_last_error;
			return hal_lcdGetStatus();
		default:

			return lcdSetError(ERR_HAL_DRIVER_INVALID_CONTROL);
	}
}

// NOLINTEND
