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

static int fileCmpReplace(file_t *file_old, file_t *file_new);
static void fileTmpCleanup(void);
static int fileTmpCleanupAll(void);
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

int fileCmpReplaceAll(void)
{
	int result = 0;

	for( size_t i = 0; i < file_tmp_source_count; i++ )
	{
		file_t file_src;
		fileInit(&file_src);
		file_src.name = file_tmp_list[i].source_name;
		if( fileOpen(&file_src, "r", FILE_READONLY, __FILE__, __LINE__) != 0 )
		{
			result = -1;
			break;
		}

		file_t file_tmp;
		fileInit(&file_tmp);
		file_tmp.name = file_tmp_list[i].temporary_name;
		if( fileOpen(&file_tmp, "r", FILE_READONLY, __FILE__, __LINE__) != 0 )
		{
			result = -1;
			(void)fileClose(&file_src, __FILE__, __LINE__);
			break;
		}

		if( fileCmpReplace(&file_src, &file_tmp) != 0 ) { result = -1; }
		if( fileClose(&file_src, __FILE__, __LINE__) != 0 ) { result = -1; }
		if( fileClose(&file_tmp, __FILE__, __LINE__) != 0 ) { result = -1; }
		if( result != 0 ) { break; }
	}

	if( fileTmpCleanupAll() != 0 )
	{
		autoCodeExit(AC_INCREMENT);
		result = -1;
	}
	return result;
}

static int fileCmpReplace(file_t *file_old, file_t *file_new)
{
	char old[AC_BUFFER_SIZE];
	char new[AC_BUFFER_SIZE];
	bool same = true;

	if( fseek(file_old->stream, 0L, SEEK_SET) != 0 )
	{
		AUTOCODE_MSG_ERROR("fseek file <%s>", file_old->name);
		return -1;
	}

	if( fseek(file_new->stream, 0L, SEEK_SET) != 0 )
	{
		AUTOCODE_MSG_ERROR("fseek file <%s>", file_new->name);
		return -1;
	}

	while( true )
	{
		file_get_line_result_t old_result = fileGetLine(file_old, old, sizeof(old));
		file_get_line_result_t new_result = fileGetLine(file_new, new, sizeof(new));

		if( (old_result == FILE_GET_LINE_ERROR) || (new_result == FILE_GET_LINE_ERROR) )
		{
			AUTOCODE_MSG_ERROR("reading files <%s> and <%s>", file_old->name, file_new->name);
			return -1;
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
		if( remove(file_new->name) != 0 )
		{
			AUTOCODE_MSG_ERROR("removing temporary file <%s>", file_new->name);
			return -1;
		}
		file_unchanged++;
	}
	else
	{
		AUTOCODE_MSG_INFO("change for the new one, tmp -> <%s>", file_old->name);
		if( rename(file_new->name, file_old->name) != 0 )
		{
			AUTOCODE_MSG_ERROR("renaming file <%s> to <%s>", file_new->name, file_old->name);
			return -1;
		}
		file_updated++;
	}
	return 0;
}

file_get_line_result_t fileGetLine(file_t *file, char *line, const size_t line_size)
{
	if( (file == NULL) || (file->stream == NULL) || (line == NULL) || (line_size < 2U) ||
		((size_t)(int)line_size != line_size) )
	{
		errno = EINVAL;
		return FILE_GET_LINE_ERROR;
	}

	if( fgets(line, (int)line_size, file->stream) == NULL )
	{
		return feof(file->stream) ? FILE_GET_LINE_EOF : FILE_GET_LINE_ERROR;
	}

	const size_t line_length = strlen(line);
	if( line_length == 0U )
	{
		errno = EILSEQ;
		return FILE_GET_LINE_ERROR;
	}

	const size_t index = line_length - 1U;
	if( autoCodeBufferIndexIsValid(index, line_size) == false )
	{
		errno = EOVERFLOW;
		return FILE_GET_LINE_ERROR;
	}

	if( line[index] == '\n' ) { return FILE_GET_LINE_SUCCESS; }

	/* Distinguish a valid final line that exactly fills the buffer from a truncated line. */
	if( feof(file->stream) ) { return FILE_GET_LINE_SUCCESS; }

	const int next_character = fgetc(file->stream);
	if( (next_character == EOF) && feof(file->stream) ) { return FILE_GET_LINE_SUCCESS; }
	if( next_character == EOF ) { return FILE_GET_LINE_ERROR; }

	errno = EOVERFLOW;
	return FILE_GET_LINE_ERROR;
}

int fileClose(file_t *file, const char *caller, const int line)
{
	int result = 0;

	if( file->stream_opened )
	{
		if( file->write_access && (ferror(file->stream) != 0) )
		{
			AUTOCODE_MSG_ERROR("from [%s:%i] stream error for file <%s>", caller, line, file->name);
			result = -1;
		}
		int err = fclose(file->stream);
		if( err != 0 )
		{
			AUTOCODE_MSG_ERROR("from [%s:%i] close file <%s>", caller, line, file->name);
			result = -1;
		}
		if( file->name_allocated ) { free(file->name); }
		fileInit(file);
	}
	return result;
}

void fileInit(file_t *file)
{
	file->name = NULL;
	file->name_allocated = false;
	file->stream = NULL;
	file->stream_opened = false;
	file->write_access = false;
}

int fileOpen(file_t *file, const char *mode, const int special_mode, const char *caller,
			 const int line)
{
	if( file->name == NULL )
	{
		AUTOCODE_MSG_ERROR("from [%s:%i] NULL name ", caller, line);
		return -1;
	}

	file->stream = fopen(file->name, mode);
	if( (file->stream == NULL) && (special_mode == FILE_READONLY) )
	{
		AUTOCODE_MSG_ERROR("from [%s:%i] opening file <%s>", caller, line, file->name);
		return -1;
	}

	if( (file->stream == NULL) && (special_mode == FILE_CREATE) && (strcmp(mode, "r") == 0) )
	{
		AUTOCODE_MSG_INFO("file don't exist -> creating <%s>", file->name);
		file->stream = fopen(file->name, "w");
		if( file->stream == NULL )
		{
			AUTOCODE_MSG_ERROR("from [%s:%i]creating file <%s>", caller, line, file->name);
			return -1;
		}
		if( fclose(file->stream) != 0 )
		{
			file->stream = NULL;
			AUTOCODE_MSG_ERROR("from [%s:%i] closing created file <%s>", caller, line, file->name);
			return -1;
		}
		file->stream = fopen(file->name, mode);
		if( file->stream == NULL )
		{
			AUTOCODE_MSG_ERROR("from [%s:%i]reopening file <%s>", caller, line, file->name);
			return -1;
		}
	}
	if( file->stream == NULL )
	{
		AUTOCODE_MSG_ERROR("from [%s:%i] opening file <%s>", caller, line, file->name);
		return -1;
	}
	file->stream_opened = true;
	file->write_access =
		(strchr(mode, 'w') != NULL) || (strchr(mode, 'a') != NULL) || (strchr(mode, '+') != NULL);
	return 0;
}

int fileMakeTmp(const char *file_src_name, file_t *file_tmp, const char *caller, const int line)
{
	file_tmp->name = fileTmpRegister(file_src_name, caller, line);
	if( file_tmp->name == NULL ) { return -1; }

	file_tmp->stream = fopen(file_tmp->name, "w+");
	if( file_tmp->stream == NULL )
	{
		AUTOCODE_MSG_ERROR("from [%s:%i] creating file <%s>", caller, line, file_tmp->name);
		return -1;
	}
	file_tmp->stream_opened = true;
	file_tmp->write_access = true;
	return 0;
}

static void fileTmpCleanup(void) { (void)fileTmpCleanupAll(); }

static int fileTmpCleanupAll(void)
{
	int result = 0;

	for( size_t i = 0; i < file_tmp_source_count; i++ )
	{
		if( (remove(file_tmp_list[i].temporary_name) != 0) && (errno != ENOENT) )
		{
			const int remove_error = errno;
			fprintf(stderr,
					"[fileUtility.c] error : removing temporary file <%s>: %s\n",
					file_tmp_list[i].temporary_name,
					strerror(remove_error));
			result = -1;
		}
		free(file_tmp_list[i].temporary_name);
		free(file_tmp_list[i].source_name);
	}

	free(file_tmp_list);
	file_tmp_list = NULL;
	file_tmp_source_count = 0;
	return result;
}

static char *fileTmpRegister(const char *file_src_name, const char *caller, const int line)
{
	if( file_tmp_cleanup_registered == false )
	{
		if( atexit(fileTmpCleanup) != 0 )
		{
			AUTOCODE_MSG_ERROR("from [%s:%i] registering temporary file cleanup", caller, line);
			return NULL;
		}
		file_tmp_cleanup_registered = true;
	}

	const size_t source_name_size = strlen(file_src_name) + 1;
	char *source_name = malloc(source_name_size);
	if( source_name == NULL )
	{
		AUTOCODE_MSG_ERROR("from [%s:%i] malloc <%s>", caller, line, file_src_name);
		return NULL;
	}
	memcpy(source_name, file_src_name, source_name_size);
	char *temporary_name = fileTmpName(file_src_name, caller, line);
	if( temporary_name == NULL )
	{
		free(source_name);
		return NULL;
	}

	file_tmp_item_t *list = realloc(file_tmp_list, (file_tmp_source_count + 1) * sizeof(*list));
	if( list == NULL )
	{
		free(temporary_name);
		free(source_name);
		AUTOCODE_MSG_ERROR("from [%s:%i] realloc temporary file list", caller, line);
		return NULL;
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
		return NULL;
	}
	snprintf(file_tmp_name, name_size, "%s.tmp", file_src_name);
	return file_tmp_name;
}
