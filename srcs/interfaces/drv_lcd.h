/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file drv_lcd.h
 * @brief Generic LCD driver interface declarations.
 */

#ifndef INTERFACES_DRV_LCD_H
#define INTERFACES_DRV_LCD_H

/* ============================================================================
 * Includes
 * ========================================================================== */

#include <stdint.h>

#include "interfaces/tm_modules.h"
#include "interfaces/tm_string.h"

/* ============================================================================
 * Public API
 * ========================================================================== */

hal_driver_state_t hal_lcdControl(hal_driver_control_t command, hal_driver_control_data_t *data);
hal_driver_state_t hal_lcdClear(void);
hal_driver_state_t hal_lcdSetCursor(uint8_t row, uint8_t col);
hal_driver_state_t hal_lcdWriteString(tm_string_t str);

#endif // INTERFACES_DRV_LCD_H
