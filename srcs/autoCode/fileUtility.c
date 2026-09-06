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

/* -----------------------------------------------
 * Private variables
 * ---------------------------------------------*/

static int file_updated = 0;
static int file_unchanged = 0;

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

void fileCmpReplace(file_t *file_old, file_t *file_new)
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
		char *old_result = fgets(old, sizeof(old), file_old->stream);
		char *new_result = fgets(new, sizeof(new), file_new->stream);

		if( (old_result == NULL) || (new_result == NULL) )
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
	const size_t name_size = strlen(file_src_name) + sizeof(".tmp");
	file_tmp->name = malloc(name_size);
	if( file_tmp->name == NULL )
	{
		AUTOCODE_MSG_ERROR("from [%s:%i] malloc <%s>", caller, line, file_src_name);
		exit(1);
	}
	file_tmp->name_allocated = true;
	snprintf(file_tmp->name, name_size, "%s.tmp", file_src_name);

	file_tmp->stream = fopen(file_tmp->name, "w+");
	if( file_tmp->stream == NULL )
	{
		AUTOCODE_MSG_ERROR("from [%s:%i] creating file <%s>", caller, line, file_tmp->name);
		exit(1);
	}
	file_tmp->stream_opened = true;
}
