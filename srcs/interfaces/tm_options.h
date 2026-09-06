/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file tm_options.h
 * @brief options header declarations.
 *
 */

#ifndef INTERFACES_TM_OPTIONS_H
#define INTERFACES_TM_OPTIONS_H

/* ============================================================================
 * Public definitions
 * ========================================================================== */

#define VERBOSE_LEVEL 0

#define TM_LIBC_TASKMATE 1
#define TM_LIBC_CSTD 0

#if !(TM_LIBC_CSTD ^ TM_LIBC_TASKMATE)
_Static_assert(0, "Select exactly one libc, external or internal");
#endif

#endif // INTERFACES_TM_OPTIONS_H
