/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file date.h
 * @brief Date command declarations.
 */

#ifndef SERVICES_COMMANDS_DATE_H
#define SERVICES_COMMANDS_DATE_H

/* ============================================================================
 * Includes
 * ========================================================================== */

#include <stdbool.h>
#include <stdint.h>

/* ============================================================================
 * Public API
 * ========================================================================== */

bool dateCommand(uint8_t argc, char *argv[]);

#endif // SERVICES_COMMANDS_DATE_H
