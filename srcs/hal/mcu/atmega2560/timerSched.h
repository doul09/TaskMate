/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file timerSched.h
 * @brief timer sched header declarations.
 *
 */

#ifndef ATMEGA2560_TIMERSCHED_H
#define ATMEGA2560_TIMERSCHED_H

#include <stdint.h>

#include "hal/arch/avr8/arch_define.h"
#include "interfaces/tm_modules.h"

typedef hal_stack_word_t *hal_timerSchedCallback_func_t(hal_stack_word_t *stack_pointer);
typedef hal_timerSchedCallback_func_t *hal_timerSchedCallback_ptr_t;

hal_driver_state_t hal_timerSchedControl(hal_driver_control_t command,
										 hal_driver_control_data_t *data);
hal_driver_state_t hal_timerSchedSetCallback(hal_timerSchedCallback_ptr_t func_ptr);
hal_driver_state_t hal_timerSchedLoad(void);

#endif // ATMEGA2560_TIMERSCHED_H
