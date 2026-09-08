/*
 * TaskMate Project
 * (c) 2025 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file parseTag.h
 * @brief parse tag header declarations.
 *
 */

#ifndef AUTOCODE_PARSETAG_H
#define AUTOCODE_PARSETAG_H

/* ============================================================================
 * Includes
 * ========================================================================== */

#include "autoCode.h"
#include "globalError.h"
#include "options.h"

/* ============================================================================
 * Public API
 * ========================================================================== */

void parseTagInit(void);
void parseTag(modules_database_t *data_base, const char *file_name, const error_catalog_t *errors,
			  const options_list_t *auto_options);
void parseTagHave(void);

#endif // AUTOCODE_PARSETAG_H
