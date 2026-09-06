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

#ifndef HAL_AVR8_PANIC_H
#define HAL_AVR8_PANIC_H

/* ============================================================================
 * Includes
 * ========================================================================== */

#include "interfaces/tm_string.h"

/* ============================================================================
 * Public API
 * ========================================================================== */

void panic(tm_string_t msg) __attribute__((noreturn, cold));

#endif // HAL_AVR8_PANIC_H
