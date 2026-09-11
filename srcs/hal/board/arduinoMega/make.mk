################################################################################
#
# TaskMate Project
# (c) 2026 PRADERE Sebastien
#
# This file is part of TaskMate and is distributed under the BSD-2-Clause License.
# See the LICENSE file for full license terms.
#
################################################################################

.ifndef HAL_BOARD_ARDUINOMEGA_MAKE_MK
HAL_BOARD_ARDUINOMEGA_MAKE_MK = 1

################################################################################
# Arduino Mega board makefile
################################################################################

# Registration in the global system
VAL_HW_STACK += arduinoMega
PATH_ARDUINOMEGA = ${PATH_SRCS}/hal/board/arduinoMega
PATHS_SOURCES += ${PATH_ARDUINOMEGA}
FILES_HALINIT_HEADER += ${PATH_ARDUINOMEGA}/arduinoMega_init.h
FILES_HALDEFINE += ${PATH_ARDUINOMEGA}/define.h
VAL_FUNCINIT := hal_arduinoMegaInit ${VAL_FUNCINIT}
CFLAGS += -DBOARD_arduinoMega

.include "${PATH_SRCS}/hal/mcu/atmega2560/make.mk"

# Board settings
VAL_CPU_FREQ = 16000000UL

VAL_PROGRAMMER = avrispmkII
VAL_PROGRAMMER_PORT = /dev/ttyU0

.else
.error Multiple inclusion of ${.PARSEDIR}/${.PARSEFILE}
.endif
