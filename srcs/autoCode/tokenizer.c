/*
 * TaskMate Project
 * (c) 2025 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file tokenizer.c
 * @brief tokenizer implementation.
 *
 */

/* =============================================================================
 * Declarations - Include
 * ===========================================================================*/

#include "tokenizer.h"

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

void tokenizer(tokenizer_t *tok)
{
	char *cursor = tok->line;

	// Start reading the line to extract arguments
	tokenizerFree(tok);

	while( (*cursor != '\n') && (*cursor != 0) )
	{
		// Skip leading spaces and tabs
		while( (*cursor == ' ') || (*cursor == '\t') ) { cursor++; }

		if( (*cursor == '\n') || (*cursor == 0) ) { break; }

		// Point to one token stored directly in the line
		char cut_character = ' ';
		char *token = cursor;
		bool quoted_string = false;

		if( *cursor == '"' ) // switch to string mode for this token
		{
			cut_character = '"';
			quoted_string = true;
			cursor++;
		}

		while( (*cursor != cut_character) && (quoted_string || (*cursor != '\t')) &&
			   (*cursor != '\n') && (*cursor != 0) )
		{
			cursor++;
		}

		if( quoted_string && (*cursor != '"') )
		{
			AUTOCODE_MSG_ERROR("unterminated string");
			exit(1);
		}

		if( *cursor == '"' ) { cursor++; }

		char **tokens = realloc(tok->tokens, (size_t)(tok->count + 1) * sizeof(*tok->tokens));
		if( tokens == NULL )
		{
			AUTOCODE_MSG_ERROR("realloc tokenizer token %i", tok->count);
			exit(1);
		}
		tok->tokens = tokens;
		tok->tokens[tok->count] = token;

		if( *cursor != 0 )
		{
			*cursor = 0;
			cursor++;
		}
		tok->count++;
	}
}

void tokenizerFree(tokenizer_t *tok)
{
	free(tok->tokens);
	tok->tokens = NULL;
	tok->count = 0;
}
