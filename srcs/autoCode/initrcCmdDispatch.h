/*
 * TaskMate Project
 * (c) 2025 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file initrcCmdDispatch.h
 * @brief initrc cmd dispatch header declarations.
 *
 */

#ifndef AUTOCODE_INITRCCMDDISPATCH_H
#define AUTOCODE_INITRCCMDDISPATCH_H

/* ============================================================================
 * Includes
 * ========================================================================== */

#include "autoCode.h"

/* ============================================================================
 * Public definitions
 * ========================================================================== */

typedef enum
{
	INITRC_DISPATCH_OK,
	INITRC_DISPATCH_UNKNOWN_COMMAND,
	INITRC_DISPATCH_UNKNOWN_DATA
} initrc_dispatch_result_t;

typedef struct
{
	const char *name;
	initrc_dispatch_result_t (*func)(const char *data, module_item_t *mod);

} initrc_cmd_t;

/* ============================================================================
 * Public API
 * ========================================================================== */

initrc_dispatch_result_t initrcCmdDispatch(const char *cmd, const char *data, module_item_t *mod);

#endif // AUTOCODE_INITRCCMDDISPATCH_H
