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

#define FILE_READONLY 1
#define FILE_CREATE 2

typedef struct
{
	FILE *stream;
	bool stream_opened; // allow fclose()
	char *name;
	bool name_allocated; // allow free()
} file_t;

typedef enum
{
	FILE_GET_LINE_ERROR = -1,
	FILE_GET_LINE_EOF = 0,
	FILE_GET_LINE_SUCCESS = 1
} file_get_line_result_t;

/* ============================================================================
 * Public API
 * ========================================================================== */

void filePrintModified(void);
int fileCmpReplaceAll(void);
int fileClose(file_t *file, const char *caller, int line);
file_get_line_result_t fileGetLine(file_t *file, char *line, size_t line_size);
void fileInit(file_t *file);
int fileOpen(file_t *file, const char *mode, int special_mode, const char *caller, int line);
int fileMakeTmp(const char *file_src_name, file_t *file_tmp, const char *caller, int line);

#endif // AUTOCODE_FILEUTILITY_H
