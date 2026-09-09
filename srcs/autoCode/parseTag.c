/*
 * TaskMate Project
 * (c) 2025 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file parseTag.c
 * @brief parse tag implementation.
 *
 */

/* =============================================================================
 * Declarations - Include
 * ===========================================================================*/

#include "parseTag.h"

#include <stdbool.h>

#include "fileUtility.h"
#include "tokenizer.h"

/* -----------------------------------------------
 * Private types
 * ---------------------------------------------*/

typedef struct
{
	const modules_database_t *data_base;
	FILE *file;
	const error_catalog_t *errors;
	const options_list_t *auto_options;
} parse_tag_t;

/* -----------------------------------------------
 * Private function prototypes
 * ---------------------------------------------*/

static void writeModulesCount(const parse_tag_t *parse);
static void writeDriversAlloc(const parse_tag_t *parse);
static void writeThreadsAlloc(const parse_tag_t *parse);
static const char *errorLevelName(err_level_t level);
static void writeErrorCatalog(const parse_tag_t *parse);
static void writeErrorEnum(const parse_tag_t *parse);
static void writeModulesList(const parse_tag_t *parse);
static void writeHalDefine(const parse_tag_t *parse);
static void writeHalInit(const parse_tag_t *parse);
static void writeGpioSignals(const parse_tag_t *parse);

/* -----------------------------------------------
 * Tag dispatch table
 * ---------------------------------------------*/

#define HAVE_TAG(X)                                           \
	X(HAVE_THREADS_ALLOC, "threads_alloc", writeThreadsAlloc) \
	X(HAVE_DRIVERS_ALLOC, "drivers_alloc", writeDriversAlloc) \
	X(HAVE_ERROR_ENUM, "error_enum", writeErrorEnum)          \
	X(HAVE_ERROR_CATALOG, "error_catalog", writeErrorCatalog) \
	X(HAVE_HAL_DEFINE, "hal_define", writeHalDefine)          \
	X(HAVE_HAL_INIT, "hal_init", writeHalInit)                \
	X(HAVE_MOD_COUNT, "modules_count", writeModulesCount)     \
	X(HAVE_MOD_LIST, "modules_list", writeModulesList)        \
	X(HAVE_GPIO_SIGNALS, "gpio_signals", writeGpioSignals)

static const struct
{
	const char *tag;
	void (*func)(const parse_tag_t *parse);
} tags_cmds[] = {
#define X(e, s, f) {(s), (f)},
	HAVE_TAG(X)
#undef X
		{NULL, NULL}};

enum
{
#define X(e, s, f) e,
	HAVE_TAG(X)
#undef X
		HAVE_COUNT
};

static const char *have_to_string[HAVE_COUNT] = {
#define X(e, s, f) [e] = (s),
	HAVE_TAG(X)
#undef X
};

static int have_tag_count[HAVE_COUNT];

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

/* -----------------------------------------------
 * Tag parsing and validation
 * ---------------------------------------------*/

static const char *string_from_have(const int id)
{
#define X(e, s, f) \
	if( id == (e) ) { return have_to_string[e]; }
	HAVE_TAG(X)
#undef X
	return NULL;
}

static int tagCmdDispatch(const char *cmd, const parse_tag_t *parse)
{
	for( int i = 0; tags_cmds[i].tag != NULL; i++ )
	{
		if( strcmp(cmd, tags_cmds[i].tag) == 0 )
		{
			(*tags_cmds[i].func)(parse);
			return 0;
		}
	}
	return -1;
}

void parseTagInit(void)
{
	// Initialise required options
	for( int i = 0; i < HAVE_COUNT; i++ ) { have_tag_count[i] = 0; }
}

void parseTag(modules_database_t *data_base, const char *file_name, const error_catalog_t *errors,
			  const options_list_t *auto_options)
{
	// Open source and temporary files
	AUTOCODE_MSG_INFO("open <%s>", file_name);

	file_t file_src;
	fileInit(&file_src);
	file_src.name = (char *)file_name;
	if( fileOpen(&file_src, "r", FILE_READONLY, __FILE__, __LINE__) != 0 ) { return; }

	file_t file_tmp;
	fileInit(&file_tmp);
	if( fileMakeTmp(file_src.name, &file_tmp, __FILE__, __LINE__) != 0 )
	{
		(void)fileClose(&file_src, __FILE__, __LINE__);
		return;
	}

	parse_tag_t parse = {.data_base = data_base,
						 .file = file_tmp.stream,
						 .errors = errors,
						 .auto_options = auto_options};

	// Read from source
	int tag_section = 0;
	int file_line_number = 0;
	tokenizer_t tok = {0};
	char line[TOKEN_LINE_SIZE_MAX];
	file_get_line_result_t line_result;

	while( (line_result = fileGetLine(&file_src, tok.line, sizeof(tok.line))) ==
		   FILE_GET_LINE_SUCCESS )
	{
		file_line_number++;
		snprintf(line, sizeof(line), "%s", tok.line);
		if( tokenizer(&tok) != 0 ) { continue; }

		if( (tok.count >= 2) && !(strcmp(tok.tokens[0], "//")) &&
			!(strcmp(tok.tokens[1], "[autoCode_tag]")) )
		{
			if( tok.count != 3 )
			{
				AUTOCODE_MSG_ERROR(
					"token count != 3 tok.line [%s:%i] %s", file_src.name, file_line_number, line);
				break;
			}

			if( tag_section == 1 )
			{
				AUTOCODE_MSG_ERROR(
					"Start new tag section without previous end tag [/tag] [%s:%i] %s",
					file_src.name,
					file_line_number,
					line);
				break;
			}

			AUTOCODE_MSG_INFO("found tag %s", tok.tokens[2]);

			fprintf(file_tmp.stream, "%s", line);

			fprintf(file_tmp.stream, "// clang-format off\n");

			fprintf(file_tmp.stream, "/*\n");
			fprintf(file_tmp.stream, " * do not edit code between tag\n");
			fprintf(file_tmp.stream, " * code generated by autoCode, any change will be lost\n");
			fprintf(file_tmp.stream, " */\n\n");

			tag_section = 1;

			int err = tagCmdDispatch(tok.tokens[2], &parse);

			if( err != 0 )
			{
				AUTOCODE_MSG_ERROR(
					"unknown tag [%s:%i] %s\n", file_name, file_line_number, tok.tokens[2]);
			}
		}

		if( (tok.count >= 2) && !(strcmp(tok.tokens[0], "//")) &&
			!(strcmp(tok.tokens[1], "[/tag]")) )
		{
			fprintf(file_tmp.stream, "\n// clang-format on\n");
			AUTOCODE_MSG_INFO("end tag");
			tag_section = 0;
		}

		if( tag_section == 0 ) { fprintf(file_tmp.stream, "%s", line); }
	}
	if( line_result == FILE_GET_LINE_ERROR )
	{
		AUTOCODE_MSG_ERROR("reading file <%s> after line %i", file_src.name, file_line_number);
	}
	tokenizerFree(&tok);

	if( tag_section == 1 )
	{
		AUTOCODE_MSG_ERROR("missing end tag [/tag] [%s:%i]", file_src.name, file_line_number);
	}
	fileClose(&file_src, __FILE__, __LINE__);
	fileClose(&file_tmp, __FILE__, __LINE__);
}

void parseTagHave(void)
{
	// Test required options
	for( int i = 0; i < HAVE_COUNT; i++ )
	{
		if( have_tag_count[i] == 0 )
		{
			AUTOCODE_MSG_ERROR("required autoCode tag %s is not set", string_from_have(i));
		}

		if( have_tag_count[i] > 1 )
		{
			AUTOCODE_MSG_ERROR("required autoCode tag %s is multiple set", string_from_have(i));
		}
	}
}

/* -----------------------------------------------
 * Code generation
 * ---------------------------------------------*/

static void writeGpioSignals(const parse_tag_t *parse)
{
	file_t file_signals;
	fileInit(&file_signals);
	file_signals.name = (char *)parse->auto_options->file_gpio_signals;
	if( fileOpen(&file_signals, "r", FILE_READONLY, __FILE__, __LINE__) != 0 ) { return; }

	fprintf(parse->file, "typedef enum\n");
	fprintf(parse->file, "{\n");

	tokenizer_t tok = {0};
	int line = 0;
	file_get_line_result_t line_result;
	while( (line_result = fileGetLine(&file_signals, tok.line, sizeof(tok.line))) ==
		   FILE_GET_LINE_SUCCESS )
	{
		if( tokenizer(&tok) != 0 ) { continue; }
		line++;
		if( (tok.count != 0) && (tok.tokens[0][0] != '#') )
		{
			if( tok.count > 1 )
			{
				AUTOCODE_MSG_ERROR(
					"in file %s wrong token count line %i\n", file_signals.name, line);
			}
			fprintf(parse->file, "\t%s,\n", tok.tokens[0]);
		}
	}
	if( line_result == FILE_GET_LINE_ERROR )
	{
		AUTOCODE_MSG_ERROR("reading file <%s> after line %i", file_signals.name, line);
	}
	fprintf(parse->file, "\tGPIO_SIGNAL_COUNT\n");
	fprintf(parse->file, "} gpio_signal_t;\n");

	tokenizerFree(&tok);
	fileClose(&file_signals, __FILE__, __LINE__);
	have_tag_count[HAVE_GPIO_SIGNALS]++;
}

static void writeHalInit(const parse_tag_t *parse)
{
	file_t file_list;
	fileInit(&file_list);
	file_list.name = (char *)parse->auto_options->file_halinit_list;
	if( fileOpen(&file_list, "r", FILE_READONLY, __FILE__, __LINE__) != 0 ) { return; }

	tokenizer_t tok = {0};
	file_get_line_result_t line_result;
	while( (line_result = fileGetLine(&file_list, tok.line, sizeof(tok.line))) ==
		   FILE_GET_LINE_SUCCESS )
	{
		if( tokenizer(&tok) != 0 ) { continue; }
		if( tok.count != 0 ) { fprintf(parse->file, "#include \"%s\"\n", tok.tokens[0]); }
	}
	if( line_result == FILE_GET_LINE_ERROR )
	{
		AUTOCODE_MSG_ERROR("reading file <%s>", file_list.name);
	}
	tokenizerFree(&tok);
	fileClose(&file_list, __FILE__, __LINE__);

	have_tag_count[HAVE_HAL_INIT]++;
}

static void writeHalDefine(const parse_tag_t *parse)
{
	file_t file_list;
	fileInit(&file_list);
	file_list.name = (char *)parse->auto_options->file_haldefine_list;
	if( fileOpen(&file_list, "r", FILE_READONLY, __FILE__, __LINE__) != 0 ) { return; }

	tokenizer_t tok = {0};
	file_get_line_result_t line_result;
	while( (line_result = fileGetLine(&file_list, tok.line, sizeof(tok.line))) ==
		   FILE_GET_LINE_SUCCESS )
	{
		if( tokenizer(&tok) != 0 ) { continue; }
		if( tok.count != 0 ) { fprintf(parse->file, "#include \"%s\"\n", tok.tokens[0]); }
	}
	if( line_result == FILE_GET_LINE_ERROR )
	{
		AUTOCODE_MSG_ERROR("reading file <%s>", file_list.name);
	}
	tokenizerFree(&tok);
	fileClose(&file_list, __FILE__, __LINE__);

	have_tag_count[HAVE_HAL_DEFINE]++;
}

static void writeModulesList(const parse_tag_t *parse)
{
	const module_type_t *mod = &parse->data_base->modules_type[TM_MOD_THREAD_ID];

	for( int i = 0; i < mod->modules_count; i++ )
	{
		if( mod->modules[i].subtype == THREAD_BIT_TYPE_SYS )
		{
			fprintf(parse->file, "#include \"system/services/%s.h\"\n", mod->modules[i].name);
		}
		if( mod->modules[i].subtype == THREAD_BIT_TYPE_USER )
		{
			fprintf(parse->file, "#include \"user/tasks/%s.h\"\n", mod->modules[i].name);
		}
	}
	fprintf(parse->file, "\n");

	mod = &parse->data_base->modules_type[TM_MOD_DRIVER_ID];

	for( int i = 0; i < mod->modules_count; i++ )
	{
		fprintf(parse->file, "#include \"interfaces/drv_%s.h\"\n", mod->modules[i].name);
	}
	have_tag_count[HAVE_MOD_LIST]++;
}

static void writeModulesCount(const parse_tag_t *parse)
{
	fprintf(parse->file,
			"#define TM_MOD_DRIVER_COUNT %i\n",
			parse->data_base->modules_type[TM_MOD_DRIVER_ID].modules_count);
	fprintf(parse->file,
			"#define TM_MOD_THREAD_COUNT %i\n",
			parse->data_base->modules_type[TM_MOD_THREAD_ID].modules_count);

	have_tag_count[HAVE_MOD_COUNT]++;
}

static void writeThreadsAlloc(const parse_tag_t *parse)
{
	int threads_count = 1;
	int thread_index;
	bool in_system = false;
	bool system_thread_found = false;
	const module_type_t *mod;

	fprintf(parse->file, "\tmod_thread_item_t *mod;\n");

	mod = &parse->data_base->modules_type[TM_MOD_THREAD_ID];

	// List other threads
	for( int i = 0; i < mod->modules_count; i++ )
	{
		// The first thread must be the system thread
		if( strcmp(mod->modules[i].name, "system") == 0 )
		{
			thread_index = 0;
			in_system = true;
			system_thread_found = true;
		}
		else { thread_index = threads_count; }

		fprintf(parse->file, "\n\tmod = mod_threadGetPointer(%i);\n", thread_index);

		fprintf(parse->file,
				"\n\thal_threadContextInit(%s, &(mod->stack_pointer), "
				"&(mod->stack[TM_MOD_THREAD_STACK_SIZE - 1]));\n",
				mod->modules[i].name);

		fprintf(parse->file, "\tmod->software_time_counter = 0;\n");
		fprintf(parse->file,
				"\tTM_STR_ROM_NEW(thread%i_name, \"%s\");\n",
				thread_index,
				mod->modules[i].name);
		fprintf(parse->file, "\tmod->name = &thread%i_name;\n", thread_index);
		fprintf(parse->file, "\tmod->status = %i;\n", mod->modules[i].status);
		fprintf(
			parse->file, "\tmod->saved_run_level = %i;\n", mod->modules[i].status & RL_LEVEL_MASK);
		fprintf(parse->file, "\tmod->main = %s;\n", mod->modules[i].name);

		if( in_system == false ) { threads_count++; }
		in_system = false;
	}
	have_tag_count[HAVE_THREADS_ALLOC]++;

	if( system_thread_found == false ) { AUTOCODE_MSG_ERROR("thread system was not found."); }
}

static void writeDriversAlloc(const parse_tag_t *parse)
{
	const module_type_t *mod = &parse->data_base->modules_type[TM_MOD_DRIVER_ID];

	fprintf(parse->file, "\tmod_driver_item_t *mod;\n");
	fprintf(parse->file, "\thal_driver_control_data_t control_data;\n");

	for( int i = 0; i < mod->modules_count; i++ )
	{
		fprintf(parse->file, "\n\tmod = mod_driverGetPointer(%i);\n", i);
		fprintf(parse->file, "\tTM_STR_ROM_NEW(driver%i_name, \"%s\");\n", i, mod->modules[i].name);
		fprintf(parse->file, "\tcontrol_data.run_level = %i;\n", mod->modules[i].status);
		fprintf(
			parse->file, "\thal_%sControl(DRV_CTRL_RLSET, &control_data);\n", mod->modules[i].name);
		fprintf(parse->file, "\t*(mod) = (mod_driver_item_t)\n");
		fprintf(parse->file, "\t{\n");
		fprintf(parse->file, "\t\t.name = &driver%i_name,\n", i);
		if( mod->modules[i].address == TM_MOD_DRIVER_ADDRESS_NONE )
		{
			fprintf(parse->file, "\t\t.address = TM_MOD_DRIVER_ADDRESS_NONE,\n");
		}
		else { fprintf(parse->file, "\t\t.address = 0x%02X,\n", mod->modules[i].address); }
		fprintf(parse->file, "\t\t.control = hal_%sControl\n", mod->modules[i].name);
		fprintf(parse->file, "\t};\n");
	}
	have_tag_count[HAVE_DRIVERS_ALLOC]++;
}

static void writeErrorCatalog(const parse_tag_t *parse)
{

	for( int i = 0; i < parse->errors->error_count; i++ )
	{
		if( parse->errors->catalog[i].level != ERR_LEVEL_FLOW )
		{
			fprintf(
				parse->file, "TM_STR_ROM_NEW(err%i, %s);\n", i, parse->errors->catalog[i].message);
		}
	}

	fprintf(parse->file, "\nconst err_item_t error_catalog[] = \n{\n");

	for( int i = 0; i < parse->errors->error_count; i++ )
	{
		if( parse->errors->catalog[i].level == ERR_LEVEL_FLOW )
		{
			fprintf(parse->file, "\t{NULL, ERR_LEVEL_FLOW},\n");
		}
		else
		{
			fprintf(parse->file,
					"\t{&err%i, %s},\n",
					i,
					errorLevelName(parse->errors->catalog[i].level));
		}
	}
	fprintf(parse->file, "};\n");

	have_tag_count[HAVE_ERROR_CATALOG]++;
}

static const char *errorLevelName(const err_level_t level)
{
	switch( level )
	{
		case ERR_LEVEL_FLOW:
			return "ERR_LEVEL_FLOW";
		case ERR_LEVEL_WARN:
			return "ERR_LEVEL_WARN";
		case ERR_LEVEL_FAIL:
			return "ERR_LEVEL_FAIL";
		case ERR_LEVEL_PANIC:
			return "ERR_LEVEL_PANIC";
	}

	AUTOCODE_MSG_ERROR("unknown TaskMate error level <%i>", level);

	return "ERR_LEVEL_FAIL";
}

static void writeErrorEnum(const parse_tag_t *parse)
{
	// Write the error enum
	fprintf(parse->file, "typedef enum\n");
	fprintf(parse->file, "{\n");

	for( int i = 0; i < parse->errors->error_count; i++ )
	{
		fprintf(parse->file, "\t%s,\n", parse->errors->catalog[i].name);
	}
	fprintf(parse->file, "\tERROR_COUNT\n");
	fprintf(parse->file, "} err_codes_t;\n\n");

	have_tag_count[HAVE_ERROR_ENUM]++;
}
