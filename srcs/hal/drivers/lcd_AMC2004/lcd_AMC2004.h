/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file lcd_AMC2004.h
 * @brief lcd amc2004 header declarations.
 *
 */

#ifndef LCD_AMC2004_LCD_AMC2004_H
#define LCD_AMC2004_LCD_AMC2004_H

#include <stdint.h>

#include "interfaces/tm_modules.h"
#include "interfaces/tm_string.h"

hal_driver_state_t hal_lcdControl(hal_driver_control_t command, hal_driver_control_data_t *data);
hal_driver_state_t hal_lcdClear(void);
hal_driver_state_t hal_lcdSetCursor(uint8_t row, uint8_t col);
hal_driver_state_t hal_lcdWriteString(tm_string_t str);

#endif // LCD_AMC2004_LCD_AMC2004_H
