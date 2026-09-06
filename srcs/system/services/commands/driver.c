/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file driver.c
 * @brief Driver command implementation.
 */

#include "driver.h"

#include "interfaces/tm_define.h"
#include "interfaces/tm_macros.h"
#include "interfaces/tm_modules.h"
#include "system/sysCall/sysCall.h"
#include "tm_libc/tm_string.h"
#include "tm_libc/tm_syslog.h"

typedef bool (*driver_cmd_func_t)(uint8_t argc, char *argv[]);

typedef struct
{
	const char *name;
	driver_cmd_func_t func;
} driver_cmd_t;

static bool driverInit(uint8_t argc, char *argv[]);
static bool driverStart(uint8_t argc, char *argv[]);
static bool driverStop(uint8_t argc, char *argv[]);
static bool driverList(uint8_t argc, char *argv[]);
static bool driverHelp(uint8_t argc, char *argv[]);

static const driver_cmd_t driver_cmd[] = {
	{"init", driverInit},
	{"start", driverStart},
	{"stop", driverStop},
	{"list", driverList},
	{"help", driverHelp},
	{0, 0},
};

bool driver(uint8_t argc, char *argv[])
{
	if( argc < 2 )
	{
		driverHelp(0, NULL);
		return false;
	}

	for( uint8_t i = 0; driver_cmd[i].name != 0; i++ )
	{
		if( tm_strncmp(TM_STR_RAM(argv[1]), TM_STR_RAM(driver_cmd[i].name), TM_STRING_SIZE_MAX) ==
			0 )
		{
			return driver_cmd[i].func(argc, argv);
		}
	}

	driverHelp(0, NULL);
	return false;
}

static bool driverHelp(uint8_t argc, char *argv[])
{
	(void)argc;
	(void)argv;

	tm_syslog(TM_STR("[driver] usage:\n"));
	tm_syslog(TM_STR("\tdriver init <driver_name>\n"));
	tm_syslog(TM_STR("\tdriver start <driver_name>\n"));
	tm_syslog(TM_STR("\tdriver stop <driver_name>\n"));
	tm_syslog(TM_STR("\tdriver list\n"));
	tm_syslog(TM_STR("\tdriver help\n"));
	return true;
}

static bool driverList(uint8_t argc, char *argv[])
{
	(void)argc;
	(void)argv;

	tm_syslog(TM_STR("[driver] drivers:\n"));
	const uint16_t driver_count = sc_driverGetCount();
	for( uint16_t id = 0; id < driver_count; id++ )
	{
		const tm_string_t *name;
		uint8_t run_level;
		uint8_t status_bits;
		if( !sc_driverGetInfo(id, &name, &run_level, &status_bits) ) { return false; }

		tm_syslog(TM_STR("\t%s runlevel=%i status[init=%i start=%i error=%i dead=%i]\n"),
				  name,
				  run_level,
				  TM_GETBIT(status_bits, DRV_BIT_INIT) != 0,
				  TM_GETBIT(status_bits, DRV_BIT_START) != 0,
				  TM_GETBIT(status_bits, DRV_BIT_ERROR) != 0,
				  TM_GETBIT(status_bits, DRV_BIT_DEAD) != 0);
	}

	return true;
}

static bool driverInit(uint8_t argc, char *argv[])
{
	if( argc != 3 )
	{
		driverHelp(0, NULL);
		return false;
	}

	tm_string_t driver_name = TM_STR_RAM(argv[2]);
	if( sc_driverInit(argv[2]) )
	{
		tm_syslog(TM_STR("[driver] %s initialized\n"), &driver_name);
		return true;
	}

	tm_syslog(TM_STR("[driver] %s not initialized\n"), &driver_name);
	return false;
}

static bool driverStart(uint8_t argc, char *argv[])
{
	if( argc != 3 )
	{
		driverHelp(0, NULL);
		return false;
	}

	tm_string_t driver_name = TM_STR_RAM(argv[2]);
	if( sc_driverStart(argv[2]) )
	{
		tm_syslog(TM_STR("[driver] %s started\n"), &driver_name);
		return true;
	}

	tm_syslog(TM_STR("[driver] %s not started\n"), &driver_name);
	return false;
}

static bool driverStop(uint8_t argc, char *argv[])
{
	if( argc != 3 )
	{
		driverHelp(0, NULL);
		return false;
	}

	tm_string_t driver_name = TM_STR_RAM(argv[2]);
	if( sc_driverStop(argv[2]) )
	{
		tm_syslog(TM_STR("[driver] %s stopped\n"), &driver_name);
		return true;
	}

	tm_syslog(TM_STR("[driver] %s not stopped\n"), &driver_name);
	return false;
}
