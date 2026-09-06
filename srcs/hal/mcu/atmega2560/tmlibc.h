/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file tmlibc.h
 * @brief tmlibc header declarations.
 *
 */

#ifndef ATMEGA2560_TMLIBC_H
#define ATMEGA2560_TMLIBC_H

// clang-format off

#include <avr/pgmspace.h>
#include <stdint.h>

#include "interfaces/tm_macros.h"
#include "interfaces/tm_string.h"

// Macro for file-scope string storage
#define TM_STR_ROM_NEW(name, txt) \
	static const char TM_UNIQUE_NAME(name)[] PROGMEM = (txt); \
	static const tm_string_t (name) = {.text = TM_UNIQUE_NAME(name), .storage = TM_MEM_ROM}

// Macro for function-scope string storage
#define TM_STR_ROM(string) ((tm_string_t){ .text = PSTR(string), .storage = TM_MEM_ROM })
#define TM_STR_RAM(string) ((tm_string_t){ .text = (string), .storage = TM_MEM_RAM })

// Default storage for AVR8
#define TM_STR(string) TM_STR_ROM(string)

// Functions
void hal_stdio_putChar(char ch);
char hal_string_getChar(const tm_string_t *str, uint8_t index);

// clang-format on

#endif // ATMEGA2560_TMLIBC_H
