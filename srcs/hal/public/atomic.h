/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file atomic.h
 * @brief atomic header declarations.
 *
 */

#ifndef HAL_PUBLIC_ATOMIC_H
#define HAL_PUBLIC_ATOMIC_H

/* ============================================================================
 * Target selection
 * ========================================================================== */

#if defined(ARCH_avr8)
	#include "hal/arch/avr8/atomic.h"
	#define HAL_ATOMIC
#endif

#if !defined(HAL_ATOMIC)
	#error "No hal implementation for atomic block on selected hardware target."
#endif

#endif // HAL_PUBLIC_ATOMIC_H
