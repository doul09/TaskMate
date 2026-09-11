################################################################################
#
# TaskMate Project
# (c) 2026 PRADERE Sebastien
#
# This file is part of TaskMate and is distributed under the BSD-2-Clause License.
# See the LICENSE file for full license terms.
#
################################################################################

.ifndef HAL_ARCH_AVR8_MAKE_MK
HAL_ARCH_AVR8_MAKE_MK = 1

################################################################################
# AVR architecture makefile
################################################################################

# Registration in the global system
VAL_HW_STACK += avr8

PATH_AVR8 = ${PATH_SRCS}/hal/arch/avr8
PATHS_SOURCES += ${PATH_AVR8}
FILES_HALINIT_HEADER += ${PATH_AVR8}/archInit.h
FILES_HALDEFINE += ${PATH_AVR8}/define.h
VAL_FUNCINIT := hal_archInit ${VAL_FUNCINIT}

CFLAGS += -DARCH_avr8
	
# Compilation redirection
FILE_ARCH_CC = ${PATH_SRCS}/hal/arch/avr8/avr8_CC.mk

# Compiler for the AVR8 architecture
CC = avr-gcc
VAL_CC_VERSION != avr-gcc -dumpversion

# General options
CFLAGS += -Os -MMD -MP -mrelax -fshort-enums
CFLAGS += -DF_CPU=${VAL_CPU_FREQ} -mmcu=${VAL_MCU_SERIAL}

# General warnings
CFLAGS += -Wall -Wextra -Wshadow -Werror=shadow -Wswitch -Wswitch-enum -Wformat=2 -Wformat-security

# Prototypes
CFLAGS += -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls \
	-Wbad-function-cast

# Conversions
CFLAGS += -Wconversion -Wsign-conversion -Wenum-conversion -Wcast-align -Wcast-qual

# Bug hunter
CFLAGS += -Wnull-dereference -Wundef -Werror=undef -Werror=implicit-function-declaration \
	-Werror=return-type -Wdouble-promotion -Wwrite-strings -fno-common -Wpointer-arith

# Command line #include and #define
CFLAGS += -I${PATH_SRCS} -I./

# Linker flags
CFLAGS += -ffunction-sections -fdata-sections -flto
LDFLAGS = -Wl,--gc-sections -Wl,-Map=${FILE_TARGET}.map

.else
.error Multiple inclusion of ${.PARSEDIR}/${.PARSEFILE}
.endif
