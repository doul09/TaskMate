/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file sysCall.c
 * @brief sys call implementation.
 *
 */

#include "sysCall.h"

#include "hal/public/atomic.h"
#include "interfaces/drv_i2c.h"
#include "interfaces/drv_lcd.h"
#include "interfaces/drv_rtc.h"
#include "interfaces/drv_timerSched.h"
#include "interfaces/drv_usart.h"
#include "interfaces/tm_macros.h"
#include "interfaces/tm_modules.h"
#include "interfaces/tm_runLevel.h"
#include "system/sysCore/modules.h"
#include "system/sysCore/tm_scheduler.h"
#include "tm_libc/tm_string.h"
#include "tm_libc/tm_syslog.h"

#define I2C_SCAN_ADDRESS_COUNT_MAX 10u

static uint8_t i2c_scan_addresses[I2C_SCAN_ADDRESS_COUNT_MAX];
static uint8_t i2c_scan_address_count;

static mod_thread_item_t *sc_threadGetPointer(const char *name);
static mod_driver_item_t *sc_driverGetPointer(const char *name);
static bool sc_driverControl(const char *name, hal_driver_control_t command);
static err_codes_t sc_driverOperationError(
	hal_driver_state_t state,
	hal_driver_state_t (*control)(hal_driver_control_t, hal_driver_control_data_t *));
static bool sc_i2cAddressFound(uint8_t address);
static void sc_i2cDriverSetOff(mod_driver_item_t *driver);

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

uint16_t sc_driverGetCount(void) { return TM_MOD_DRIVER_COUNT; }

bool sc_driverGetInfo(uint16_t id, const tm_string_t **name, uint8_t *run_level,
					  uint8_t *status_bits)
{
	if( (id >= TM_MOD_DRIVER_COUNT) || (name == 0) || (run_level == 0) || (status_bits == 0) )
	{
		return false;
	}

	mod_driver_item_t *driver = mod_driverGetPointer((uint8_t)id);
	if( (driver->name == 0) || (driver->control == 0) ) { return false; }

	hal_driver_control_data_t control_data;
	if( driver->control(DRV_CTRL_RLGET, &control_data) == DRV_STATE_ERROR ) { return false; }
	*run_level = control_data.run_level;
	*status_bits = 0;

	static const hal_driver_status_bit_t status_bit[] = {
		DRV_BIT_INIT,
		DRV_BIT_START,
		DRV_BIT_ERROR,
		DRV_BIT_DEAD,
	};
	for( uint8_t i = 0; i < (sizeof(status_bit) / sizeof(status_bit[0])); i++ )
	{
		control_data.status_bit = status_bit[i];
		if( driver->control(DRV_CTRL_GETBIT, &control_data) == DRV_STATE_ERROR ) { return false; }
		if( control_data.bit_value ) { TM_SETBIT(*status_bits, status_bit[i]); }
	}

	*name = driver->name;
	return *name != 0;
}

bool sc_driverInit(const char *name) { return sc_driverControl(name, DRV_CTRL_INIT); }

bool sc_driverStart(const char *name) { return sc_driverControl(name, DRV_CTRL_START); }

bool sc_driverStop(const char *name) { return sc_driverControl(name, DRV_CTRL_STOP); }

err_codes_t sc_lcdClear(void)
{
	return sc_driverOperationError(hal_lcdClear(), hal_lcdControl);
}

err_codes_t sc_lcdSetCursor(uint8_t row, uint8_t col)
{
	return sc_driverOperationError(hal_lcdSetCursor(row, col), hal_lcdControl);
}

err_codes_t sc_lcdWriteString(tm_string_t str)
{
	return sc_driverOperationError(hal_lcdWriteString(str), hal_lcdControl);
}

err_codes_t sc_rtcRead(hal_rtc_time_t *time)
{
	if( time == 0 ) { return ERR_NULL_POINTER; }
	return sc_driverOperationError(hal_rtcRead(time), hal_rtcControl);
}

err_codes_t sc_i2cScan(void)
{
	uint8_t address;
	hal_driver_control_data_t control_data;
	bool address_buffer_full = false;

	i2c_scan_address_count = 0;
	while( hal_i2cScan(&address) == DRV_STATE_RUNNING )
	{
		if( i2c_scan_address_count < I2C_SCAN_ADDRESS_COUNT_MAX )
		{
			i2c_scan_addresses[i2c_scan_address_count] = address;
			i2c_scan_address_count++;
		}
		else { address_buffer_full = true; }

		tm_syslog(TM_STR("[sysCall:i2cscan] found : 0x%02x\n"), address);
	}

	hal_i2cControl(DRV_CTRL_GETLASTERROR, &control_data);
	if( control_data.error != ERR_HAL_I2C_SCAN_COMPLETE ) { return control_data.error; }
	if( address_buffer_full ) { return ERR_I2C_SCAN_ADDRESS_BUFFER_FULL; }

	control_data.status_bit = DRV_BIT_DEAD;
	for( uint8_t i = 0; i < TM_MOD_DRIVER_COUNT; i++ )
	{
		mod_driver_item_t *driver = mod_driverGetPointer(i);
		if( (driver->address != TM_MOD_DRIVER_ADDRESS_NONE) &&
			!sc_i2cAddressFound(driver->address) )
		{
			driver->control(DRV_CTRL_SETBIT, &control_data);
		}
	}

	for( uint8_t i = 0; i < TM_MOD_DRIVER_COUNT; i++ )
	{
		mod_driver_item_t *driver = mod_driverGetPointer(i);
		if( (driver->control(DRV_CTRL_GETSTATUS, 0) == DRV_STATE_DEAD) &&
			sc_i2cAddressFound(driver->address) )
		{
			sc_i2cDriverSetOff(driver);
		}
	}

	return ERR_NO_ERROR;
}

err_codes_t sc_usartRead(uint8_t *data)
{
	if( data == 0 ) { return ERR_NULL_POINTER; }

	/* Keep the failed operation and its error snapshot indivisible from the RX ISR. */
	hal_atomic_state_t state = hal_atomicStart();
	if( hal_usartRead(data) == DRV_STATE_RUNNING )
	{
		hal_atomicEnd(state);
		return ERR_NO_ERROR;
	}
	hal_driver_control_data_t control_data;
	hal_usartControl(DRV_CTRL_GETLASTERROR, &control_data);
	hal_atomicEnd(state);
	return control_data.error;
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

static err_codes_t sc_driverOperationError(
	hal_driver_state_t state,
	hal_driver_state_t (*control)(hal_driver_control_t, hal_driver_control_data_t *))
{
	if( state == DRV_STATE_RUNNING ) { return ERR_NO_ERROR; }

	hal_driver_control_data_t control_data;
	control(DRV_CTRL_GETLASTERROR, &control_data);
	return control_data.error;
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

static mod_driver_item_t *sc_driverGetPointer(const char *name)
{
	if( name == 0 ) { return 0; }

	for( uint8_t i = 0; i < TM_MOD_DRIVER_COUNT; i++ )
	{
		mod_driver_item_t *driver = mod_driverGetPointer(i);
		if( (driver->name != 0) && (driver->control != 0) &&
			tm_strncmp(*driver->name, TM_STR_RAM(name), TM_MOD_NAME_SIZE_MAX) == 0 )
		{
			return driver;
		}
	}

	return 0;
}

static bool sc_driverControl(const char *name, hal_driver_control_t command)
{
	mod_driver_item_t *driver = sc_driverGetPointer(name);
	if( driver == 0 ) { return false; }

	hal_driver_state_t state = driver->control(command, 0);
	return (state != DRV_STATE_ERROR) && (state != DRV_STATE_DEAD);
}

static bool sc_i2cAddressFound(uint8_t address)
{
	for( uint8_t i = 0; i < i2c_scan_address_count; i++ )
	{
		if( i2c_scan_addresses[i] == address ) { return true; }
	}
	return false;
}

static void sc_i2cDriverSetOff(mod_driver_item_t *driver)
{
	hal_driver_control_data_t control_data;

	control_data.status_bit = DRV_BIT_START;
	driver->control(DRV_CTRL_CLEARBIT, &control_data);
	control_data.status_bit = DRV_BIT_INIT;
	driver->control(DRV_CTRL_CLEARBIT, &control_data);
	control_data.status_bit = DRV_BIT_ERROR;
	driver->control(DRV_CTRL_CLEARBIT, &control_data);
	control_data.status_bit = DRV_BIT_DEAD;
	driver->control(DRV_CTRL_CLEARBIT, &control_data);
}
