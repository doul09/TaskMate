/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file panic.h
 * @brief panic header declarations.
 *
 */

#ifndef HAL_PUBLIC_PANIC_H
#define HAL_PUBLIC_PANIC_H

/* ============================================================================
 * Target selection
 * ========================================================================== */

#if defined(ARCH_avr8)
	#include "hal/arch/avr8/panic.h"
	#define HAL_PANIC
#endif

#if !defined(HAL_PANIC)
	#error "No hal implementation for panic on selected hardware target."
#endif

#endif // HAL_PUBLIC_PANIC_H
