/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file tm_stdio.h
 * @brief tm stdio header declarations.
 *
 */

#ifndef TM_LIBC_TM_STDIO_H
#define TM_LIBC_TM_STDIO_H

// clang-format off

#include "interfaces/tm_options.h" // get libc selection

#if TM_LIBC_CSTD
	#include <stdio.h>
	#define tm_snprintf snprintf
	#define tm_printf printf
#endif

#if TM_LIBC_TASKMATE
	#include "tm_libc/tm_snprintf.h"
#endif

// clang-format on

#endif // TM_LIBC_TM_STDIO_H
