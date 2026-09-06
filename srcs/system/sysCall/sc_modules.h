/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file sc_modules.h
 * @brief Module and system syscall declarations.
 */

#ifndef SYSCALL_SC_MODULES_H
#define SYSCALL_SC_MODULES_H

#include <stdbool.h>
#include <stdint.h>

#include "interfaces/tm_string.h"

void sc_threadSetSTC(uint16_t count);
uint16_t sc_threadGetSTC(void);

uint16_t sc_threadGetCount(void);
bool sc_threadGetInfo(uint16_t id, const tm_string_t **name, uint8_t *run_level);
bool sc_threadStart(const char *name, uint8_t initial_run_level);
bool sc_threadStop(const char *name);

void sc_coopYield(void);

#endif // SYSCALL_SC_MODULES_H
