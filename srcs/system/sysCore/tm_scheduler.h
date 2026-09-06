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
 * Public API
 * ========================================================================== */

void tm_schedulerInit(void);
void tm_schedulerStart(void);
void tm_schedulerCoop(void);

#endif // SYSCORE_TM_SCHEDULER_H
