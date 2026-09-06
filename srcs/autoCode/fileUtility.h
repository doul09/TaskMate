/*
 * TaskMate Project
 * (c) 2025 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file fileUtility.h
 * @brief file utility header declarations.
 *
 */

#ifndef AUTOCODE_FILEUTILITY_H
#define AUTOCODE_FILEUTILITY_H

/* ============================================================================
 * Includes
 * ========================================================================== */

#include "autoCode.h"

/* ============================================================================
 * Public definitions
 * ========================================================================== */

#define FILE_TOKEN_SIZE 256

#define FILE_READONLY 1
#define FILE_CREATE 2

typedef struct
{
	FILE *stream;
	bool stream_opened; // allow fclose()
	char *name;
	bool name_allocated; // allow free()
	char token[FILE_TOKEN_SIZE];
} file_t;

/* ============================================================================
 * Public API
 * ========================================================================== */

void filePrintModified(void);
void fileCmpReplace(file_t *file_old, file_t *file_new);
void fileClose(file_t *file, const char *caller, int line);
void fileInit(file_t *file);
void fileOpen( file_t *file, const char *mode, int special_mode, const char *caller, int line);
void fileMakeTmp(const char *file_src_name, file_t *file_tmp, const char *caller, int line);

#endif // AUTOCODE_FILEUTILITY_H
