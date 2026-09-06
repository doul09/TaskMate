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

#include "interfaces/tm_modules.h"
#include "interfaces/tm_runLevel.h"
#include "system/sysCall/sc_hal.h"
#include "system/sysCore/modules.h"
#include "system/sysCore/modules_list.h"
#include "tm_libc/tm_syslog.h"

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

void boot(void)
{

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
