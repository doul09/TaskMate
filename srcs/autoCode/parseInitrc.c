/*
 * TaskMate Project
 * (c) 2025 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file parseInitrc.c
 * @brief parse initrc implementation.
 *
 */

/* =============================================================================
 * Declarations - Include
 * ===========================================================================*/

#include "parseInitrc.h"

#include "fileUtility.h"
#include "initrcCmdDispatch.h"
#include "tokenizer.h"

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

void parseInitrc(modules_database_t *data_base, const char *initrc_name)
{
	// Open list files
	AUTOCODE_MSG_INFO("open <%s>", initrc_name);

	file_t initrc_list;
	fileInit(&initrc_list);
	initrc_list.name = (char *)initrc_name;
	fileOpen(&initrc_list, "r", FILE_READONLY, __FILE__, __LINE__);

	// Variables
	int file_line_number = 0;
	tokenizer_t tok = {0};
	module_item_t mod_tmp;
	file_get_line_result_t line_result;

	while( (line_result = fileGetLine(&initrc_list, tok.line, sizeof(tok.line))) ==
		   FILE_GET_LINE_SUCCESS )
	{
		// Start
		file_line_number++;
		tokenizer(&tok);

		// Process arguments
		if( (tok.count > 0) && (strcmp(tok.tokens[0], "#") != 0) ) // skip empty line or comment
		{
			if( (tok.count != 5) && (tok.count != 7) )
			{
				AUTOCODE_MSG_ERROR("wrong token count [%s:%i] is %i, should be 5 or 7",
								   initrc_name,
								   file_line_number,
								   tok.count);
				exit(1);
			}

			// Reset temporary module
			mod_tmp.status = 0;
			mod_tmp.type = 0;
			mod_tmp.subtype = 0;
			mod_tmp.address = TM_MOD_DRIVER_ADDRESS_NONE;
			mod_tmp.cnt_set_runlevel = 0;
			mod_tmp.cnt_set_type = 0;
			mod_tmp.cnt_set_address = 0;

			// Parse command/data pairs
			for( int i = 1; i < tok.count; i += 2 )
			{
				initrc_dispatch_result_t result =
					initrcCmdDispatch(tok.tokens[i], tok.tokens[i + 1], &mod_tmp);
				if( result == INITRC_DISPATCH_UNKNOWN_COMMAND )
				{
					AUTOCODE_MSG_ERROR(
						"unknown command [%s:%i] %s", initrc_name, file_line_number, tok.tokens[i]);
					exit(1);
				}
				if( result == INITRC_DISPATCH_UNKNOWN_DATA )
				{
					AUTOCODE_MSG_ERROR("unknown data [%s:%i] %s for command %s",
									   initrc_name,
									   file_line_number,
									   tok.tokens[i + 1],
									   tok.tokens[i]);
					exit(1);
				}
			}

			// Process name
			if( autoCodeBufferStringFits(tok.tokens[0], sizeof(mod_tmp.name)) == false )
			{
				AUTOCODE_MSG_ERROR(
					"Name too long <%s> (maximum %zu characters)",
					tok.tokens[0],
					sizeof(mod_tmp.name) - 1U);
				exit(1);
			}

			AUTOCODE_MSG_INFO("found module : %s", tok.tokens[0]);

			// Continue with the selected initrc type
			module_type_t *mod = &data_base->modules_type[mod_tmp.type];
			int module_count = mod->modules_count;

			// Check for duplicate names
			for( int j = 0; j < module_count; j++ )
			{
				if( strcmp(mod->modules[j].name, tok.tokens[0]) == 0 )
				{
					AUTOCODE_MSG_ERROR("duplicate name [%s:%i] %s\n\n",
									   initrc_name,
									   file_line_number,
									   tok.tokens[0]);
					exit(1);
				}
			}

			// Check options
			if( mod_tmp.cnt_set_runlevel < 1 )
			{
				AUTOCODE_MSG_ERROR("Module %s : -run option is not set", tok.tokens[0]);
				exit(1);
			}
			if( mod_tmp.cnt_set_runlevel > 1 )
			{
				AUTOCODE_MSG_ERROR("Module %s : -run option is multiple set", tok.tokens[0]);
				exit(1);
			}
			if( mod_tmp.cnt_set_type < 1 )
			{
				AUTOCODE_MSG_ERROR("Module %s : -type option is not set", tok.tokens[0]);
				exit(1);
			}
			if( mod_tmp.cnt_set_type > 1 )
			{
				AUTOCODE_MSG_ERROR("Module %s : -type option is multiple set", tok.tokens[0]);
				exit(1);
			}
			if( mod_tmp.cnt_set_address > 1 )
			{
				AUTOCODE_MSG_ERROR("Module %s : -i2c option is multiple set", tok.tokens[0]);
				exit(1);
			}
			if( (mod_tmp.cnt_set_address == 1) && (mod_tmp.type != TM_MOD_DRIVER_ID) )
			{
				AUTOCODE_MSG_ERROR("Module %s : -i2c option is only valid for drivers",
								   tok.tokens[0]);
				exit(1);
			}

			// Copy the temporary module to the destination module
			int index = mod->modules_count;
			if( index > TM_MOD_COUNT_MAX - 1 )
			{
				AUTOCODE_MSG_ERROR("too much modules > %i type=%i\n", index, mod_tmp.type);
				exit(1);
			}

			snprintf(
				mod->modules[index].name, sizeof(mod->modules[index].name), "%s", tok.tokens[0]);
			mod->modules[index].status = mod_tmp.status;
			mod->modules_count = index + 1;
			mod->modules[index].subtype = mod_tmp.subtype;
			mod->modules[index].address = mod_tmp.address;
		}
	}
	if( line_result == FILE_GET_LINE_ERROR )
	{
		AUTOCODE_MSG_ERROR("reading file <%s> after line %i", initrc_list.name, file_line_number);
		exit(1);
	}
	tokenizerFree(&tok);
	fileClose(&initrc_list, __FILE__, __LINE__);
}
