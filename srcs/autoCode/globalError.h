/*
 * TaskMate Project
 * (c) 2025 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file globalError.h
 * @brief global error header declarations.
 *
 */

#ifndef AUTOCODE_GLOBALERROR_H
#define AUTOCODE_GLOBALERROR_H

/* ============================================================================
 * Includes
 * ========================================================================== */

#include "autoCode.h"
#include "interfaces/error_level.h"

/* ============================================================================
 * Public definitions
 * ========================================================================== */

#define ERROR_COUNT_MAX 256

typedef struct
{
	char name[BYTE_INDEX];
	char message[BYTE_INDEX];
	err_level_t level;
} error_item_t;

typedef struct
{
	error_item_t catalog[ERROR_COUNT_MAX];
	int error_count;
} error_catalog_t;

/* ============================================================================
 * Public API
 * ========================================================================== */

void globalError(const char *src_name, error_catalog_t *errors);

#endif // AUTOCODE_GLOBALERROR_H
