/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file date.c
 * @brief Date command implementation.
 */

/* =============================================================================
 * Declarations - Include
 * ===========================================================================*/

#include "date.h"

#include "interfaces/drv_rtc.h"
#include "interfaces/tm_define.h"
#include "system/sysCall/error.h"
#include "system/sysCall/sc_hal.h"
#include "tm_libc/tm_string.h"
#include "tm_libc/tm_syslog.h"

/* -----------------------------------------------
 * Private types
 * ---------------------------------------------*/

typedef bool (*date_cmd_func_t)(uint8_t argc, char *argv[]);

typedef struct
{
	const char *name;
	date_cmd_func_t func;
} date_cmd_t;

/* -----------------------------------------------
 * Private function prototypes
 * ---------------------------------------------*/

static bool dateShow(uint8_t argc, char *argv[]);
static bool dateShowStartup(uint8_t argc, char *argv[]);
static bool dateSetTime(uint8_t argc, char *argv[]);
static bool dateSetDate(uint8_t argc, char *argv[]);
static bool dateHelp(uint8_t argc, char *argv[]);
static bool dateRead(hal_rtc_time_t *time);
static bool dateWrite(const hal_rtc_time_t *time);
static void datePrint(const hal_rtc_time_t *time);
static void datePrintError(err_codes_t error);
static bool dateParseField(
	const char **cursor, char separator, uint8_t digit_count_min, uint8_t digit_count_max,
	uint16_t *value);
static bool dateParseTime(const char *text, hal_rtc_time_t *time);
static bool dateParseDate(const char *text, hal_rtc_time_t *time);

/* -----------------------------------------------
 * Command table
 * ---------------------------------------------*/

static const date_cmd_t date_cmd[] = {
	{"startup", dateShowStartup},
	{"time", dateSetTime},
	{"date", dateSetDate},
	{"help", dateHelp},
	{0, 0},
};

/* =============================================================================
 * Implementation - Functions
 * ===========================================================================*/

/* -----------------------------------------------
 * Command dispatch
 * ---------------------------------------------*/

bool dateCommand(uint8_t argc, char *argv[])
{
	if( argc == 1u ) { return dateShow(argc, argv); }

	for( uint8_t i = 0; date_cmd[i].name != 0; i++ )
	{
		if( tm_strncmp(TM_STR_RAM(argv[1]), TM_STR_RAM(date_cmd[i].name), TM_STRING_SIZE_MAX) ==
			0 )
		{
			return date_cmd[i].func(argc, argv);
		}
	}

	dateHelp(0, NULL);
	return false;
}

/* -----------------------------------------------
 * Command handlers
 * ---------------------------------------------*/

static bool dateShow(uint8_t argc, char *argv[])
{
	(void)argc;
	(void)argv;

	hal_rtc_time_t time;
	if( !dateRead(&time) ) { return false; }
	datePrint(&time);
	return true;
}

static bool dateShowStartup(uint8_t argc, char *argv[])
{
	(void)argv;

	if( argc != 2u )
	{
		dateHelp(0, NULL);
		return false;
	}

	hal_rtc_time_t time;
	err_codes_t error = sc_rtcGetStartupTime(&time);
	if( error != ERR_NO_ERROR )
	{
		datePrintError(error);
		return false;
	}

	datePrint(&time);
	return true;
}

static bool dateSetTime(uint8_t argc, char *argv[])
{
	if( argc != 3u )
	{
		dateHelp(0, NULL);
		return false;
	}

	hal_rtc_time_t time;
	if( !dateRead(&time) ) { return false; }
	if( !dateParseTime(argv[2], &time) )
	{
		tm_syslog(TM_STR("[date] invalid time, expected hh:mm:ss\n"));
		return false;
	}
	if( !dateWrite(&time) ) { return false; }

	datePrint(&time);
	return true;
}

static bool dateSetDate(uint8_t argc, char *argv[])
{
	if( argc != 3u )
	{
		dateHelp(0, NULL);
		return false;
	}

	hal_rtc_time_t time;
	if( !dateRead(&time) ) { return false; }
	if( !dateParseDate(argv[2], &time) )
	{
		tm_syslog(TM_STR("[date] invalid date, expected day/month/year (2000-2099)\n"));
		return false;
	}
	if( !dateWrite(&time) ) { return false; }

	datePrint(&time);
	return true;
}

static bool dateHelp(uint8_t argc, char *argv[])
{
	(void)argc;
	(void)argv;

	tm_syslog(TM_STR("[date] usage:\n"));
	tm_syslog(TM_STR("\tdate\n"));
	tm_syslog(TM_STR("\tdate startup\n"));
	tm_syslog(TM_STR("\tdate time hh:mm:ss\n"));
	tm_syslog(TM_STR("\tdate date day/month/year\n"));
	tm_syslog(TM_STR("\tdate help\n"));
	return true;
}

/* -----------------------------------------------
 * RTC access and reporting
 * ---------------------------------------------*/

static bool dateRead(hal_rtc_time_t *time)
{
	err_codes_t error = sc_rtcRead(time);
	if( error == ERR_NO_ERROR ) { return true; }
	datePrintError(error);
	return false;
}

static bool dateWrite(const hal_rtc_time_t *time)
{
	err_codes_t error = sc_rtcWrite(time);
	if( error == ERR_NO_ERROR ) { return true; }
	datePrintError(error);
	return false;
}

static void datePrint(const hal_rtc_time_t *time)
{
	tm_syslog(TM_STR("[date] %02i/%02i/20%02i %02i:%02i:%02i\n"),
			  time->day,
			  time->month,
			  time->year,
			  time->hours,
			  time->minutes,
			  time->seconds);
}

static void datePrintError(err_codes_t error)
{
	const tm_string_t *message = err_getMessage((uint8_t)error);
	if( message != 0 ) { tm_syslog(TM_STR("[date] RTC error: %s\n"), message); }
	else { tm_syslog(TM_STR("[date] RTC error\n")); }
}

/* -----------------------------------------------
 * Input parsing
 * ---------------------------------------------*/

static bool dateParseField(
	const char **cursor, char separator, uint8_t digit_count_min, uint8_t digit_count_max,
	uint16_t *value)
{
	if( (cursor == 0) || (*cursor == 0) || (value == 0) ) { return false; }

	uint16_t parsed = 0;
	uint8_t digit_count = 0;
	while( ((**cursor >= '0') && (**cursor <= '9')) && (digit_count < digit_count_max) )
	{
		parsed = (parsed * 10u) + (uint8_t)(**cursor - '0');
		(*cursor)++;
		digit_count++;
	}
	if( (digit_count < digit_count_min) || (digit_count > digit_count_max) ) { return false; }

	if( separator == 0 )
	{
		if( **cursor != 0 ) { return false; }
	}
	else
	{
		if( **cursor != separator ) { return false; }
		(*cursor)++;
	}

	*value = parsed;
	return true;
}

static bool dateParseTime(const char *text, hal_rtc_time_t *time)
{
	if( (text == 0) || (time == 0) ) { return false; }

	const char *cursor = text;
	uint16_t hours;
	uint16_t minutes;
	uint16_t seconds;
	if( !dateParseField(&cursor, ':', 2u, 2u, &hours) ||
		!dateParseField(&cursor, ':', 2u, 2u, &minutes) ||
		!dateParseField(&cursor, 0, 2u, 2u, &seconds) )
	{
		return false;
	}
	if( (hours > 23u) || (minutes > 59u) || (seconds > 59u) ) { return false; }

	time->hours = (uint8_t)hours;
	time->minutes = (uint8_t)minutes;
	time->seconds = (uint8_t)seconds;
	return true;
}

static bool dateParseDate(const char *text, hal_rtc_time_t *time)
{
	if( (text == 0) || (time == 0) ) { return false; }

	const char *cursor = text;
	uint16_t day;
	uint16_t month;
	uint16_t year;
	if( !dateParseField(&cursor, '/', 1u, 2u, &day) ||
		!dateParseField(&cursor, '/', 1u, 2u, &month) ||
		!dateParseField(&cursor, 0, 4u, 4u, &year) )
	{
		return false;
	}
	if( (year < 2000u) || (year > 2099u) || (month < 1u) || (month > 12u) )
	{
		return false;
	}

	uint8_t rtc_year = (uint8_t)(year - 2000u);

	time->day = (uint8_t)day;
	time->month = (uint8_t)month;
	time->year = rtc_year;
	return true;
}
