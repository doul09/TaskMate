/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file usart.c
 * @brief usart implementation.
 *
 */

/* =============================================================================
 * Declarations - Include
 * ===========================================================================*/

#include <avr/interrupt.h>

#include "interfaces/drv_usart.h"
#include "interfaces/tm_macros.h"
#include "interfaces/tm_modules.h"
#include "interfaces/tm_runLevel.h"
#include "define.h" // Get the USART baud rate
#include "tmlibc.h"

/* -----------------------------------------------
 * Circular buffers
 * ---------------------------------------------*/

// Always use a power of two for the buffer size to avoid modulo operations
#define HAL_USART_BUFFER_SIZE 64

_Static_assert((HAL_USART_BUFFER_SIZE & (HAL_USART_BUFFER_SIZE - 1)) == 0,
			   "HAL_USART_BUFFER_SIZE must be a power of two");

_Static_assert((HAL_USART_BUFFER_SIZE <= 256), "HAL_USART_BUFFER_SIZE must be 256 max");

#define CB_MASK (HAL_USART_BUFFER_SIZE - 1)
#define CB_NEXT(index) (((index) + 1) & CB_MASK)
#define CB_FULL(head, tail) (CB_NEXT(head) == (tail))
#define CB_EMPTY(head, tail) ((head) == (tail))

/* -----------------------------------------------
 * Private variables
 * ---------------------------------------------*/

static volatile uint8_t buffer_rx[HAL_USART_BUFFER_SIZE];
static volatile uint8_t buffer_tx[HAL_USART_BUFFER_SIZE];
static volatile uint8_t buffer_rx_head = 0, buffer_rx_tail = 0;
static volatile uint8_t buffer_tx_head = 0, buffer_tx_tail = 0;
static hal_driver_status_t usart_status;
// Shared with the RX ISR; err_codes_t is one byte on the AVR8 build (-fshort-enums).
static volatile err_codes_t usart_last_error = ERR_NO_ERROR;

/* -----------------------------------------------
 * Private function prototypes
 * ---------------------------------------------*/

static err_codes_t usartWriteChar(uint8_t data);
static hal_driver_state_t usartSetError(err_codes_t error);

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

/* -----------------------------------------------
 * Driver life cycle
 * ---------------------------------------------*/

static hal_driver_state_t usartSetError(err_codes_t error)
{
	usart_last_error = error;
	return DRV_STATE_ERROR;
}

static hal_driver_state_t hal_usartGetStatus(void)
{
	if( TM_GETBIT(usart_status, DRV_BIT_DEAD) != 0 )
	{
		usart_last_error = ERR_HAL_DRIVER_DEAD;
		return DRV_STATE_DEAD;
	}
	if( TM_GETBIT(usart_status, DRV_BIT_ERROR) != 0 )
	{
		return usartSetError(ERR_HAL_DRIVER_INVALID_STATE);
	}
	if( TM_GETBIT(usart_status, DRV_BIT_INIT) == 0 )
	{
		if( TM_GETBIT(usart_status, DRV_BIT_START) == 0 ) { return DRV_STATE_OFF; }
		return usartSetError(ERR_HAL_DRIVER_INVALID_STATE);
	}
	if( TM_GETBIT(usart_status, DRV_BIT_START) == 0 ) { return DRV_STATE_INITIALIZED; }
	return DRV_STATE_RUNNING;
}

static hal_driver_state_t usartRequireRunning(void)
{
	hal_driver_state_t state = hal_usartGetStatus();
	if( (state == DRV_STATE_OFF) || (state == DRV_STATE_INITIALIZED) )
	{
		return usartSetError(ERR_HAL_DRIVER_NOT_RUNNING);
	}
	return state;
}

static hal_driver_state_t hal_usartInit(void)
{
	if( TM_GETBIT(usart_status, DRV_BIT_DEAD) != 0 ) { return usartSetError(ERR_HAL_DRIVER_DEAD); }
	uint16_t ubrr = (F_CPU / (16UL * USART_BAUD_RATE)) - 1;

	UBRR1H = (uint8_t)(ubrr >> 8);
	UBRR1L = (uint8_t)ubrr;

	TM_WRITEBIT(UCSR1B, RXEN1, TXEN1); // Enable Rx and Tx
	TM_WRITEBIT(UCSR1C, UCSZ11, UCSZ10); // 8-bit data, 1 stop bit, no parity

	TM_SETBIT(usart_status, DRV_BIT_INIT);
	usart_last_error = ERR_NO_ERROR;
	return DRV_STATE_INITIALIZED;
}

static hal_driver_state_t hal_usartStart(void)
{
	if( TM_GETBIT(usart_status, DRV_BIT_DEAD) != 0 ) { return usartSetError(ERR_HAL_DRIVER_DEAD); }
	if( TM_GETBIT(usart_status, DRV_BIT_INIT) == 0 )
	{
		return usartSetError(ERR_HAL_DRIVER_NOT_INITIALIZED);
	}

	TM_SETBIT(UCSR1B, RXCIE1); // enable Rx interrupt

	TM_SETBIT(usart_status, DRV_BIT_START);
	return DRV_STATE_RUNNING;
}

static hal_driver_state_t hal_usartStop(void)
{
	// nothing to do ?
	TM_CLEARBIT(usart_status, DRV_BIT_START);
	return hal_usartGetStatus();
}

/* -----------------------------------------------
 * Receive path
 * ---------------------------------------------*/

ISR(USART1_RX_vect)
{
	uint8_t next_head = CB_NEXT(buffer_rx_head);
	uint8_t data = UDR1; // Read the received byte

	if( !CB_FULL(buffer_rx_head, buffer_rx_tail) )
	{
		buffer_rx[buffer_rx_head] = data;
		buffer_rx_head = next_head;
	}
	else { usart_last_error = ERR_HAL_USART_RX_BUFFER_FULL; }
}

// Read a character from Rx buffer (non-blocking)
hal_driver_state_t hal_usartRead(uint8_t *data)
{
	hal_driver_state_t state = usartRequireRunning();
	if( state != DRV_STATE_RUNNING ) { return state; }
	if( data == 0 ) { return usartSetError(ERR_NULL_POINTER); }
	if( CB_EMPTY(buffer_rx_head, buffer_rx_tail) )
	{
		return usartSetError(ERR_HAL_USART_RX_BUFFER_EMPTY);
	}

	*data = buffer_rx[buffer_rx_tail];
	buffer_rx_tail = CB_NEXT(buffer_rx_tail);
	return DRV_STATE_RUNNING;
}

/* -----------------------------------------------
 * Transmit path
 * ---------------------------------------------*/

static err_codes_t usartWriteChar(uint8_t data)
{
	uint8_t next_head = CB_NEXT(buffer_tx_head);
	if( CB_FULL(buffer_tx_head, buffer_tx_tail) ) { return ERR_HAL_USART_TX_BUFFER_FULL; }

	buffer_tx[buffer_tx_head] = data;
	buffer_tx_head = next_head;
	return ERR_NO_ERROR;
}

hal_driver_state_t hal_usartWriteChar(uint8_t data)
{
	hal_driver_state_t state = usartRequireRunning();
	if( state != DRV_STATE_RUNNING ) { return state; }
	err_codes_t error = usartWriteChar(data);
	if( error != ERR_NO_ERROR ) { return usartSetError(error); }
	return DRV_STATE_RUNNING;
}

hal_driver_state_t hal_usartSendTXBuffer(void)
{
	hal_driver_state_t state = usartRequireRunning();
	if( state != DRV_STATE_RUNNING ) { return state; }
	while( !CB_EMPTY(buffer_tx_head, buffer_tx_tail) )
	{
		while( !TM_GETBIT(UCSR1A, UDRE1) ); // Wait for empty transmit buffer
		UDR1 = buffer_tx[buffer_tx_tail]; // Put data into buffer, sends the data

		buffer_tx_tail = CB_NEXT(buffer_tx_tail);
	}
	return DRV_STATE_RUNNING;
}

hal_driver_state_t hal_usartWriteString(tm_string_t str)
{
	uint8_t index = 0;

	hal_driver_state_t state = usartRequireRunning();
	if( state != DRV_STATE_RUNNING ) { return state; }
	if( str.text == 0 ) { return usartSetError(ERR_NULL_POINTER); }

	while( index < TM_STRING_SIZE_MAX )
	{
		char str_char = hal_string_getChar(&str, index);
		if( str_char == 0 ) { break; }
		if( usartWriteChar((uint8_t)str_char) == ERR_HAL_USART_TX_BUFFER_FULL )
		{
			return usartSetError(ERR_HAL_USART_TX_BUFFER_FULL);
		};
		index++;
	}
	return DRV_STATE_RUNNING;
}

/* -----------------------------------------------
 * Driver control
 * ---------------------------------------------*/

hal_driver_state_t hal_usartControl(hal_driver_control_t command, hal_driver_control_data_t *data)
{
	switch( command )
	{
		case DRV_CTRL_INIT:
			return hal_usartInit();
		case DRV_CTRL_START:
			return hal_usartStart();
		case DRV_CTRL_STOP:
			return hal_usartStop();
		case DRV_CTRL_RLSET:
			if( data == 0 ) { return usartSetError(ERR_NULL_POINTER); }
			if( data->run_level >= RL_LEVEL_COUNT )
			{
				return usartSetError(ERR_HAL_DRIVER_INVALID_VALUE);
			}
			usart_status &= (hal_driver_status_t)~RL_LEVEL_MASK;
			usart_status |= data->run_level;
			return hal_usartGetStatus();
		case DRV_CTRL_RLGET:
			if( data == 0 ) { return usartSetError(ERR_NULL_POINTER); }
			data->run_level = usart_status & RL_LEVEL_MASK;
			return hal_usartGetStatus();
		case DRV_CTRL_SETBIT:
			if( data == 0 ) { return usartSetError(ERR_NULL_POINTER); }
			if( (data->status_bit < DRV_BIT_INIT) || (data->status_bit > DRV_BIT_DEAD) )
			{
				return usartSetError(ERR_HAL_DRIVER_INVALID_VALUE);
			}
			TM_SETBIT(usart_status, data->status_bit);
			return hal_usartGetStatus();
		case DRV_CTRL_CLEARBIT:
			if( data == 0 ) { return usartSetError(ERR_NULL_POINTER); }
			if( (data->status_bit < DRV_BIT_INIT) || (data->status_bit > DRV_BIT_DEAD) )
			{
				return usartSetError(ERR_HAL_DRIVER_INVALID_VALUE);
			}
			TM_CLEARBIT(usart_status, data->status_bit);
			return hal_usartGetStatus();
		case DRV_CTRL_GETBIT:
			if( data == 0 ) { return usartSetError(ERR_NULL_POINTER); }
			if( (data->status_bit < DRV_BIT_INIT) || (data->status_bit > DRV_BIT_DEAD) )
			{
				return usartSetError(ERR_HAL_DRIVER_INVALID_VALUE);
			}
			data->bit_value = TM_GETBIT(usart_status, data->status_bit) != 0;
			return hal_usartGetStatus();
		case DRV_CTRL_GETSTATUS:
			return hal_usartGetStatus();
		case DRV_CTRL_GETLASTERROR:
			if( data == 0 ) { return usartSetError(ERR_NULL_POINTER); }
			data->error = usart_last_error;
			return hal_usartGetStatus();
		default:
			return usartSetError(ERR_HAL_DRIVER_INVALID_CONTROL);
	}
}
