/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file i2c.c
 * @brief I2C command implementation.
 */

#include "i2c.h"

#include "interfaces/tm_define.h"
#include "system/sysCall/error.h"
#include "system/sysCall/sc_hal.h"
#include "tm_libc/tm_string.h"
#include "tm_libc/tm_syslog.h"

typedef bool (*i2c_cmd_func_t)(uint8_t argc, char *argv[]);

typedef struct
{
	const char *name;
	i2c_cmd_func_t func;
} i2c_cmd_t;

static bool i2cScan(uint8_t argc, char *argv[]);
static bool i2cHelp(uint8_t argc, char *argv[]);

static const i2c_cmd_t i2c_cmd[] = {
	{"scan", i2cScan},
	{"help", i2cHelp},
	{0, 0},
};

bool i2c(uint8_t argc, char *argv[])
{
	if( argc < 2 )
	{
		i2cHelp(0, NULL);
		return false;
	}

	for( uint8_t i = 0; i2c_cmd[i].name != 0; i++ )
	{
		if( tm_strncmp(TM_STR_RAM(argv[1]), TM_STR_RAM(i2c_cmd[i].name), TM_STRING_SIZE_MAX) == 0 )
		{
			return i2c_cmd[i].func(argc, argv);
		}
	}

	i2cHelp(0, NULL);
	return false;
}

static bool i2cScan(uint8_t argc, char *argv[])
{
	(void)argv;

	if( argc != 2 )
	{
		i2cHelp(0, NULL);
		return false;
	}

	err_codes_t error = sc_i2cScan();
	if( error == ERR_NO_ERROR )
	{
		tm_syslog(TM_STR("[i2c] scan complete\n"));
		return true;
	}

	const tm_string_t *message = err_getMessage((uint8_t)error);
	if( message != 0 ) { tm_syslog(TM_STR("[i2c] scan error: %s\n"), message); }
	else { tm_syslog(TM_STR("[i2c] scan error\n")); }
	return false;
}

static bool i2cHelp(uint8_t argc, char *argv[])
{
	(void)argc;
	(void)argv;

	tm_syslog(TM_STR("[i2c] usage:\n"));
	tm_syslog(TM_STR("\ti2c scan\n"));
	tm_syslog(TM_STR("\ti2c help\n"));
	return true;
}
