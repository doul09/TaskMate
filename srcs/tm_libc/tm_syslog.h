/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file tm_syslog.h
 * @brief tm syslog header declarations.
 *
 */

#ifndef TM_LIBC_TM_SYSLOG_H
#define TM_LIBC_TM_SYSLOG_H

/* ============================================================================
 * Target selection
 * ========================================================================== */

// clang-format off

#include "tm_string.h"
#include "interfaces/tm_options.h" // get libc selection

#if TM_LIBC_CSTD
	#include <syslog.h>
#endif

#if TM_LIBC_TASKMATE
	#include <stdarg.h>
	void tm_syslog(tm_string_t format, ...);
	void tm_vsyslog(tm_string_t format, va_list args);
#endif

// clang-format on

#endif // TM_LIBC_TM_SYSLOG_H
