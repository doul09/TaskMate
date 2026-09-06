/*
 * TaskMate Project
 * (c) 2025 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file globalError.c
 * @brief global error implementation.
 *
 */

/* =============================================================================
 * Declarations - Include
 * ===========================================================================*/

#include "globalError.h"

#include "fileUtility.h"
#include "tokenizer.h"

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

void globalError(const char *src_name, error_catalog_t *errors)
{
	AUTOCODE_MSG_INFO("open file.err <%s>", src_name);

	// Open files
	file_t file_src;
	fileInit(&file_src);
	file_src.name = (char *)src_name;
	fileOpen(&file_src, "r", FILE_READONLY, __FILE__, __LINE__);

	// Read from source
	int file_src_line_number = 0;
	int error_index = errors->error_count;
	tokenizer_t tok = {0};
	char line[TOKEN_LINE_SIZE_MAX];

	while( fgets(tok.line, TOKEN_LINE_SIZE_MAX, file_src.stream) )
	{
		file_src_line_number++;
		snprintf(line, sizeof(line), "%s", tok.line);
		tokenizer(&tok);

		if( (tok.count != 0) && (tok.tokens[0][0] != '#') )
		{
			if( tok.count != 3 )
			{
				AUTOCODE_MSG_ERROR("wrong token count != 3 tok.line [%s:%i] <%s>",
								   file_src.name,
								   file_src_line_number,
								   line);
				exit(1);
			}

			if( error_index >= ERROR_COUNT_MAX )
			{
				AUTOCODE_MSG_ERROR("Too many errors >= %i", ERROR_COUNT_MAX);
				exit(1);
			}

			for( int i = 0; i < error_index; i++ )
			{
				if( strcmp(tok.tokens[0], errors->catalog[i].name) == 0 )
				{
					AUTOCODE_MSG_ERROR("Duplicate error name <%s>", tok.tokens[0]);
					exit(1);
				}
			}
			const size_t name_length = strlen(tok.tokens[0]);
			const size_t message_length = strlen(tok.tokens[1]);
			if( (name_length >= sizeof(errors->catalog[error_index].name)) ||
				(message_length >= sizeof(errors->catalog[error_index].message)) )
			{
				AUTOCODE_MSG_ERROR("Error name or message is too long [%s:%i]",
								   file_src.name,
								   file_src_line_number);
				exit(1);
			}
			memcpy(errors->catalog[error_index].name, tok.tokens[0], name_length + 1U);
			memcpy(errors->catalog[error_index].message, tok.tokens[1], message_length + 1U);

			AUTOCODE_MSG_INFO("[%i] %s", error_index, tok.tokens[0]);

			if( strcmp(tok.tokens[2], "FLOW") == 0 )
			{
				errors->catalog[error_index].level = ERR_LEVEL_FLOW;
			}
			else if( strcmp(tok.tokens[2], "WARN") == 0 )
			{
				errors->catalog[error_index].level = ERR_LEVEL_WARN;
			}
			else if( strcmp(tok.tokens[2], "FAIL") == 0 )
			{
				errors->catalog[error_index].level = ERR_LEVEL_FAIL;
			}
			else if( strcmp(tok.tokens[2], "PANIC") == 0 )
			{
				errors->catalog[error_index].level = ERR_LEVEL_PANIC;
			}
			else
			{
				AUTOCODE_MSG_ERROR("wrong error level argument <%s>", tok.tokens[2]);
				exit(1);
			}

			error_index++;

			errors->error_count = error_index;
		}
	}

	tokenizerFree(&tok);
	fileClose(&file_src, __FILE__, __LINE__);
}
