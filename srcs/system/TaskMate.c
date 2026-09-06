/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file TaskMate.c
 * @brief TaskMate implementation.
 *
 */

/* =============================================================================
 * Declarations - Include
 * ===========================================================================*/

#include <stdint.h>

#include "hal/public/panic.h"
#include "interfaces/tm_info.h"
#include "interfaces/tm_macros.h"
#include "system/sysCore/boot.h"
#include "system/sysCore/tm_scheduler.h"
#include "system/sysCore/tm_softwareTimeCounter.h"
#include "tm_libc/tm_syslog.h"

/* -----------------------------------------------
 * File metadata
 * ---------------------------------------------*/

TM_STORE_FILE_NAME(file_name);

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

int main(void)
{
	// System startup
	boot();

	tm_syslog(
		TM_STR("[info] %s v%i.%i build : %i\n"), &file_name, TM_VER_MAJOR, TM_VER_MINOR, TM_BUILD);

	// Start scheduler
	tm_syslog(TM_STR("[info] start round-robin scheduler\n"));

	tm_softwareTimeCounterInit();

	tm_schedulerInit();
	tm_schedulerStart();

	panic(TM_STR("\nsystem launch fail"));

	return 0; // You should never get here
}
