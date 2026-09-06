/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file tm_string.h
 * @brief tm string header declarations.
 *
 */

#ifndef TM_LIBC_TM_STRING_H
#define TM_LIBC_TM_STRING_H

/* ============================================================================
 * Target selection
 * ========================================================================== */

// clang-format off

#include "interfaces/tm_options.h" // get libc selection
#include "interfaces/tm_string.h"


#if TM_LIBC_CSTD
	#include <string.h>
	#define tm_strncmp strncmp
	#define tm_strncpy strncpy

	#define TM_STR_RAM(string) (string)
	#define TM_STR_ROM(string) (string)
	#define TM_STR(string) (string)
	#define TM_STR_ROM_NEW(name, txt) \
		const char name[] = (txt);
#endif

#if TM_LIBC_TASKMATE
	#include <stdint.h>
	#include "hal/public/tmlibc.h" // HAL definitions for the TM_STR_* macros

	int tm_strncmp(tm_string_t left, tm_string_t right, uint8_t n);
	void tm_strncpy(char *dest, tm_string_t src, uint8_t n);


#endif

// clang-format on

#endif // TM_LIBC_TM_STRING_H
