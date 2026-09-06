/*
 * TaskMate Project
 * (c) 2025 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file options.c
 * @brief options implementation.
 *
 */

/* =============================================================================
 * Declarations - Include
 * ===========================================================================*/

#include "options.h"

#include "fileUtility.h"
#include "tokenizer.h"

/* -----------------------------------------------
 * Private function prototypes
 * ---------------------------------------------*/

static void setErrorsFile(const char *value, options_list_t *opt);
static void setInitrcFile(const char *value, options_list_t *opt);
static void setParseTagFile(const char *value, options_list_t *opt);
static void setHalInitFile(const char *value, options_list_t *opt);
static void setHalDefineFile(const char *value, options_list_t *opt);
static void setGpioSignalsFile(const char *value, options_list_t *opt);

/* -----------------------------------------------
 * Option dispatch table
 * ---------------------------------------------*/

#define HAVE_OPTIONS(X)                                \
	X(HAVE_ERRORS, "--errors", setErrorsFile)          \
	X(HAVE_INITRC, "--initrc", setInitrcFile)          \
	X(HAVE_PARSETAG, "--parsetag", setParseTagFile)    \
	X(HAVE_HALINIT, "--halinit", setHalInitFile)       \
	X(HAVE_HALDEFINE, "--haldefine", setHalDefineFile) \
	X(HAVE_GPIO_SIGNALS, "--gpio_signals", setGpioSignalsFile)

static const struct
{
	const char *name;
	void (*func)(const char *value, options_list_t *opt);
} options_cmds[] = {
#define X(e, s, f) {(s), (f)},
	HAVE_OPTIONS(X)
#undef X
		{NULL, NULL}};

enum
{
#define X(e, s, f) e,
	HAVE_OPTIONS(X)
#undef X
		HAVE_COUNT
};

static const char *have_to_string[HAVE_COUNT] = {
#define X(e, s, f) [e] = (s),
	HAVE_OPTIONS(X)
#undef X
};

static int have_options_count[HAVE_COUNT];

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

static void setFileName(char *destination, const size_t destination_size, const char *value)
{
	const size_t value_length = strlen(value);

	if( value_length >= destination_size )
	{
		AUTOCODE_MSG_ERROR("option value is too long (maximum %zu characters)",
						   destination_size - 1U);
		exit(1);
	}

	memcpy(destination, value, value_length + 1U);
}

static const char *string_from_have(const int id)
{
#define X(e, s, f) \
	if( id == (e) ) { return have_to_string[e]; }
	HAVE_OPTIONS(X)
#undef X
	return NULL;
}

static void setErrorsFile(const char *value, options_list_t *opt)
{
	setFileName(opt->file_errors_list, sizeof(opt->file_errors_list), value);
	have_options_count[HAVE_ERRORS]++;
}

static void setInitrcFile(const char *value, options_list_t *opt)
{
	setFileName(opt->file_initrc_list, sizeof(opt->file_initrc_list), value);
	have_options_count[HAVE_INITRC]++;
}

static void setParseTagFile(const char *value, options_list_t *opt)
{
	setFileName(opt->file_parsetag_list, sizeof(opt->file_parsetag_list), value);
	have_options_count[HAVE_PARSETAG]++;
}

static void setHalInitFile(const char *value, options_list_t *opt)
{
	setFileName(opt->file_halinit_list, sizeof(opt->file_halinit_list), value);
	have_options_count[HAVE_HALINIT]++;
}

static void setHalDefineFile(const char *value, options_list_t *opt)
{
	setFileName(opt->file_haldefine_list, sizeof(opt->file_haldefine_list), value);
	have_options_count[HAVE_HALDEFINE]++;
}

static void setGpioSignalsFile(const char *value, options_list_t *opt)
{
	setFileName(opt->file_gpio_signals, sizeof(opt->file_gpio_signals), value);
	have_options_count[HAVE_GPIO_SIGNALS]++;
}

static int optionCmdDispatch(const char *cmd, const char *value, options_list_t *opt)
{
	for( int i = 0; options_cmds[i].name != NULL; i++ )
	{
		if( strcmp(cmd, options_cmds[i].name) == 0 )
		{
			(*options_cmds[i].func)(value, opt);
			return 0;
		}
	}
	return -1;
}

void options(const char *file_name, options_list_t *opt)
{
	// Initialise required options
	for( int i = 0; i < HAVE_COUNT; i++ ) { have_options_count[i] = 0; }

	// Process options from files
	file_t file;
	fileInit(&file);
	file.name = (char *)file_name;
	fileOpen(&file, "r", FILE_READONLY, __FILE__, __LINE__);

	int file_line_number = 0;
	tokenizer_t tok = {0};
	AUTOCODE_MSG_INFO("read file %s", file_name);
	while( fgets(tok.line, TOKEN_LINE_SIZE_MAX, file.stream) )
	{
		file_line_number++;
		tokenizer(&tok);

		if( (tok.count != 0) && (tok.tokens[0][0] != '#') )
		{
			if( tok.count == 2 )
			{
				AUTOCODE_MSG_INFO("parsing %s = %s", tok.tokens[0], tok.tokens[1]);

				int err = optionCmdDispatch(tok.tokens[0], tok.tokens[1], opt);

				if( err != 0 )
				{
					AUTOCODE_MSG_ERROR(
						"unknown option [%s:%i] %s\n", file.name, file_line_number, tok.tokens[0]);
					exit(1);
				}
			}
			else
			{
				AUTOCODE_MSG_ERROR("wrong token count [%s:%i] is %i, should be 2",
								   file.name,
								   file_line_number,
								   tok.count);
				exit(1);
			}
		}
	}
	tokenizerFree(&tok);

	// Test required options
	for( int i = 0; i < HAVE_COUNT; i++ )
	{
		if( have_options_count[i] == 0 )
		{
			AUTOCODE_MSG_ERROR("required autoCode option %s is not set", string_from_have(i));
			exit(1);
		}

		if( have_options_count[i] > 1 )
		{
			AUTOCODE_MSG_ERROR("required autoCode option %s is multiple set", string_from_have(i));
			exit(1);
		}
	}
}
