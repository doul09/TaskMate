/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file hal_halt.h
 * @brief Hardware-independent halt contract.
 */

#ifndef INTERFACES_HAL_HALT_H
#define INTERFACES_HAL_HALT_H

/* ============================================================================
 * Public API
 * ========================================================================== */

/**
 * Stop execution permanently.
 *
 * The selected HAL implementation must leave the target in a deterministic state and must not
 * depend on scheduling, allocation, logging, or interrupts.
 */
_Noreturn void hal_halt(void);

#endif // INTERFACES_HAL_HALT_H
