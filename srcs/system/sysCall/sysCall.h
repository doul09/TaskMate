/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file sysCall.h
 * @brief sys call header declarations.
 *
 */

#ifndef SYSCALL_SYSCALL_H
#define SYSCALL_SYSCALL_H

#include <stdbool.h>
#include <stdint.h>

#include "interfaces/drv_rtc.h"
#include "interfaces/error_catalog.h"
#include "interfaces/tm_string.h"

void sc_threadSetSTC(uint16_t count);
uint16_t sc_threadGetSTC(void);

uint16_t sc_threadGetCount(void);
bool sc_threadGetInfo(uint16_t id, const tm_string_t **name, uint8_t *run_level);
bool sc_threadStart(const char *name, uint8_t initial_run_level);
bool sc_threadStop(const char *name);

uint16_t sc_driverGetCount(void);
bool sc_driverGetInfo(uint16_t id, const tm_string_t **name, uint8_t *run_level,
					  uint8_t *status_bits);
bool sc_driverInit(const char *name);
bool sc_driverStart(const char *name);
bool sc_driverStop(const char *name);

err_codes_t sc_lcdClear(void);
err_codes_t sc_lcdSetCursor(uint8_t row, uint8_t col);
err_codes_t sc_lcdWriteString(tm_string_t str);
err_codes_t sc_rtcRead(hal_rtc_time_t *time);

err_codes_t sc_i2cScan(void);
err_codes_t sc_usartRead(uint8_t *data);

void sc_coopYield(void);

#endif // SYSCALL_SYSCALL_H
