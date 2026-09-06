/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file system.c
 * @brief system management implementation.
 */

/* =============================================================================
 * Declarations - Include
 * ===========================================================================*/

#include "system.h"

#include <stdbool.h>
#include <stdint.h>

#include "interfaces/tm_info.h"
#include "interfaces/tm_runLevel.h"
#include "system/sysCall/error.h"
#include "system/sysCall/sc_hal.h"
#include "system/sysCall/sc_modules.h"
#include "tm_libc/tm_stdio.h"
#include "tm_libc/tm_string.h"
#include "tm_libc/tm_syslog.h"

/* -----------------------------------------------
 * Constants
 * ---------------------------------------------*/

#define SYSTEM_RUN_LEVEL_RR_ROUND_COUNT 10u

/* -----------------------------------------------
 * Private function prototypes
 * ---------------------------------------------*/

static void systemStart(void);
static bool systemRunLevelIsReady(uint8_t run_level);

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

void system(void)
{
	sc_threadSetInitialized();
	systemStart();

	// External RTC module test

	hal_rtc_time_t t;
	char msg[30];

	sc_rtcRead(&t);
	tm_syslog(TM_STR("[system] date & time : %02i/%02i/20%02i %02i:%02i\n"),
			  t.day,
			  t.month,
			  t.year,
			  t.hours,
			  t.minutes);

	tm_snprintf(
		msg, sizeof(msg), TM_STR("TaskMate %i.%i %i"), TM_VER_MAJOR, TM_VER_MINOR, TM_BUILD);
	sc_lcdClear();
	sc_lcdWriteString(TM_STR_RAM(msg), 0, 0);
	
	while( 1 )
	{

		// print date and time
		sc_rtcRead(&t);
		tm_snprintf(msg,
					sizeof(msg),
					TM_STR("%02i/%02i/20%02i %02i:%02i:%02i"),
					t.day,
					t.month,
					t.year,
					t.hours,
					t.minutes,
					t.seconds);
		sc_lcdWriteString(TM_STR_RAM(msg), 1, 0);

		sc_threadSetSTC(50);
		while( sc_threadGetSTC() > 0 ) { sc_coopYield(); };
	}
}

/* -----------------------------------------------
 * System startup
 * ---------------------------------------------*/

static void systemStart(void)
{
	for( uint8_t run_level = RL_RUN_CORE; run_level < RL_LEVEL_COUNT; run_level++ )
	{
		tm_syslog(TM_STR("[system] start run level %i\n"), run_level);
		sc_driverRunLevelStart(run_level);
		sc_threadRunLevelStart(run_level);

		if( run_level == RL_RUN_CORE ) { (void)sc_i2cScan(); }
		if( run_level == RL_RUN_DRIVER ) { (void)sc_rtcSaveStartupTime(); }

		for( uint8_t round = 0; round < SYSTEM_RUN_LEVEL_RR_ROUND_COUNT; round++ )
		{
			sc_coopYield();
		}

		if( !systemRunLevelIsReady(run_level) )
		{
			sc_panic(TM_STR("run level initialization failed"));
		}
	}
}

static bool systemRunLevelIsReady(uint8_t run_level)
{
	for( uint8_t level = RL_RUN_NONE; level <= run_level; level++ )
	{
		if( !sc_driverRunLevelIsReady(level) ) { return false; }
	}

	for( uint8_t level = RL_RUN_CORE; level <= run_level; level++ )
	{
		if( !sc_threadRunLevelIsReady(level) ) { return false; }
	}

	return true;
}
