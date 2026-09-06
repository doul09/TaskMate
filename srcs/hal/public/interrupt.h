/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file interrupt.h
 * @brief interrupt header declarations.
 *
 */

#ifndef HAL_PUBLIC_INTERRUPT_H
#define HAL_PUBLIC_INTERRUPT_H

/* ============================================================================
 * Target selection
 * ========================================================================== */

#if !defined(HAL_SYSTEM_CRITICAL_ALLOWED)
	#error "NOT ALLOWED INCLUDE : interrupt.h"
#endif

#if defined(ARCH_avr8)
	#include "hal/arch/avr8/interrupt.h"
	#define HAL_INT
#endif

#if !defined(HAL_INT)
	#error "No hal implementation for interrupt on selected hardware target."
#endif

#endif // HAL_PUBLIC_INTERRUPT_H
