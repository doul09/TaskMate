/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file tm_scheduler.h
 * @brief tm scheduler header declarations.
 *
 */

#ifndef SYSCORE_TM_SCHEDULER_H
#define SYSCORE_TM_SCHEDULER_H

/* ============================================================================
 * Includes
 * ========================================================================== */

#include <stdbool.h>
#include <stdint.h>

/* ============================================================================
 * Public API
 * ========================================================================== */

void tm_schedulerInit(void);
void tm_schedulerStart(void);
void tm_schedulerCoop(void);
bool tm_schedulerRunLevelSet(uint8_t run_level);
uint8_t tm_schedulerRunLevelGet(void);

#endif // SYSCORE_TM_SCHEDULER_H
