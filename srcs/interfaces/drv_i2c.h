/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file drv_i2c.h
 * @brief Generic I2C driver interface declarations.
 */

#ifndef INTERFACES_DRV_I2C_H
#define INTERFACES_DRV_I2C_H

/* ============================================================================
 * Includes
 * ========================================================================== */

#include <stdint.h>

#include "interfaces/tm_modules.h"

/* ============================================================================
 * Public definitions
 * ========================================================================== */

typedef enum
{
	HAL_I2C_WRITE,
	HAL_I2C_READ
} hal_i2c_direction_t;

typedef enum
{
	HAL_I2C_NACK,
	HAL_I2C_ACK
} hal_i2c_ack_t;

/* ============================================================================
 * Public API
 * ========================================================================== */

hal_driver_state_t hal_i2cControl(hal_driver_control_t command, hal_driver_control_data_t *data);
hal_driver_state_t hal_i2cScan(uint8_t *address);
hal_driver_state_t hal_i2cCommStart(uint8_t address, hal_i2c_direction_t direction);
hal_driver_state_t hal_i2cCommStop(void);
hal_driver_state_t hal_i2cWrite(uint8_t data);
hal_driver_state_t hal_i2cRead(uint8_t *data, hal_i2c_ack_t ack);

#endif // INTERFACES_DRV_I2C_H
