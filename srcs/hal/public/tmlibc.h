/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file tmlibc.h
 * @brief tmlibc header declarations.
 *
 */

#ifndef HAL_PUBLIC_TMLIBC_H
#define HAL_PUBLIC_TMLIBC_H

/* ============================================================================
 * Target selection
 * ========================================================================== */

#if defined(MCU_atmega2560)
	#include "hal/mcu/atmega2560/tmlibc.h"
	#define HAL_TMLIBC
#endif

#if !defined(HAL_TMLIBC)
	#error "No hal implementation for tm_libc on selected hardware target."
#endif

#endif // HAL_PUBLIC_TMLIBC_H
