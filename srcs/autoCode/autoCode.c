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

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

int main(int argc, const char *argv[])
{
	tokenizer_t tok = {0};

	// Get options
	if( argc != 2 )
	{
		AUTOCODE_MSG_ERROR("autoCode bad argc (is %i, not 2)\n\tuse autoCode configuration_file",
						   argc);
		exit(1);
	}

	options_list_t auto_options;
	options(argv[1], &auto_options);

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

	while( fgets(tok.line, TOKEN_LINE_SIZE_MAX, ferror.stream) )
	{
		tokenizer(&tok);
		if( tok.count != 0 ) { globalError(tok.tokens[0], &errors_catalog); }
	}
	fileClose(&ferror, __FILE__, __LINE__);
	tokenizerFree(&tok);

	// Read init.rc files and store entries in the database
	file_t finitrc;
	fileInit(&finitrc);
	finitrc.name = auto_options.file_initrc_list;
	fileOpen(&finitrc, "r", FILE_READONLY, __FILE__, __LINE__);

	while( fgets(tok.line, TOKEN_LINE_SIZE_MAX, finitrc.stream) )
	{
		tokenizer(&tok);
		if( tok.count != 0 ) { parseInitrc(&data_base, tok.tokens[0]); }
	}
	fileClose(&finitrc, __FILE__, __LINE__);
	tokenizerFree(&tok);

	// Parse tags and generate code
	file_t ftag;
	fileInit(&ftag);
	ftag.name = auto_options.file_parsetag_list;
	fileOpen(&ftag, "r", FILE_READONLY, __FILE__, __LINE__);

	parseTagInit();
	while( fgets(tok.line, TOKEN_LINE_SIZE_MAX, ftag.stream) )
	{
		tokenizer(&tok);
		if( tok.count != 0 )
		{
			parseTag(&data_base, tok.tokens[0], &errors_catalog, &auto_options);
		}
	}
	fileClose(&ftag, __FILE__, __LINE__);
	tokenizerFree(&tok);
	parseTagHave();

	// Print module information
	printModules(&data_base);
	filePrintModified();
	return 0;
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
