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

#include "system.h"

#include "hal/public/lcd.h"
#include "hal/public/rtc.h"
#include "interfaces/tm_info.h"
#include "system/sysCall/sysCall.h"
#include "tm_libc/tm_stdio.h"
#include "tm_libc/tm_string.h"
#include "tm_libc/tm_syslog.h"

void system(void)
{
	// External RTC module test

	hal_rtc_time_t t;
	char msg[30];

	hal_rtcRead(&t);
	tm_syslog(TM_STR("[system] date & time : %02i/%02i/20%02i %02i:%02i\n"),
			  t.day,
			  t.month,
			  t.year,
			  t.hours,
			  t.minutes);

	tm_snprintf(
		msg, sizeof(msg), TM_STR("TaskMate %i.%i %i"), TM_VER_MAJOR, TM_VER_MINOR, TM_BUILD);
	hal_lcdClear();
	hal_lcdSetCursor(0, 0);
	hal_lcdWriteString(TM_STR_RAM(msg));

	while( 1 )
	{

		// print date and time
		hal_rtcRead(&t);
		tm_snprintf(msg,
					sizeof(msg),
					TM_STR("%02i/%02i/20%02i %02i:%02i:%02i"),
					t.day,
					t.month,
					t.year,
					t.hours,
					t.minutes,
					t.seconds);
		hal_lcdSetCursor(1, 0);
		hal_lcdWriteString(TM_STR_RAM(msg));

		sc_threadSetSTC(50);
		while( sc_threadGetSTC() > 0 ) { sc_coopYield(); };
	}
}
