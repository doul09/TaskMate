/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file context.h
 * @brief context header declarations.
 *
 */

#ifndef HAL_PUBLIC_CONTEXT_H
#define HAL_PUBLIC_CONTEXT_H

/* ============================================================================
 * Target selection
 * ========================================================================== */

#if !defined(HAL_SYSTEM_CRITICAL_ALLOWED)
	#error "NOT ALLOWED INCLUDE : context.h"
#endif

#if defined(ARCH_avr8)
	#include "hal/arch/avr8/context.h"
	#define HAL_CONTEXT
#endif

#if !defined(HAL_CONTEXT)
	#error "No hal implementation for context switch on selected hardware target."
#endif

#endif // HAL_PUBLIC_CONTEXT_H
