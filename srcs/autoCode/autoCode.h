/*
 * TaskMate Project
 * (c) 2025 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file autoCode.h
 * @brief auto code header declarations.
 *
 */

#ifndef AUTOCODE_AUTOCODE_H
#define AUTOCODE_AUTOCODE_H

/* ============================================================================
 * Constants
 * ========================================================================== */

#define AUTOCODE_BUFFER_SIZE 256

/* ============================================================================
 * Includes
 * ========================================================================== */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Get TaskMate definitions
#define TM_SYSTEM_CRITICAL_ALLOWED
#include "interfaces/tm_modules.h"
#include "interfaces/tm_runLevel.h"

/* ============================================================================
 * Public definitions
 * ========================================================================== */

/* Buffer capacity includes the trailing NUL byte. */
static inline bool autoCodeBufferIndexIsValid(const size_t index, const size_t buffer_size)
{
	if( buffer_size < 2U ) { return false; }
	if( index > buffer_size - 2U ) { return false; }
	return true;
}

static inline bool autoCodeBufferStringFits(const char *string, const size_t buffer_size)
{
	const size_t string_length = strlen(string);

	if( string_length == 0U ) { return buffer_size > 0U; }
	return autoCodeBufferIndexIsValid(string_length - 1U, buffer_size);
}

/* -----------------------------------------------
 * Message macros
 * ---------------------------------------------*/

#define AUTOCODE_MSG_ERROR(format, ...)                                                          \
	do {                                                                                         \
		fprintf(stderr, "[%s:%d] error : " format "\n", __FILE_NAME__, __LINE__, ##__VA_ARGS__); \
		perror("\t"); \
		autoCodeExit(AC_INCREMENT); \
	} while( 0 )

#define AUTOCODE_MSG_INFO(format, ...) \
	fprintf(stdout, "[%s] info : " format "\n", __FILE_NAME__, ##__VA_ARGS__)

/* -----------------------------------------------
 * Error handling
 * ---------------------------------------------*/

typedef enum
{
	AC_INCREMENT,
	AC_FORCE_EXIT
} ac_error_cmd_t;

void autoCodeExit(ac_error_cmd_t cmd);

/* -----------------------------------------------
 * Module database types
 * ---------------------------------------------*/

typedef struct
{
	char name[TM_MOD_NAME_SIZE_MAX];
	unsigned char status;
	unsigned char type;
	unsigned char subtype;
	unsigned char address;
	int cnt_set_runlevel;
	int cnt_set_type;
	int cnt_set_address;

} module_item_t;

typedef struct
{
	module_item_t modules[TM_MOD_COUNT_MAX];
	int modules_count;

} module_type_t;

typedef struct
{
	module_type_t modules_type[TM_MOD_TYPE_COUNT];

} modules_database_t;

#endif // AUTOCODE_AUTOCODE_H
