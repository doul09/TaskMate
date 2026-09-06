/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file sc_gpio.h
 * @brief sc gpio header declarations.
 */

#ifndef SYSCALL_SC_GPIO_H
#define SYSCALL_SC_GPIO_H

/* ============================================================================
 * Includes
 * ========================================================================== */

#include <stdbool.h>
#include <stdint.h>

#include "interfaces/gpio_signals.h"

/* ============================================================================
 * Public API
 * ========================================================================== */

void sc_gpio_signalSet(gpio_signal_t signal, bool val);
bool sc_gpio_signalGet(gpio_signal_t signal);
void sc_gpio_signalToggle(gpio_signal_t signal);

#endif // SYSCALL_SC_GPIO_H
