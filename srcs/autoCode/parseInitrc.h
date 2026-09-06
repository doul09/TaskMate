/*
 * TaskMate Project
 * (c) 2025 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file parseInitrc.h
 * @brief parse initrc header declarations.
 *
 */

#ifndef AUTOCODE_PARSEINITRC_H
#define AUTOCODE_PARSEINITRC_H

/* ============================================================================
 * Includes
 * ========================================================================== */

#include "autoCode.h"

/* ============================================================================
 * Public API
 * ========================================================================== */

void parseInitrc(modules_database_t *data_base, const char *initrc_name);

#endif // AUTOCODE_PARSEINITRC_H
