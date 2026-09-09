/*
 * TaskMate Project
 * (c) 2025 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file printModules.c
 * @brief print modules implementation.
 *
 */

/* =============================================================================
 * Declarations - Include
 * ===========================================================================*/

#include "printModules.h"

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

void printModules(const modules_database_t *data_base)
{
	AUTOCODE_MSG_INFO("+++ modules informations +++\n");

	AUTOCODE_MSG_INFO("found drivers :");
	const module_type_t *driver = &data_base->modules_type[MOD_DRIVER_ID];
	for( int i = 0; i < driver->modules_count; i++ )
	{
		printf("\tdrivers[%i] \"%s\" runlevel=%i",
			   i,
			   driver->modules[i].name,
			   driver->modules[i].status);
		if( driver->modules[i].address != MOD_DRIVER_ADDRESS_NONE )
		{
			printf(" i2c=0x%02X", driver->modules[i].address);
		}
		printf("\n");
	}
	printf("\n");

	AUTOCODE_MSG_INFO("found threads :");
	const module_type_t *threads = &data_base->modules_type[MOD_THREAD_ID];
	for( int i = 0; i < threads->modules_count; i++ )
	{
		printf("\tthread[%i] \"%s\" runlevel=%i type=%i\n",
			   i,
			   threads->modules[i].name,
			   RL_GET_RUN_LEVEL(threads->modules[i].status),
			   threads->modules[i].status & (~RL_LEVEL_MASK));
	}
	printf("\n");
}
