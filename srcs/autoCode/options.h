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
	unsigned int error_count;
	char file_errors_list[AUTOCODE_BUFFER_SIZE];
	char file_initrc_list[AUTOCODE_BUFFER_SIZE];
	char file_parsetag_list[AUTOCODE_BUFFER_SIZE];
	char file_halinit_list[AUTOCODE_BUFFER_SIZE];
	char file_funcinit_list[AUTOCODE_BUFFER_SIZE];
	char file_haldefine_list[AUTOCODE_BUFFER_SIZE];
	char file_gpio_signals[AUTOCODE_BUFFER_SIZE];

} options_list_t;

/* ============================================================================
 * Public API
 * ========================================================================== */

void options(const char *file_name, options_list_t *opt);

#endif // AUTOCODE_OPTIONS_H
