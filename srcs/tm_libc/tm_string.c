/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file tm_string.c
 * @brief tm string implementation.
 *
 */

/* =============================================================================
 * Declarations - Include
 * ===========================================================================*/

#include "tm_string.h"

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

int tm_strncmp(tm_string_t left, tm_string_t right, uint8_t n)
{
	for( uint8_t i = 0; i < n; i++ )
	{
		uint8_t left_char = 0;
		uint8_t right_char = 0;

		if( left.text != 0 ) { left_char = (uint8_t)hal_string_getChar(&left, i); }
		if( right.text != 0 ) { right_char = (uint8_t)hal_string_getChar(&right, i); }

		if( left_char < right_char ) { return -1; }
		if( left_char > right_char ) { return 1; }
		if( left_char == 0 ) { return 0; }
	}

	return 0;
}

void tm_strncpy(char *dest, tm_string_t src, uint8_t n)
{
	uint8_t i = 0;

	if( (dest == 0) || (n == 0) ) { return; }
	if( src.text == 0 )
	{
		dest[0] = 0;
		return;
	}

	while( (i < (uint8_t)(n - 1)) && (i < (TM_STRING_SIZE_MAX - 1)) )
	{
		char src_char = hal_string_getChar(&src, i);
		if( src_char == 0 ) { break; }
		dest[i] = src_char;
		i++;
	}
	dest[i] = 0;
}
