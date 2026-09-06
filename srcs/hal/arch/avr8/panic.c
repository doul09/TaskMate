/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file panic.c
 * @brief panic implementation
 *
 */

#include "panic.h"

#include "hal/arch/avr8/interrupt.h"
#include "hal/public/tmlibc.h"
#include "interfaces/drv_usart.h"

void panic(tm_string_t msg)
{
	hal_clearGlobalInterrupt();
	hal_usartSendTXBuffer();
	hal_usartWriteString(TM_STR("\nSYSTEM PANIC : "));
	hal_usartSendTXBuffer();
	hal_usartWriteString(msg);
	hal_usartSendTXBuffer();
	hal_usartWriteString(TM_STR(", halt."));
	hal_usartSendTXBuffer();
	while( 1 );
}

// the end, really.
