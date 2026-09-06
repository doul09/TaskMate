/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file sc_gpio.c
 * @brief sc gpio implementation.
 */

/* =============================================================================
 * Declarations - Include
 * ===========================================================================*/

#include "sc_gpio.h"

#include "system/sysCore/gpio.h"

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

void sc_gpio_signalSet(gpio_signal_t signal, bool val) { gpio_signalSet(signal, val); }
bool sc_gpio_signalGet(gpio_signal_t signal) { return gpio_signalGet(signal); }
void sc_gpio_signalToggle(gpio_signal_t signal) { gpio_signalSet(signal, !gpio_signalGet(signal)); }
