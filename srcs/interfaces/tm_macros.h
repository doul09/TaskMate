/*
 * TaskMate Project
 * (c) 2026 PRADERE Sebastien
 *
 * This file is part of TaskMate and is distributed under the BSD-2-Clause License.
 * See the LICENSE file for full license terms.
 */

/**
 * @file tm_macros.h
 * @brief macros header declarations.
 *
 */

#ifndef INTERFACES_TM_MACROS_H
#define INTERFACES_TM_MACROS_H

/* ============================================================================
 * Includes
 * ========================================================================== */

#include <stdint.h>

/*
 * Bit manipulation macros
 */


/* ============================================================================
 * Public definitions
 * ========================================================================== */

// Masks
#define TM_BIT_MASK_8(bit) ((uint8_t)(1u << (bit)))
#define TM_BIT_MASK_16(bit) ((uint16_t)(1u << (bit)))
#define TM_BIT_MASK_32(bit) ((uint32_t)(1UL << (bit)))
#define TM_BIT_MASK_64(bit) ((uint64_t)(1ULL << (bit)))

#define TM_REGISTER_BIT_MASK(reg, bit)          \
	((sizeof(reg) == 1)	  ? TM_BIT_MASK_8(bit)  \
	 : (sizeof(reg) == 2) ? TM_BIT_MASK_16(bit) \
	 : (sizeof(reg) == 4) ? TM_BIT_MASK_32(bit) \
						  : TM_BIT_MASK_64(bit))
// N-element macros
#define TM_SET_BITS_1(reg, a) TM_REGISTER_BIT_MASK(reg, a)

#define TM_SET_BITS_2(reg, a, b) (TM_REGISTER_BIT_MASK(reg, a) | TM_REGISTER_BIT_MASK(reg, b))

#define TM_SET_BITS_3(reg, a, b, c) \
	(TM_REGISTER_BIT_MASK(reg, a) | TM_REGISTER_BIT_MASK(reg, b) | TM_REGISTER_BIT_MASK(reg, c))

#define TM_SET_BITS_4(reg, a, b, c, d)                                                            \
	(TM_REGISTER_BIT_MASK(reg, a) | TM_REGISTER_BIT_MASK(reg, b) | TM_REGISTER_BIT_MASK(reg, c) | \
	 TM_REGISTER_BIT_MASK(reg, d))

// Selection macros
#define TM_REGISTER_SELECT(arg1, arg2, arg3, arg4, name, ...) name

#define TM_SET_BITS(reg, ...)                                                                    \
	TM_REGISTER_SELECT(__VA_ARGS__, TM_SET_BITS_4, TM_SET_BITS_3, TM_SET_BITS_2, TM_SET_BITS_1)( \
		reg, __VA_ARGS__)

// User API macros
#define TM_WRITEBIT(reg, ...) ((reg) = TM_SET_BITS((reg), __VA_ARGS__))
#define TM_SETBIT(reg, ...) ((reg) |= TM_SET_BITS((reg), __VA_ARGS__))
#define TM_CLEARBIT(reg, ...) ((reg) &= (__typeof__(reg))~(TM_SET_BITS((reg), __VA_ARGS__)))
#define TM_GETBIT(reg, bit) ((reg) & TM_REGISTER_BIT_MASK((reg), (bit)))

/*
 * Other macros
 */

// String macros
#define TM_CONCAT_INNER(a, b) a##b
#define TM_CONCAT(a, b) TM_CONCAT_INNER(a, b)
#define TM_STRINGIFY_INNER(x) #x
#define TM_STRINGIFY(x) TM_STRINGIFY_INNER(x)

// Store the source file name in ROM
#define TM_STORE_FILE_NAME(name) TM_STR_ROM_NEW(name, __FILE_NAME__)

// Generate a unique name
#define TM_UNIQUE_NAME(prefix) TM_CONCAT(prefix, __LINE__)

#endif // INTERFACES_TM_MACROS_H
