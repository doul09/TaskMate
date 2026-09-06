/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file tm_snprintf.h
 * @brief tm snprintf header declarations.
 *
 */

#ifndef TM_LIBC_TM_SNPRINTF_H
#define TM_LIBC_TM_SNPRINTF_H

/* ============================================================================
 * Includes
 * ========================================================================== */

#include <stdarg.h>
#include <stdint.h>

#include "tm_string.h"

/* ============================================================================
 * Public API
 * ========================================================================== */

/* -----------------------------------------------
 * Stream formatting
 * ---------------------------------------------*/

int tm_printf(tm_string_t format, ...);
int tm_vprintf(tm_string_t format, va_list args);

/* -----------------------------------------------
 * Bounded buffer formatting
 * ---------------------------------------------*/

int tm_snprintf(char *ptr, uint8_t size, tm_string_t format, ...);
int tm_vsnprintf(char *ptr, uint8_t size, tm_string_t format, va_list args);

#endif // TM_LIBC_TM_SNPRINTF_H
