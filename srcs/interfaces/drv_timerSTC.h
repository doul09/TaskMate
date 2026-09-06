/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file drv_timerSTC.h
 * @brief Generic software time-counter timer driver interface declarations.
 */

#ifndef INTERFACES_DRV_TIMERSTC_H
#define INTERFACES_DRV_TIMERSTC_H

#include "interfaces/tm_modules.h"

typedef void (*hal_timerSTCCallback_t)(void);

hal_driver_state_t hal_timerSTCControl(hal_driver_control_t command,
									   hal_driver_control_data_t *data);
hal_driver_state_t hal_timerSTCSetCallback(hal_timerSTCCallback_t func_ptr);

#endif // INTERFACES_DRV_TIMERSTC_H
