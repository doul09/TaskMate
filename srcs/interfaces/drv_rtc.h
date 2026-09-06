/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file drv_rtc.h
 * @brief Generic real-time clock driver interface declarations.
 */

#ifndef INTERFACES_DRV_RTC_H
#define INTERFACES_DRV_RTC_H

/* ============================================================================
 * Includes
 * ========================================================================== */

#include <stdint.h>

#include "interfaces/tm_modules.h"

/* ============================================================================
 * Public definitions
 * ========================================================================== */

typedef struct
{
	uint8_t seconds; // 0-59
	uint8_t minutes; // 0-59
	uint8_t hours; // 0-23
	uint8_t weekday; // 1-7
	uint8_t day; // 1-31
	uint8_t month; // 1-12
	uint8_t year; // 0-99
} hal_rtc_time_t;

/* ============================================================================
 * Public API
 * ========================================================================== */

hal_driver_state_t hal_rtcControl(hal_driver_control_t command, hal_driver_control_data_t *data);
hal_driver_state_t hal_rtcRead(hal_rtc_time_t *time);
hal_driver_state_t hal_rtcWrite(const hal_rtc_time_t *time);

#endif // INTERFACES_DRV_RTC_H
