/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file tm_syslog.c
 * @brief tm syslog implementation.
 *
 */

/* =============================================================================
 * Declarations - Include
 * ===========================================================================*/

#include "tm_syslog.h"

#include "tm_stdio.h"

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

void tm_syslog(const tm_string_t format, ...)
{
	va_list args;
	va_start(args, format);
	tm_vsyslog(format, args);
	va_end(args);
}

void tm_vsyslog(const tm_string_t format, va_list args) { tm_vprintf(format, args); }
