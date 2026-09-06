/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file boot.c
 * @brief boot implementation.
 *
 */

/* =============================================================================
 * Declarations - Include
 * ===========================================================================*/

#include "boot.h"

#include "interfaces/drv_usart.h"
#include "interfaces/tm_modules.h"
#include "interfaces/tm_runLevel.h"
#include "system/sysCall/sc_hal.h"
#include "system/sysCore/gpio.h"
#include "system/sysCore/hal_init.h"
#include "system/sysCore/modules.h"
#include "system/sysCore/modules_list.h"
#include "tm_libc/tm_syslog.h"

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

void boot(void)
{
	// System startup
	hal_usartControl(DRV_CTRL_INIT, 0);
	hal_usartControl(DRV_CTRL_START, 0);

	tm_syslog(TM_STR("\n\n[boot] System startup ...\n"));

	// Initialise static system allocations
	tm_syslog(TM_STR("[boot] system static allocation\n"));

	mod_driversAlloc();
	mod_threadsAlloc();

	// Initialise HAL hardware
	tm_syslog(TM_STR("[boot] hal hardware init\n"));

	hal_archInit();
	hal_mcuInit();
	hal_boardInit();
	gpio_signalsInit();

	// Start drivers
	for( uint8_t runlevel = 1; runlevel < RL_LEVEL_COUNT; runlevel++ )
	{
		for( uint8_t i = 0; i < TM_MOD_DRIVER_COUNT; i++ )
		{
			mod_driver_item_t *mod = mod_driverGetPointer(i);
			hal_driver_control_data_t control_data;

			if( ((*(mod->control))(DRV_CTRL_RLGET, &control_data) != DRV_STATE_ERROR) &&
				(control_data.run_level == runlevel) )
			{
				tm_syslog(TM_STR("[boot] driver <%s> ... "), mod->name);
				(*(mod->control))(DRV_CTRL_INIT, 0);
				tm_syslog(TM_STR("init ... "), mod->name);
				(*(mod->control))(DRV_CTRL_START, 0);
				tm_syslog(TM_STR("start ... ok\n"), mod->name);
				if( mod->control == hal_i2cControl ) { sc_i2cScan(); }
				if( mod->control == hal_rtcControl ) { sc_rtcSaveStartupTime(); }
			}
		}
	}
}
