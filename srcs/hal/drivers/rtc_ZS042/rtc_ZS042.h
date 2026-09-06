/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file rtc_ZS042.h
 * @brief rtc zs042 header declarations.
 *
 */

#ifndef RTC_ZS042_RTC_ZS042_H
#define RTC_ZS042_RTC_ZS042_H

#include <stdint.h>

#include "interfaces/tm_modules.h"

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

hal_driver_state_t hal_rtcControl(hal_driver_control_t command, hal_driver_control_data_t *data);
hal_driver_state_t hal_rtcRead(hal_rtc_time_t *time);
hal_driver_state_t hal_rtcWrite(const hal_rtc_time_t *time);

#endif // RTC_ZS042_RTC_ZS042_H
