/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file scli.c
 * @brief scli implementation.
 *
 */

/* =============================================================================
 * Declarations - Include
 * ===========================================================================*/

#include "scli.h"

#include <stdbool.h>
#include <stdint.h>

#include "system/services/commands/date.h"
#include "system/services/commands/driver.h"
#include "system/services/commands/i2c.h"
#include "system/services/commands/thread.h"
#include "system/sysCall/error.h"
#include "system/sysCall/sc_hal.h"
#include "system/sysCall/sc_modules.h"
#include "tm_libc/tm_string.h"
#include "tm_libc/tm_syslog.h"

/* -----------------------------------------------
 * Constants
 * ---------------------------------------------*/

#define SCLI_LINE_SIZE 64
#define SCLI_ARGUMENT_COUNT_MAX 4

/* -----------------------------------------------
 * Private types
 * ---------------------------------------------*/

typedef bool (*scli_cmd_func_t)(uint8_t argc, char *argv[]);

typedef struct
{
	const char *name;
	scli_cmd_func_t func;
} scli_cmd_t;

/* -----------------------------------------------
 * Private variables
 * ---------------------------------------------*/

static char scli_line[SCLI_LINE_SIZE];
static uint8_t scli_line_length;

/* -----------------------------------------------
 * Private function prototypes
 * ---------------------------------------------*/

static err_codes_t scliRead(void);
static void scliLineProcess(void);
static uint8_t scliTokenize(char *line, char *argv[]);
static bool scliCommandDispatch(uint8_t argc, char *argv[]);

/* -----------------------------------------------
 * Command table
 * ---------------------------------------------*/

static const scli_cmd_t scli_commands[] = {
	{"date", dateCommand},
	{"driver", driver},
	{"i2c", i2c},
	{"thread", thread},
	{0, 0},
};

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

/* -----------------------------------------------
 * Service loop
 * ---------------------------------------------*/

void scli(void)
{
	while( 1 )
	{
		err_codes_t error = scliRead();
		if( error != ERR_NO_ERROR )
		{
			const tm_string_t *message = err_getMessage((uint8_t)error);
			if( message != 0 ) { tm_syslog(TM_STR("[scli] error: %s\n"), message); }
		}
		sc_threadSetSTC(50);
		while( sc_threadGetSTC() > 0 ) { sc_coopYield(); };
	}
}

/* -----------------------------------------------
 * Input processing
 * ---------------------------------------------*/

static err_codes_t scliRead(void)
{
	uint8_t data = 0;
	uint8_t i = 0;
	err_codes_t error = ERR_NO_ERROR;

	while( i < (sizeof(scli_line) - 1) )
	{
		error = sc_usartRead(&data);
		if( error != ERR_NO_ERROR ) { break; }
		scli_line[i++] = (char)data;
	}

	if( i > 0 )
	{
		scli_line[i] = 0;
		scliLineProcess();
	}

	if( error == ERR_HAL_USART_RX_BUFFER_EMPTY ) { return ERR_NO_ERROR; }
	return error;
}

static void scliLineProcess(void)
{
	char *argv[SCLI_ARGUMENT_COUNT_MAX];

	uint8_t argc = scliTokenize(scli_line, argv);
	if( (argc > 0) && !scliCommandDispatch(argc, argv) )
	{
		tm_string_t command = TM_STR_RAM(argv[0]);
		tm_syslog(TM_STR("[scli] error: unknown command %s\n"), &command);
		for( uint8_t i = 0; scli_commands[i].name != 0; i++ )
		{
			tm_string_t command_name = TM_STR_RAM(scli_commands[i].name);
			tm_syslog(TM_STR("\tcmd %s\n"), &command_name);
		}
	}
	scli_line_length = 0;
}

static uint8_t scliTokenize(char *line, char *argv[])
{
	uint8_t argc = 0;
	char *cursor = line;

	while( *cursor != 0 )
	{
		while( (*cursor == ' ') || (*cursor == '\t') ) { cursor++; }
		if( *cursor == 0 ) { break; }
		if( argc == SCLI_ARGUMENT_COUNT_MAX ) { return argc; }

		argv[argc++] = cursor;
		while( (*cursor != 0) && (*cursor != ' ') && (*cursor != '\t') ) { cursor++; }
		if( *cursor != 0 ) { *cursor++ = 0; }
	}

	return argc;
}

/* -----------------------------------------------
 * Command dispatch
 * ---------------------------------------------*/

static bool scliCommandDispatch(uint8_t argc, char *argv[])
{
	for( uint8_t i = 0; scli_commands[i].name != 0; i++ )
	{
		if( tm_strncmp(
				TM_STR_RAM(argv[0]), TM_STR_RAM(scli_commands[i].name), TM_STRING_SIZE_MAX) == 0 )
		{
			scli_commands[i].func(argc, argv);
			return true;
		}
	}

	return false;
}
