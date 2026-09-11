################################################################################
#
# TaskMate Project
# (c) 2026 PRADERE Sebastien
#
# This file is part of TaskMate and is distributed under the BSD-2-Clause License.
# See the LICENSE file for full license terms.
#
################################################################################

.ifndef HAL_MCU_ATMEGA2560_MAKE_MK
HAL_MCU_ATMEGA2560_MAKE_MK = 1

################################################################################
# ATMega 2560 makefile
################################################################################

# Registration in the global system
VAL_HW_STACK += atmega2560

PATH_ATMEGA2560 = ${PATH_SRCS}/hal/mcu/atmega2560
PATHS_SOURCES += ${PATH_ATMEGA2560}
FILES_HALINIT += ${PATH_ATMEGA2560}/mcuInit.h
FILES_HALDEFINE += ${PATH_ATMEGA2560}/define.h

CFLAGS += -DMCU_atmega2560

.include "${PATH_SRCS}/hal/arch/avr8/make.mk"

# MCU settings
VAL_MCU_SERIAL = atmega2560
VAL_MCU_ARCHTYPE = avr6

# ATmega2560 flash is 128 kB; PCHH is not implemented in the context switch,
# so flash is limited to 64 kB.
VAL_FLASH_SIZE_K=64
VAL_RAM_SIZE_K=8

.endif
