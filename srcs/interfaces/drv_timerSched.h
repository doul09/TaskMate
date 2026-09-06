/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file drv_timerSched.h
 * @brief Generic scheduler timer driver interface declarations.
 */

#ifndef INTERFACES_DRV_TIMERSCHED_H
#define INTERFACES_DRV_TIMERSCHED_H

/* ============================================================================
 * Includes
 * ========================================================================== */

#include "interfaces/tm_modules.h"

/* ============================================================================
 * Public definitions
 * ========================================================================== */

/* The scheduler context is opaque outside the architecture-specific implementation. */
typedef void *hal_timerSchedCallback_func_t(void *context);
typedef hal_timerSchedCallback_func_t *hal_timerSchedCallback_ptr_t;

/* ============================================================================
 * Public API
 * ========================================================================== */

hal_driver_state_t hal_timerSchedControl(hal_driver_control_t command,
										 hal_driver_control_data_t *data);
hal_driver_state_t hal_timerSchedSetCallback(hal_timerSchedCallback_ptr_t func_ptr);
hal_driver_state_t hal_timerSchedLoad(void);

#endif // INTERFACES_DRV_TIMERSCHED_H
