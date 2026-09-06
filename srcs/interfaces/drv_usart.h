/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file drv_usart.h
 * @brief Generic USART driver interface declarations.
 */

#ifndef INTERFACES_DRV_USART_H
#define INTERFACES_DRV_USART_H

#include <stdint.h>

#include "interfaces/tm_modules.h"
#include "interfaces/tm_string.h"

hal_driver_state_t hal_usartControl(hal_driver_control_t command, hal_driver_control_data_t *data);
hal_driver_state_t hal_usartRead(uint8_t *data);
hal_driver_state_t hal_usartWriteChar(uint8_t data);
hal_driver_state_t hal_usartSendTXBuffer(void);
hal_driver_state_t hal_usartWriteString(tm_string_t str);

#endif // INTERFACES_DRV_USART_H
