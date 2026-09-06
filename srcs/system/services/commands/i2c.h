/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file i2c.h
 * @brief I2C command declarations.
 */

#ifndef SERVICES_COMMANDS_I2C_H
#define SERVICES_COMMANDS_I2C_H

/* ============================================================================
 * Includes
 * ========================================================================== */

#include <stdbool.h>
#include <stdint.h>

/* ============================================================================
 * Public API
 * ========================================================================== */

bool i2c(uint8_t argc, char *argv[]);

#endif // SERVICES_COMMANDS_I2C_H
