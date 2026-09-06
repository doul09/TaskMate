/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file tm_runLevel.h
 * @brief run level define header declarations.
 */

#ifndef INTERFACES_TM_RUNLEVEL_H
#define INTERFACES_TM_RUNLEVEL_H

/* ============================================================================
 * Public definitions
 * ========================================================================== */

#define RL_RUN_NONE 0
#define RL_RUN_CORE 1
#define RL_RUN_DRIVER 2
#define RL_RUN_SERVICE 3
#define RL_RUN_USER 4

#define RL_LEVEL_MASK 0x07
#define RL_LEVEL_COUNT 5

#define RL_GET_RUN_LEVEL(status) ((status) & RL_LEVEL_MASK)

#endif // INTERFACES_TM_RUNLEVEL_H
