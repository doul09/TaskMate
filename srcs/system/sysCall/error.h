/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file error.h
 * @brief error header declarations.
 *
 */

#ifndef SYSCALL_ERROR_H
#define SYSCALL_ERROR_H

#include <stdint.h>

#include "interfaces/error_catalog.h"
#include "interfaces/error_level.h"
#include "interfaces/tm_string.h"

typedef struct
{
	const tm_string_t *name;
	const err_level_t level;
} err_item_t;

const tm_string_t *err_getMessage(uint8_t num);

#endif // SYSCALL_ERROR_H
