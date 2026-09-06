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

#define BYTE_INDEX 256

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

/* -----------------------------------------------
 * Message macros
 * ---------------------------------------------*/

#define AUTOCODE_MSG_ERROR(format, ...)                                                          \
	do {                                                                                         \
		fprintf(stderr, "[%s:%d] error : " format "\n", __FILE_NAME__, __LINE__, ##__VA_ARGS__); \
		perror("\t");                                                                            \
	} while( 0 )

#define AUTOCODE_MSG_INFO(format, ...) \
	fprintf(stdout, "[%s] info : " format "\n", __FILE_NAME__, ##__VA_ARGS__)

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
