/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file tm_snprintf.c
 * @brief tm snprintf implementation.
 *
 */

#include "tm_snprintf.h"

#include "hal/public/tmlibc.h"
#include "interfaces/tm_define.h"
#include "system/sysCall/sysCall.h"

static void baseConvert(uint16_t value, uint8_t base);
static void tm_putChar(char ch);

#define SNPRINTF_BUFF_TEMP_SIZE 32

static uint8_t tm_snprintf_lock = 0;

// Buffer data structure
typedef struct
{
	char *ptr;
	uint8_t size;
	uint8_t index;
	uint8_t padding;
} tm_snprintf_buffer_t;

static tm_snprintf_buffer_t tm_snprintf_buffer;

// Reverse-order base converter
static void baseConvert(uint16_t value, uint8_t base)
{
	const char digits[] = "0123456789abcdef";

	char tmp[SNPRINTF_BUFF_TEMP_SIZE];
	uint8_t pos = 0;

	// Convert in reverse order
	if( value == 0 ) { tmp[pos++] = '0'; }
	else
	{
		while( (value != 0) && (pos < SNPRINTF_BUFF_TEMP_SIZE) )
		{
			uint16_t data = (value % base);
			value /= base;
			tmp[pos++] = digits[data];
		}
	}

	while( pos < tm_snprintf_buffer.padding ) { tmp[pos++] = '0'; }

	// Reverse order
	while( pos > 0 ) { tm_putChar(tmp[--pos]); }
}

int tm_snprintf(char *ptr, uint8_t size, const tm_string_t format, ...)
{
	int ret;
	va_list args;
	va_start(args, format);
	ret = tm_vsnprintf(ptr, size, format, args);
	va_end(args);
	return ret;
}

int tm_printf(const tm_string_t format, ...)
{
	int ret;
	va_list args;
	va_start(args, format);
	ret = tm_vprintf(format, args);
	va_end(args);
	return ret;
}

int tm_vprintf(const tm_string_t format, va_list args)
{
	return tm_vsnprintf(NULL, 0, format, args);
}

static void tm_putChar(char ch)
{
	if( tm_snprintf_buffer.ptr != NULL )
	{
		if( (tm_snprintf_buffer.index + 1) < (tm_snprintf_buffer.size - 1) )
		{
			tm_snprintf_buffer.ptr[tm_snprintf_buffer.index++] = ch;
		}
		else { tm_snprintf_buffer.ptr[tm_snprintf_buffer.index - 1] = 0; }
	}

	if( tm_snprintf_buffer.ptr == NULL ) { hal_stdio_putChar(ch); }
}

int tm_vsnprintf(char *ptr, uint8_t size, const tm_string_t format, va_list args)
{
	if( tm_snprintf_lock == 1 ) { sc_coopYield(); }
	tm_snprintf_lock = 1;

	// Store variables
	tm_snprintf_buffer.ptr = ptr;
	tm_snprintf_buffer.size = size;
	tm_snprintf_buffer.index = 0;
	tm_snprintf_buffer.padding = 0;

	uint8_t format_index = 0;
	char format_c = hal_string_getChar(&format, format_index++);

	while( format_c )
	{
		if( format_c == '%' )
		{
			format_c = hal_string_getChar(&format, format_index++);

			if( (format_c) == '0' )
			{
				format_c = hal_string_getChar(&format, format_index++);
				tm_snprintf_buffer.padding = (uint8_t)(format_c - 48); // atoi
				if( tm_snprintf_buffer.padding > 9 ) { goto exit; }
				format_c = hal_string_getChar(&format, format_index++);
			}

			switch( format_c )
			{

				case 'c':
				{
					int cc = va_arg(args, int);
					tm_putChar((char)cc);
					break;
				}

				case 's':
				{
					const tm_string_t *str = va_arg(args, const tm_string_t *);
					uint8_t str_index = 0;
					char str_c = hal_string_getChar(str, str_index++);
					while( str_c != 0 )
					{
						tm_putChar(str_c);
						str_c = hal_string_getChar(str, str_index++);
					}
					break;
				}

				case 'i':
				case 'x':
				case 'b':
				{
					uint16_t value = va_arg(args, uint16_t);
					uint8_t base;

					switch( format_c )
					{
						case 'i':
							base = 10;
							break;
						case 'x':
							base = 16;
							break;
						case 'b':
							base = 2;
							break;
						default:
							base = 10;
							break;
					}
					baseConvert(value, base);
					tm_snprintf_buffer.padding = 0;
					break;
				}

				case '%':
					tm_putChar('%');
					break;
				default:
					tm_putChar('?');
					break;
			}
		}
		else { tm_putChar(format_c); }
		format_c = hal_string_getChar(&format, format_index++);
	}

	tm_putChar(0); // close string

exit:

	tm_snprintf_lock = 0;
	return tm_snprintf_buffer.index;
}
