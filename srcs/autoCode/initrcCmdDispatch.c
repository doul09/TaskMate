/*
 * TaskMate Project
 * (c) 2025 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file initrcCmdDispatch.c
 * @brief initrc cmd dispatch implementation.
 *
 */

/* =============================================================================
 * Declarations - Include
 * ===========================================================================*/

#include "initrcCmdDispatch.h"

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

/* -----------------------------------------------
 * Command handlers
 * ---------------------------------------------*/

static initrc_dispatch_result_t funcRun(const char *data, module_item_t *mod)
{
	if( strcmp(data, "none") == 0 ) { mod->status |= RL_RUN_NONE; }
	else if( strcmp(data, "core") == 0 ) { mod->status |= RL_RUN_CORE; }
	else if( strcmp(data, "driver") == 0 ) { mod->status |= RL_RUN_DRIVER; }
	else if( strcmp(data, "service") == 0 ) { mod->status |= RL_RUN_SERVICE; }
	else if( strcmp(data, "user") == 0 ) { mod->status |= RL_RUN_USER; }
	else { return INITRC_DISPATCH_UNKNOWN_DATA; }

	mod->cnt_set_runlevel++;
	return INITRC_DISPATCH_OK;
}

static initrc_dispatch_result_t funcType(const char *data, module_item_t *mod)
{
	if( strcmp(data, "driver") == 0 ) { mod->type = MOD_DRIVER_ID; }
	else if( strcmp(data, "service") == 0 )
	{
		mod->type = MOD_THREAD_ID;
		mod->subtype = THREAD_BIT_TYPE_SYS;
		mod->status |= (1 << THREAD_BIT_TYPE_SYS);
	}
	else if( strcmp(data, "user") == 0 )
	{
		mod->type = MOD_THREAD_ID;
		mod->subtype = THREAD_BIT_TYPE_USER;
		mod->status |= (1 << THREAD_BIT_TYPE_USER);
	}
	else { return INITRC_DISPATCH_UNKNOWN_DATA; }

	mod->cnt_set_type++;
	return INITRC_DISPATCH_OK;
}

static initrc_dispatch_result_t funcI2cAddress(const char *data, module_item_t *mod)
{
	char *end;
	const unsigned long address = strtoul(data, &end, 16);

	if( (data[0] == 0) || (*end != 0) || (address > MOD_I2C_ADDRESS_MAX) )
	{
		return INITRC_DISPATCH_UNKNOWN_DATA;
	}

	mod->address = (unsigned char)address;
	mod->cnt_set_address++;
	return INITRC_DISPATCH_OK;
}

static const initrc_cmd_t initrc_cmds[] = {
	{"-run", funcRun}, {"-type", funcType}, {"-i2c", funcI2cAddress}, {NULL, NULL}};

/* -----------------------------------------------
 * Command dispatch
 * ---------------------------------------------*/

initrc_dispatch_result_t initrcCmdDispatch(const char *cmd, const char *data, module_item_t *mod)
{
	for( int i = 0; initrc_cmds[i].name != NULL; i++ )
	{
		if( strcmp(cmd, initrc_cmds[i].name) == 0 ) { return (*initrc_cmds[i].func)(data, mod); }
	}
	return INITRC_DISPATCH_UNKNOWN_COMMAND;
}
