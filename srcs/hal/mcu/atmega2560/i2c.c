/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file i2c.c
 * @brief i2c implementation.
 *
 */

/* =============================================================================
 * Declarations - Include
 * ===========================================================================*/

#include <avr/io.h>
#include <util/twi.h>

#include "interfaces/drv_i2c.h"
#include "interfaces/tm_macros.h"
#include "interfaces/tm_modules.h"
#include "interfaces/tm_runLevel.h"
#include "mcu_define.h" // Get the I2C frequency

// NOLINTBEGIN
// NOLINT(readability-magic-numbers)

/* -----------------------------------------------
 * Constants
 * ---------------------------------------------*/

#define I2C_TWBR_VALUE ((F_CPU / I2C_FREQ - 16) / 2)

/* -----------------------------------------------
 * Private variables
 * ---------------------------------------------*/

static hal_driver_status_t i2c_status;
static err_codes_t i2c_last_error = ERR_NO_ERROR;
static uint8_t i2c_scan_address = 0;

/* -----------------------------------------------
 * Private function prototypes
 * ---------------------------------------------*/

static void i2cCommStop(void);
static uint8_t i2cWrite(uint8_t data);
static hal_driver_state_t i2cSetError(err_codes_t error);

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

/* -----------------------------------------------
 * Driver state and startup
 * ---------------------------------------------*/

static hal_driver_state_t i2cSetError(err_codes_t error)
{
	i2c_last_error = error;
	return DRV_STATE_ERROR;
}

static hal_driver_state_t hal_i2cGetStatus(void)
{
	if( TM_GETBIT(i2c_status, DRV_BIT_DEAD) != 0 )
	{
		i2c_last_error = ERR_HAL_DRIVER_DEAD;
		return DRV_STATE_DEAD;
	}
	if( TM_GETBIT(i2c_status, DRV_BIT_ERROR) != 0 )
	{
		return i2cSetError(ERR_HAL_DRIVER_INVALID_STATE);
	}
	if( TM_GETBIT(i2c_status, DRV_BIT_INIT) == 0 )
	{
		if( TM_GETBIT(i2c_status, DRV_BIT_START) == 0 ) { return DRV_STATE_OFF; }
		return i2cSetError(ERR_HAL_DRIVER_INVALID_STATE);
	}
	if( TM_GETBIT(i2c_status, DRV_BIT_START) == 0 ) { return DRV_STATE_INITIALIZED; }
	return DRV_STATE_RUNNING;
}

static hal_driver_state_t i2cRequireRunning(void)
{
	hal_driver_state_t state = hal_i2cGetStatus();
	if( (state == DRV_STATE_OFF) || (state == DRV_STATE_INITIALIZED) )
	{
		return i2cSetError(ERR_HAL_DRIVER_NOT_RUNNING);
	}
	return state;
}

static hal_driver_state_t hal_i2cInit(void)
{
	if( hal_i2cGetStatus() == DRV_STATE_DEAD ) { return i2cSetError(ERR_HAL_DRIVER_DEAD); }

	TWBR = (uint8_t)I2C_TWBR_VALUE; // Set baud rate
	TWSR = 0x00; // Pre scaler = 1

	TM_SETBIT(i2c_status, DRV_BIT_INIT);
	i2c_last_error = ERR_NO_ERROR;
	return DRV_STATE_INITIALIZED;
}

static hal_driver_state_t hal_i2cStart(void)
{
	if( TM_GETBIT(i2c_status, DRV_BIT_DEAD) != 0 ) { return i2cSetError(ERR_HAL_DRIVER_DEAD); }
	if( TM_GETBIT(i2c_status, DRV_BIT_INIT) == 0 )
	{

		return i2cSetError(ERR_HAL_DRIVER_NOT_INITIALIZED);
	}

	TWCR = (uint8_t)(1u << TWEN); // Enable TWI

	TM_SETBIT(i2c_status, DRV_BIT_START);
	i2c_last_error = ERR_NO_ERROR;
	return DRV_STATE_RUNNING;
}

/* -----------------------------------------------
 * Bus discovery
 * ---------------------------------------------*/

hal_driver_state_t hal_i2cScan(uint8_t *address)
{
	hal_driver_state_t state = i2cRequireRunning();
	if( state != DRV_STATE_RUNNING ) { return state; }
	if( address == 0 ) { return i2cSetError(ERR_NULL_POINTER); }

	while( i2c_scan_address <= MOD_I2C_ADDRESS_MAX )
	{
		TM_WRITEBIT(TWCR, TWSTA, TWEN, TWINT);
		while( !(TM_GETBIT(TWCR, TWINT)) );
		if( (TW_STATUS != TW_START) && (TW_STATUS != TW_REP_START) )
		{
			i2cCommStop();
			return i2cSetError(ERR_HAL_I2C_START_FAILED);
		}

		const uint8_t status = i2cWrite((uint8_t)(i2c_scan_address << 1));
		i2cCommStop();

		if( status == TW_MT_SLA_ACK )
		{
			*address = i2c_scan_address++;
			i2c_last_error = ERR_NO_ERROR;
			return DRV_STATE_RUNNING;
		}
		i2c_scan_address++;
	}

	i2c_scan_address = 0;
	return i2cSetError(ERR_HAL_I2C_SCAN_COMPLETE);
}

/* -----------------------------------------------
 * Driver shutdown and bus transactions
 * ---------------------------------------------*/

static hal_driver_state_t hal_i2cStop(void)
{
	TM_CLEARBIT(TWCR, TWEN);

	TM_CLEARBIT(i2c_status, DRV_BIT_START);
	return hal_i2cGetStatus();
}

hal_driver_state_t hal_i2cCommStart(uint8_t address, hal_i2c_direction_t direction)
{
	hal_driver_state_t state = i2cRequireRunning();
	if( state != DRV_STATE_RUNNING ) { return state; }
	if( (address > 0x7Fu) || ((direction != HAL_I2C_WRITE) && (direction != HAL_I2C_READ)) )
	{
		return i2cSetError(ERR_HAL_DRIVER_INVALID_VALUE);
	}
	TM_WRITEBIT(TWCR, TWSTA, TWEN, TWINT);
	while( !(TM_GETBIT(TWCR, TWINT)) );
	if( (TW_STATUS != TW_START) && (TW_STATUS != TW_REP_START) )
	{
		i2cCommStop();
		return i2cSetError(ERR_HAL_I2C_START_FAILED);
	}

	const uint8_t twi_status = i2cWrite((uint8_t)((address << 1) | direction));
	const uint8_t expected_status = (direction == HAL_I2C_READ) ? TW_MR_SLA_ACK : TW_MT_SLA_ACK;
	if( twi_status != expected_status )
	{
		i2cCommStop();
		return i2cSetError(ERR_HAL_I2C_ADDRESS_NACK);
	}

	return DRV_STATE_RUNNING;
}

static void i2cCommStop(void) { TM_WRITEBIT(TWCR, TWSTO, TWEN, TWINT); }

hal_driver_state_t hal_i2cCommStop(void)
{
	hal_driver_state_t state = i2cRequireRunning();
	if( state != DRV_STATE_RUNNING ) { return state; }
	i2cCommStop();
	return DRV_STATE_RUNNING;
}

static uint8_t i2cWrite(uint8_t data)
{
	TWDR = data;
	TM_WRITEBIT(TWCR, TWEN, TWINT);
	while( !(TM_GETBIT(TWCR, TWINT)) );

	return TW_STATUS;
}

hal_driver_state_t hal_i2cWrite(uint8_t data)
{
	hal_driver_state_t state = i2cRequireRunning();
	if( state != DRV_STATE_RUNNING ) { return state; }
	if( i2cWrite(data) != TW_MT_DATA_ACK )
	{
		i2cCommStop();
		return i2cSetError(ERR_HAL_I2C_WRITE_FAILED);
	}
	return DRV_STATE_RUNNING;
}

hal_driver_state_t hal_i2cRead(uint8_t *data, hal_i2c_ack_t ack)
{
	hal_driver_state_t state = i2cRequireRunning();
	if( state != DRV_STATE_RUNNING ) { return state; }
	if( data == 0 ) { return i2cSetError(ERR_NULL_POINTER); }
	if( (ack != HAL_I2C_NACK) && (ack != HAL_I2C_ACK) )
	{
		return i2cSetError(ERR_HAL_DRIVER_INVALID_VALUE);
	}
	if( ack ) { TM_WRITEBIT(TWCR, TWEN, TWINT, TWEA); }
	else { TM_WRITEBIT(TWCR, TWEN, TWINT); }
	while( !(TM_GETBIT(TWCR, TWINT)) );

	const uint8_t expected_status = (ack == HAL_I2C_ACK) ? TW_MR_DATA_ACK : TW_MR_DATA_NACK;
	if( TW_STATUS != expected_status )
	{
		i2cCommStop();
		return i2cSetError(ERR_HAL_I2C_READ_FAILED);
	}

	*data = TWDR;
	return DRV_STATE_RUNNING;
}

/* -----------------------------------------------
 * Driver control
 * ---------------------------------------------*/

hal_driver_state_t hal_i2cControl(hal_driver_control_t command, hal_driver_control_data_t *data)
{
	switch( command )
	{
		case DRV_CTRL_INIT:
			return hal_i2cInit();
		case DRV_CTRL_START:
			return hal_i2cStart();
		case DRV_CTRL_STOP:
			return hal_i2cStop();
		case DRV_CTRL_RLSET:
			if( data == 0 ) { return i2cSetError(ERR_NULL_POINTER); }
			if( data->run_level >= RL_LEVEL_COUNT )
			{
				return i2cSetError(ERR_HAL_DRIVER_INVALID_VALUE);
			}
			i2c_status &= (hal_driver_status_t)~RL_LEVEL_MASK;
			i2c_status |= data->run_level;
			return hal_i2cGetStatus();
		case DRV_CTRL_RLGET:
			if( data == 0 ) { return i2cSetError(ERR_NULL_POINTER); }
			data->run_level = i2c_status & RL_LEVEL_MASK;
			return hal_i2cGetStatus();
		case DRV_CTRL_SETBIT:
			if( data == 0 ) { return i2cSetError(ERR_NULL_POINTER); }
			if( (data->status_bit < DRV_BIT_INIT) || (data->status_bit > DRV_BIT_DEAD) )
			{
				return i2cSetError(ERR_HAL_DRIVER_INVALID_VALUE);
			}
			TM_SETBIT(i2c_status, data->status_bit);
			return hal_i2cGetStatus();
		case DRV_CTRL_CLEARBIT:
			if( data == 0 ) { return i2cSetError(ERR_NULL_POINTER); }
			if( (data->status_bit < DRV_BIT_INIT) || (data->status_bit > DRV_BIT_DEAD) )
			{
				return i2cSetError(ERR_HAL_DRIVER_INVALID_VALUE);
			}
			TM_CLEARBIT(i2c_status, data->status_bit);
			return hal_i2cGetStatus();
		case DRV_CTRL_GETBIT:
			if( data == 0 ) { return i2cSetError(ERR_NULL_POINTER); }
			if( (data->status_bit < DRV_BIT_INIT) || (data->status_bit > DRV_BIT_DEAD) )
			{
				return i2cSetError(ERR_HAL_DRIVER_INVALID_VALUE);
			}
			data->bit_value = TM_GETBIT(i2c_status, data->status_bit) != 0;
			return hal_i2cGetStatus();
		case DRV_CTRL_GETSTATUS:
			return hal_i2cGetStatus();
		case DRV_CTRL_GETLASTERROR:
			if( data == 0 ) { return i2cSetError(ERR_NULL_POINTER); }
			data->error = i2c_last_error;
			return hal_i2cGetStatus();
		default:

			return i2cSetError(ERR_HAL_DRIVER_INVALID_CONTROL);
	}
}

// NOLINTEND
