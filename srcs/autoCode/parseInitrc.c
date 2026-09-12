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

#include <errno.h>

#include "fileUtility.h"
#include "initrcCmdDispatch.h"
#include "tokenizer.h"

/* =============================================================================
 * Declarations - Local types
 * ===========================================================================*/

typedef enum
{
	AC_INITRC_VERSION_EXPECT_MAJOR,
	AC_INITRC_VERSION_EXPECT_MINOR,
	AC_INITRC_VERSION_VALID,
	AC_INITRC_VERSION_INVALID
} initrc_version_state_t;

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

static bool initrcVersionValueMatches(const char *value, const unsigned long expected)
{
	char *end;

	errno = 0;
	const unsigned long parsed = strtoul(value, &end, 10);
	if( (value[0] == 0) || (*end != 0) || (errno == ERANGE) ) { return false; }
	return parsed == expected;
}

static bool initrcVersionHeaderParse(const tokenizer_t *tok,
									 initrc_version_state_t *state,
									 const char *initrc_name,
									 const int file_line_number)
{
	if( *state == AC_INITRC_VERSION_EXPECT_MAJOR )
	{
		if( (file_line_number != 1) || (tok->count != 2) ||
			(strcmp(tok->tokens[0], "!set_version_major") != 0) )
		{
			AUTOCODE_MSG_ERROR("first init.rc line [%s:%i] must be !set_version_major %i",
							   initrc_name,
							   file_line_number,
							   AC_SYNTAX_VERSION_MAJOR);
			*state = AC_INITRC_VERSION_INVALID;
			return false;
		}
		if( initrcVersionValueMatches(tok->tokens[1], AC_SYNTAX_VERSION_MAJOR) == false )
		{
			AUTOCODE_MSG_ERROR("unsupported init.rc major syntax version [%s:%i] %s, expected %i",
							   initrc_name,
							   file_line_number,
							   tok->tokens[1],
							   AC_SYNTAX_VERSION_MAJOR);
			*state = AC_INITRC_VERSION_INVALID;
			return false;
		}

		*state = AC_INITRC_VERSION_EXPECT_MINOR;
		return false;
	}

	if( *state == AC_INITRC_VERSION_EXPECT_MINOR )
	{
		if( (file_line_number != 2) || (tok->count != 2) ||
			(strcmp(tok->tokens[0], "!set_version_minor") != 0) )
		{
			AUTOCODE_MSG_ERROR("second init.rc line [%s:%i] must be !set_version_minor %i",
							   initrc_name,
							   file_line_number,
							   AC_SYNTAX_VERSION_MINOR);
			*state = AC_INITRC_VERSION_INVALID;
			return false;
		}
		if( initrcVersionValueMatches(tok->tokens[1], AC_SYNTAX_VERSION_MINOR) == false )
		{
			AUTOCODE_MSG_ERROR("unsupported init.rc minor syntax version [%s:%i] %s, expected %i",
							   initrc_name,
							   file_line_number,
							   tok->tokens[1],
							   AC_SYNTAX_VERSION_MINOR);
			*state = AC_INITRC_VERSION_INVALID;
			return false;
		}

		*state = AC_INITRC_VERSION_VALID;
		return false;
	}

	if( *state == AC_INITRC_VERSION_INVALID ) { return false; }
	if( (tok->count > 0) && (tok->tokens[0][0] == '!') )
	{
		AUTOCODE_MSG_ERROR("init.rc version command outside header [%s:%i] %s",
						   initrc_name,
						   file_line_number,
						   tok->tokens[0]);
		return false;
	}

	return true;
}

void parseInitrc(modules_database_t *data_base, const char *initrc_name)
{
	// Open list files
	AUTOCODE_MSG_INFO("open <%s>", initrc_name);

	file_t initrc_list;
	fileInit(&initrc_list);
	initrc_list.name = (char *)initrc_name;
	if( fileOpen(&initrc_list, "r", FILE_READONLY, __FILE__, __LINE__) != 0 ) { return; }

	// Variables
	int file_line_number = 0;
	tokenizer_t tok = {0};
	module_item_t mod_tmp;
	file_get_line_result_t line_result;
	initrc_version_state_t version_state = AC_INITRC_VERSION_EXPECT_MAJOR;

	while( (line_result = fileGetLine(&initrc_list, tok.line, sizeof(tok.line))) ==
		   FILE_GET_LINE_SUCCESS )
	{
		// Start
		file_line_number++;
		if( tokenizer(&tok) != 0 ) { continue; }
		const bool line_is_module =
			initrcVersionHeaderParse(&tok, &version_state, initrc_name, file_line_number);
		if( version_state == AC_INITRC_VERSION_INVALID ) { break; }
		if( line_is_module == false )
		{
			continue;
		}

		// Process arguments
		if( (tok.count > 0) && (strcmp(tok.tokens[0], "#") != 0) ) // skip empty line or comment
		{
			bool module_is_valid = true;

			if( (tok.count != 5) && (tok.count != 7) )
			{
				AUTOCODE_MSG_ERROR("wrong token count [%s:%i] is %i, should be 5 or 7",
								   initrc_name,
								   file_line_number,
								   tok.count);

				continue;
			}

			// Reset temporary module
			mod_tmp.status = 0;
			mod_tmp.type = 0;
			mod_tmp.subtype = 0;
			mod_tmp.address = MOD_DRIVER_ADDRESS_NONE;
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

					module_is_valid = false;
				}
				if( result == INITRC_DISPATCH_UNKNOWN_DATA )
				{
					AUTOCODE_MSG_ERROR("unknown data [%s:%i] %s for command %s",
									   initrc_name,
									   file_line_number,
									   tok.tokens[i + 1],
									   tok.tokens[i]);

					module_is_valid = false;
				}
			}
			if( module_is_valid == false ) { continue; }

			// Process name
			if( autoCodeBufferStringFits(tok.tokens[0], sizeof(mod_tmp.name)) == false )
			{
				AUTOCODE_MSG_ERROR("Name too long <%s> (maximum %zu characters)",
								   tok.tokens[0],
								   sizeof(mod_tmp.name) - 1U);

				continue;
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

					module_is_valid = false;
				}
			}

			// Check options
			if( mod_tmp.cnt_set_runlevel < 1 )
			{
				AUTOCODE_MSG_ERROR("Module %s : -run option is not set", tok.tokens[0]);

				module_is_valid = false;
			}
			if( mod_tmp.cnt_set_runlevel > 1 )
			{
				AUTOCODE_MSG_ERROR("Module %s : -run option is multiple set", tok.tokens[0]);

				module_is_valid = false;
			}
			if( mod_tmp.cnt_set_type < 1 )
			{
				AUTOCODE_MSG_ERROR("Module %s : -type option is not set", tok.tokens[0]);

				module_is_valid = false;
			}
			if( mod_tmp.cnt_set_type > 1 )
			{
				AUTOCODE_MSG_ERROR("Module %s : -type option is multiple set", tok.tokens[0]);

				module_is_valid = false;
			}
			if( mod_tmp.cnt_set_address > 1 )
			{
				AUTOCODE_MSG_ERROR("Module %s : -i2c option is multiple set", tok.tokens[0]);

				module_is_valid = false;
			}
			if( (mod_tmp.cnt_set_address == 1) && (mod_tmp.type != MOD_DRIVER_ID) )
			{
				AUTOCODE_MSG_ERROR("Module %s : -i2c option is only valid for drivers",
								   tok.tokens[0]);

				module_is_valid = false;
			}
			if( module_is_valid == false ) { continue; }

			// Copy the temporary module to the destination module
			int index = mod->modules_count;
			if( index > MOD_COUNT_MAX - 1 )
			{
				AUTOCODE_MSG_ERROR("too much modules > %i type=%i\n", index, mod_tmp.type);

				continue;
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
	}
	if( version_state == AC_INITRC_VERSION_EXPECT_MAJOR )
	{
		AUTOCODE_MSG_ERROR("missing !set_version_major %i on first line of init.rc file <%s>",
						   AC_SYNTAX_VERSION_MAJOR,
						   initrc_name);
	}
	if( version_state == AC_INITRC_VERSION_EXPECT_MINOR )
	{
		AUTOCODE_MSG_ERROR("missing !set_version_minor %i after init.rc major version <%s>",
						   AC_SYNTAX_VERSION_MINOR,
						   initrc_name);
	}
	tokenizerFree(&tok);
	fileClose(&initrc_list, __FILE__, __LINE__);
}
