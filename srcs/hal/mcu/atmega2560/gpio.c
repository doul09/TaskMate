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
 *
 */

/* =============================================================================
 * Declarations - Include
 * ===========================================================================*/

#include "gpio.h"

#include <avr/io.h>
#include <stdbool.h>

#include "hal/arch/avr8/define.h"
#include "interfaces/tm_macros.h"

/* -----------------------------------------------
 * Private variables
 * ---------------------------------------------*/

static const hal_port_t mcu_ports[PORT_COUNT] = {
	[PORT_A] = (hal_port_t){(volatile uint8_t *)_SFR_MEM_ADDR(DDRA),
							(volatile uint8_t *)_SFR_MEM_ADDR(PORTA),
							(volatile uint8_t *)_SFR_MEM_ADDR(PINA)},
	[PORT_B] = (hal_port_t){(volatile uint8_t *)_SFR_MEM_ADDR(DDRB),
							(volatile uint8_t *)_SFR_MEM_ADDR(PORTB),
							(volatile uint8_t *)_SFR_MEM_ADDR(PINB)} /*,
			[PORT_C] = { DDRC, PORTC, PINC },
			[PORT_D] = { DDRD, PORTD, PIND },
			[PORT_E] = { DDRE, PORTE, PINE },
			[PORT_F] = { DDRF, PORTF, PINF },
			[PORT_G] = { DDRG, PORTG, PING },
			[PORT_H] = { DDRH, PORTH, PINH },
			[PORT_I] = { DDRI, PORTI, PINI },
			[PORT_J] = { DDRJ, PORTJ, PINJ },
			[PORT_K] = { DDRK, PORTK, PINK },
			[PORT_L] = { DDRL, PORTL, PINL }*/

};

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

void hal_gpioPinInit(const hal_pin_t *pin)
{
	if( pin->mode == GPIO_PIN_MODE_INPUT )
	{
		TM_CLEARBIT(*(mcu_ports[pin->port].ddr), pin->number);
	}
	if( pin->mode == GPIO_PIN_MODE_OUTPUT_PP )
	{
		TM_SETBIT(*(mcu_ports[pin->port].ddr), pin->number);
	}

	if( pin->pull == GPIO_PIN_PULL_UP ) { TM_SETBIT(*(mcu_ports[pin->port].port), pin->number); }
}

void hal_gpioPinWrite(const hal_pin_t pin, bool value)
{
	if( value ) { TM_SETBIT(*(mcu_ports[pin.port].port), pin.number); }
	else { TM_CLEARBIT(*(mcu_ports[pin.port].port), pin.number); }
}

bool hal_gpioPinRead(const hal_pin_t pin)
{
	if( pin.mode == GPIO_PIN_MODE_INPUT )
	{
		return (*(mcu_ports[pin.port].pin) & (1 << pin.number)) >> pin.number;
	}
	return (*(mcu_ports[pin.port].port) & (1 << pin.number)) >> pin.number;
}
