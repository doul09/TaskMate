/*
 * TaskMate Project
 * (c) 2025 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file fileUtility.c
 * @brief file utility implementation.
 *
 */

/* =============================================================================
 * Declarations - Include
 * ===========================================================================*/

#include "fileUtility.h"

#include <errno.h>
#include <limits.h>

/* -----------------------------------------------
 * Private types
 * ---------------------------------------------*/

typedef struct
{
	char *source_name;
	char *temporary_name;
} file_tmp_item_t;

/* -----------------------------------------------
 * Private variables
 * ---------------------------------------------*/

static int file_updated = 0;
static int file_unchanged = 0;
static file_tmp_item_t *file_tmp_list = NULL;
static size_t file_tmp_source_count = 0;
static bool file_tmp_cleanup_registered = false;

/* -----------------------------------------------
 * Private function prototypes
 * ---------------------------------------------*/

static void fileCmpReplace(file_t *file_old, file_t *file_new);
static void fileTmpCleanup(void);
static char *fileTmpRegister(const char *file_src_name, const char *caller, int line);
static char *fileTmpName(const char *file_src_name, const char *caller, int line);

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

void filePrintModified(void)
{
	AUTOCODE_MSG_INFO("*******************************************************");
	AUTOCODE_MSG_INFO(
		"* summary of modified files : %i updated, %i unchanged *", file_updated, file_unchanged);
	AUTOCODE_MSG_INFO("*******************************************************");
}

void fileCmpReplaceAll(void)
{
	for( size_t i = 0; i < file_tmp_source_count; i++ )
	{
		file_t file_src;
		fileInit(&file_src);
		file_src.name = file_tmp_list[i].source_name;
		fileOpen(&file_src, "r", FILE_READONLY, __FILE__, __LINE__);

		file_t file_tmp;
		fileInit(&file_tmp);
		file_tmp.name = file_tmp_list[i].temporary_name;
		fileOpen(&file_tmp, "r", FILE_READONLY, __FILE__, __LINE__);

		fileCmpReplace(&file_src, &file_tmp);
		fileClose(&file_src, __FILE__, __LINE__);
		fileClose(&file_tmp, __FILE__, __LINE__);
	}

	fileTmpCleanup();
}

static void fileCmpReplace(file_t *file_old, file_t *file_new)
{
	char old[BYTE_INDEX];
	char new[BYTE_INDEX];
	bool same = true;

	if( fseek(file_old->stream, 0L, SEEK_SET) != 0 )
	{
		AUTOCODE_MSG_ERROR("fseek file <%s>", file_old->name);
		exit(1);
	}

	if( fseek(file_new->stream, 0L, SEEK_SET) != 0 )
	{
		AUTOCODE_MSG_ERROR("fseek file <%s>", file_new->name);
		exit(1);
	}

	while( true )
	{
		file_get_line_result_t old_result = fileGetLine(file_old, old, sizeof(old));
		file_get_line_result_t new_result = fileGetLine(file_new, new, sizeof(new));

		if( (old_result == FILE_GET_LINE_ERROR) || (new_result == FILE_GET_LINE_ERROR) )
		{
			AUTOCODE_MSG_ERROR("reading files <%s> and <%s>", file_old->name, file_new->name);
			exit(1);
		}
		if( (old_result == FILE_GET_LINE_EOF) || (new_result == FILE_GET_LINE_EOF) )
		{
			same = (old_result == new_result);
			break;
		}
		if( strcmp(old, new) != 0 )
		{
			same = false;
			break;
		}
	}

	if( same == true )
	{
		AUTOCODE_MSG_INFO("keep the old one <%s>", file_old->name);
		remove(file_new->name);
		file_unchanged++;
	}
	else
	{
		AUTOCODE_MSG_INFO("change for the new one, tmp -> <%s>", file_old->name);
		remove(file_old->name);
		rename(file_new->name, file_old->name);
		file_updated++;
	}
}

file_get_line_result_t fileGetLine(file_t *file, char *line, const size_t line_size)
{
	if( (file == NULL) || (file->stream == NULL) || (line == NULL) || (line_size < 2U) ||
		(line_size > INT_MAX) )
	{
		errno = EINVAL;
		return FILE_GET_LINE_ERROR;
	}

	if( fgets(line, (int)line_size, file->stream) == NULL )
	{
		return feof(file->stream) ? FILE_GET_LINE_EOF : FILE_GET_LINE_ERROR;
	}

	if( strchr(line, '\n') != NULL ) { return FILE_GET_LINE_SUCCESS; }

	/* Distinguish a valid final line that exactly fills the buffer from a truncated line. */
	if( feof(file->stream) ) { return FILE_GET_LINE_SUCCESS; }

	const int next_character = fgetc(file->stream);
	if( (next_character == EOF) && feof(file->stream) ) { return FILE_GET_LINE_SUCCESS; }
	if( next_character == EOF ) { return FILE_GET_LINE_ERROR; }

	errno = EOVERFLOW;
	return FILE_GET_LINE_ERROR;
}

void fileClose(file_t *file, const char *caller, const int line)
{
	if( file->stream_opened )
	{
		int err = fclose(file->stream);
		if( err != 0 )
		{
			AUTOCODE_MSG_ERROR("from [%s:%i] close file <%s>", caller, line, file->name);
			exit(1);
		}
		if( file->name_allocated ) { free(file->name); }
		fileInit(file);
	}
}

void fileInit(file_t *file)
{
	file->name = NULL;
	file->name_allocated = false;
	file->stream = NULL;
	file->stream_opened = false;
}

void fileOpen(file_t *file, const char *mode, const int special_mode, const char *caller,
			  const int line)
{
	if( file->name == NULL )
	{
		AUTOCODE_MSG_ERROR("from [%s:%i] NULL name ", caller, line);
		exit(1);
	}

	file->stream = fopen(file->name, mode);
	if( (file->stream == NULL) && (special_mode == FILE_READONLY) )
	{
		AUTOCODE_MSG_ERROR("from [%s:%i] opening file <%s>", caller, line, file->name);
		exit(1);
	}

	if( (file->stream == NULL) && (special_mode == FILE_CREATE) && (strcmp(mode, "r") == 0) )
	{
		AUTOCODE_MSG_INFO("file don't exist -> creating <%s>", file->name);
		file->stream = fopen(file->name, "w");
		if( file->stream == NULL )
		{
			AUTOCODE_MSG_ERROR("from [%s:%i]creating file <%s>", caller, line, file->name);
			exit(1);
		}
		fclose(file->stream);
		file->stream = fopen(file->name, mode);
		if( file->stream == NULL )
		{
			AUTOCODE_MSG_ERROR("from [%s:%i]reopening file <%s>", caller, line, file->name);
			exit(1);
		}
	}
	file->stream_opened = true;
}

void fileMakeTmp(const char *file_src_name, file_t *file_tmp, const char *caller, const int line)
{
	file_tmp->name = fileTmpRegister(file_src_name, caller, line);

	file_tmp->stream = fopen(file_tmp->name, "w+");
	if( file_tmp->stream == NULL )
	{
		AUTOCODE_MSG_ERROR("from [%s:%i] creating file <%s>", caller, line, file_tmp->name);
		exit(1);
	}
	file_tmp->stream_opened = true;
}

static void fileTmpCleanup(void)
{
	for( size_t i = 0; i < file_tmp_source_count; i++ )
	{
		remove(file_tmp_list[i].temporary_name);
		free(file_tmp_list[i].temporary_name);
		free(file_tmp_list[i].source_name);
	}

	free(file_tmp_list);
	file_tmp_list = NULL;
	file_tmp_source_count = 0;
}

static char *fileTmpRegister(const char *file_src_name, const char *caller, const int line)
{
	if( file_tmp_cleanup_registered == false )
	{
		if( atexit(fileTmpCleanup) != 0 )
		{
			AUTOCODE_MSG_ERROR("from [%s:%i] registering temporary file cleanup", caller, line);
			exit(1);
		}
		file_tmp_cleanup_registered = true;
	}

	const size_t source_name_size = strlen(file_src_name) + 1;
	char *source_name = malloc(source_name_size);
	if( source_name == NULL )
	{
		AUTOCODE_MSG_ERROR("from [%s:%i] malloc <%s>", caller, line, file_src_name);
		exit(1);
	}
	memcpy(source_name, file_src_name, source_name_size);
	char *temporary_name = fileTmpName(file_src_name, caller, line);

	file_tmp_item_t *list = realloc(file_tmp_list, (file_tmp_source_count + 1) * sizeof(*list));
	if( list == NULL )
	{
		free(temporary_name);
		free(source_name);
		AUTOCODE_MSG_ERROR("from [%s:%i] realloc temporary file list", caller, line);
		exit(1);
	}

	file_tmp_list = list;
	file_tmp_list[file_tmp_source_count].source_name = source_name;
	file_tmp_list[file_tmp_source_count].temporary_name = temporary_name;
	file_tmp_source_count++;
	return temporary_name;
}

static char *fileTmpName(const char *file_src_name, const char *caller, const int line)
{
	const size_t name_size = strlen(file_src_name) + sizeof(".tmp");
	char *file_tmp_name = malloc(name_size);
	if( file_tmp_name == NULL )
	{
		AUTOCODE_MSG_ERROR("from [%s:%i] malloc <%s>", caller, line, file_src_name);
		exit(1);
	}
	snprintf(file_tmp_name, name_size, "%s.tmp", file_src_name);
	return file_tmp_name;
}
