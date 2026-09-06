/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file gpio.c
 * @brief gpio implementation.
 */

#include "gpio.h"

#include "hal/public/gpio.h"
#include "system/sysCore/hal_init.h"

static hal_signal_t signal_table[GPIO_SIGNAL_COUNT];

void gpio_signalsInit(void)
{
	for( uint8_t i = 0; i < GPIO_SIGNAL_COUNT; i++ )
	{
		targetWireSignal(signal_table, i);
		hal_gpioPinInit(&signal_table[i].pin);
	}
}

void gpio_signalSet(gpio_signal_t signal, bool val)
{
	hal_gpioPinWrite(signal_table[signal].pin, val);
}
bool gpio_signalGet(gpio_signal_t signal) { return hal_gpioPinRead(signal_table[signal].pin); }
