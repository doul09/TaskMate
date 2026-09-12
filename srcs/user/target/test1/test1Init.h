/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file test1Init.h
 * @brief test1 init header declarations.
 *
 */

/* ============================================================================
 * Includes
 * ========================================================================== */

#include "hal/public/gpio.h"
#include "interfaces/gpio_signals.h"

/* ============================================================================
 * Public API
 * ========================================================================== */

void test1Init(void);
void targetWireSignal(hal_signal_t *table, gpio_signal_t signal);
