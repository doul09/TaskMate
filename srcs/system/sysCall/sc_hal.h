/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file sc_hal.h
 * @brief Driver and HAL syscall declarations.
 */

#ifndef SYSCALL_SC_HAL_H
#define SYSCALL_SC_HAL_H

/* ============================================================================
 * Includes
 * ========================================================================== */

#include <stdbool.h>
#include <stdint.h>

#include "interfaces/drv_rtc.h"
#include "interfaces/error_catalog.h"
#include "interfaces/tm_string.h"

/* ============================================================================
 * Public API
 * ========================================================================== */

/* -----------------------------------------------
 * Driver life cycle
 * ---------------------------------------------*/

uint16_t sc_driverGetCount(void);
bool sc_driverGetInfo(uint16_t id, const tm_string_t **name, uint8_t *run_level,
					  uint8_t *status_bits);
bool sc_driverInit(const char *name);
bool sc_driverStart(const char *name);
bool sc_driverStop(const char *name);

/* -----------------------------------------------
 * LCD operations
 * ---------------------------------------------*/

err_codes_t sc_lcdClear(void);
err_codes_t sc_lcdWriteString(tm_string_t str, uint8_t row, uint8_t col);

/* -----------------------------------------------
 * RTC operations
 * ---------------------------------------------*/

err_codes_t sc_rtcRead(hal_rtc_time_t *time);
err_codes_t sc_rtcWrite(const hal_rtc_time_t *time);
err_codes_t sc_rtcSaveStartupTime(void);

/* -----------------------------------------------
 * I2C operations
 * ---------------------------------------------*/

err_codes_t sc_i2cScan(void);

/* -----------------------------------------------
 * USART operations
 * ---------------------------------------------*/

err_codes_t sc_usartRead(uint8_t *data);

#endif // SYSCALL_SC_HAL_H
