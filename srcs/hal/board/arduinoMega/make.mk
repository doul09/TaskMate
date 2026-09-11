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

# Registration in the global system
VAL_HW_STACK += arduinoMega
PATH_ARDUINOMEGA = ${PATH_SRCS}/hal/board/arduinoMega
PATHS_SOURCES += ${PATH_ARDUINOMEGA}
FILES_HALINIT += ${PATH_ARDUINOMEGA}/boardInit.h
FILES_HALDEFINE += ${PATH_ARDUINOMEGA}/define.h
CFLAGS += -DBOARD_arduinoMega

.include "${PATH_SRCS}/hal/mcu/atmega2560/make.mk"

# Board settings
VAL_CPU_FREQ = 16000000UL

VAL_PROGRAMMER = avrispmkII
VAL_PROGRAMMER_PORT = /dev/ttyU0

.endif
