/*
 * TaskMate Project
 * (c) 2025 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file options.h
 * @brief options header declarations.
 *
 */

#ifndef AUTOCODE_OPTIONS_H
#define AUTOCODE_OPTIONS_H

/* ============================================================================
 * Includes
 * ========================================================================== */

#include "autoCode.h"

/* ============================================================================
 * Public definitions
 * ========================================================================== */

typedef struct
{
	char file_errors_list[BYTE_INDEX];
	char file_initrc_list[BYTE_INDEX];
	char file_parsetag_list[BYTE_INDEX];
	char file_halinit_list[BYTE_INDEX];
	char file_haldefine_list[BYTE_INDEX];
	char file_gpio_signals[BYTE_INDEX];
	
} options_list_t;

/* ============================================================================
 * Public API
 * ========================================================================== */

void options(const char *file_name, options_list_t *opt);

#endif // AUTOCODE_OPTIONS_H
