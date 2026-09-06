/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file tm_string.h
 * @brief tm string storage header declarations.
 *
 */

#ifndef INTERFACES_TM_STRING_H
#define INTERFACES_TM_STRING_H

/* ============================================================================
 * Public definitions
 * ========================================================================== */

#define TM_STRING_SIZE_MAX 255

typedef enum
{
	TM_MEM_RAM,
	TM_MEM_ROM
} tm_string_storage_t;

typedef struct
{
	const char *text;
	const tm_string_storage_t storage;
} tm_string_t;

#endif // INTERFACES_TM_STRING_H
