/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file gpio_common.h
 * @brief gpio common header declarations.
 */

#ifndef INTERFACES_GPIO_COMMON_H
#define INTERFACES_GPIO_COMMON_H

/* ============================================================================
 * Public definitions
 * ========================================================================== */

typedef enum
{
	GPIO_PIN_MODE_INPUT,
	GPIO_PIN_MODE_OUTPUT_PP, // push-pull
	GPIO_PIN_MODE_OUTPUT_OD, // open-drain
	GPIO_PIN_MODE_OUTPUT_HZ, // high impedance
} gpio_pin_mode_t;

typedef enum
{
	GPIO_PIN_PULL_NONE,
	GPIO_PIN_PULL_UP,
	GPIO_PIN_PULL_DOWN
} gpio_pin_pull_t;

#endif // INTERFACES_GPIO_COMMON_H
