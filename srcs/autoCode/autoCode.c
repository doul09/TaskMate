/*
 * TaskMate Project
 * (c) 2025 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file autoCode.c
 * @brief auto code implementation.
 *
 * - Simple and reliable; reads plain-text init.rc files
 * - Writes includes, static allocations, and initialisation directly into code
 *
 * @note
 * Tag format is a one-line C comment: // [autoCode_tag] <object> <action>
 * End-tag format: // [/tag]
 *
 */

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * autoCode is a critical component: if it generates incorrect code,
 * TaskMate may still compile but will behave unpredictably at runtime.
 * Any change to autoCode must be considered system-critical and tested
 * accordingly.
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * */

/* =============================================================================
 * Declarations - Include
 * ===========================================================================*/

#include "autoCode.h"

#include "fileUtility.h"
#include "globalError.h"
#include "options.h"
#include "parseInitrc.h"
#include "parseTag.h"
#include "printModules.h"
#include "tokenizer.h"

/* -----------------------------------------------
 * Private function prototypes
 * ---------------------------------------------*/

static void setupDatabase(modules_database_t *data_base);

/* -----------------------------------------------
 * Private variables
 * ---------------------------------------------*/

static unsigned int error_count = 0U;
static unsigned int error_count_maximum = 10U;

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

void autoCodeExit(ac_error_cmd_t cmd)
{
	if( cmd == AC_INCREMENT )
	{
		error_count++;
		if( error_count > error_count_maximum ) { exit(EXIT_FAILURE); }
	}
	if( cmd == AC_FORCE_EXIT )
	{
		if( error_count > 0 ) { exit(EXIT_FAILURE); }
	}
}

int main(int argc, const char *argv[])
{
	tokenizer_t tok = {0};

	// Get options
	if( argc != 2 )
	{
		AUTOCODE_MSG_ERROR("autoCode bad argc (is %i, not 2)\n\tuse autoCode configuration_file",
						   argc);
		return EXIT_FAILURE;
	}

	options_list_t auto_options = {0};
	options(argv[1], &auto_options);
	autoCodeExit(AC_FORCE_EXIT);

	// Set up database
	modules_database_t data_base;
	setupDatabase(&data_base);

	// Read error files and store entries in the error catalogue
	error_catalog_t errors_catalog;
	errors_catalog.error_count = 0;

	file_t ferror;
	fileInit(&ferror);
	ferror.name = auto_options.file_errors_list;
	fileOpen(&ferror, "r", FILE_READONLY, __FILE__, __LINE__);

	file_get_line_result_t line_result;
	while( (line_result = fileGetLine(&ferror, tok.line, sizeof(tok.line))) ==
		   FILE_GET_LINE_SUCCESS )
	{
		if( tokenizer(&tok) != 0 ) { continue; }
		if( tok.count != 0 ) { globalError(tok.tokens[0], &errors_catalog); }
	}
	if( line_result == FILE_GET_LINE_ERROR )
	{
		AUTOCODE_MSG_ERROR("reading file <%s>", ferror.name);
	}
	fileClose(&ferror, __FILE__, __LINE__);
	tokenizerFree(&tok);
	autoCodeExit(AC_FORCE_EXIT);

	// Read init.rc files and store entries in the database
	file_t finitrc;
	fileInit(&finitrc);
	finitrc.name = auto_options.file_initrc_list;
	fileOpen(&finitrc, "r", FILE_READONLY, __FILE__, __LINE__);

	while( (line_result = fileGetLine(&finitrc, tok.line, sizeof(tok.line))) ==
		   FILE_GET_LINE_SUCCESS )
	{
		if( tokenizer(&tok) != 0 ) { continue; }
		if( tok.count != 0 ) { parseInitrc(&data_base, tok.tokens[0]); }
	}
	if( line_result == FILE_GET_LINE_ERROR )
	{
		AUTOCODE_MSG_ERROR("reading file <%s>", finitrc.name);
	}
	fileClose(&finitrc, __FILE__, __LINE__);
	tokenizerFree(&tok);
	autoCodeExit(AC_FORCE_EXIT);

	// Parse tags and generate code
	file_t ftag;
	fileInit(&ftag);
	ftag.name = auto_options.file_parsetag_list;
	fileOpen(&ftag, "r", FILE_READONLY, __FILE__, __LINE__);

	parseTagInit();
	while( (line_result = fileGetLine(&ftag, tok.line, sizeof(tok.line))) == FILE_GET_LINE_SUCCESS )
	{
		if( tokenizer(&tok) != 0 ) { continue; }
		if( tok.count != 0 )
		{
			parseTag(&data_base, tok.tokens[0], &errors_catalog, &auto_options);
		}
	}
	if( line_result == FILE_GET_LINE_ERROR ) { AUTOCODE_MSG_ERROR("reading file <%s>", ftag.name); }
	fileClose(&ftag, __FILE__, __LINE__);
	tokenizerFree(&tok);
	parseTagHave();
	autoCodeExit(AC_FORCE_EXIT);

	// Compare and replace temp files
	fileCmpReplaceAll();

	// Print module information
	printModules(&data_base);
	filePrintModified();
	return EXIT_SUCCESS;
}

static void setupDatabase(modules_database_t *data_base)
{
	for( int i = 0; i < TM_MOD_TYPE_COUNT; i++ )
	{
		data_base->modules_type[i].modules_count = 0;
		for( int j = 0; j < TM_MOD_COUNT_MAX; j++ )
		{
			data_base->modules_type[i].modules[j].cnt_set_runlevel = 0;
			data_base->modules_type[i].modules[j].cnt_set_type = 0;
			data_base->modules_type[i].modules[j].cnt_set_address = 0;
		}
	}
}
