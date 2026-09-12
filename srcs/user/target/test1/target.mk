################################################################################
#
# TaskMate Project
# (c) 2026 PRADERE Sebastien
#
# This file is part of TaskMate and is distributed under the BSD-2-Clause License.
# See the LICENSE file for full license terms.
#
################################################################################

.ifndef TARGET_MK
TARGET_MK = 1

################################################################################
# test1 hardware target
################################################################################

# Registration in the global system
VAL_HW_STACK = test1

PATH_TEST1 = ${PATH_SRCS}/user/target/test1
PATHS_SOURCES = ${PATH_TEST1}
FILES_HALINIT_HEADER = ${PATH_TEST1}/targetInit.h
FILES_HALDEFINE = ${PATH_TEST1}/define.h
VAL_FUNCINIT = targetInit

FILE_GPIO_SIGNALS = ${PATH_TEST1}/signals.gpio
.if !exists(${FILE_GPIO_SIGNALS})
.error GPIO signals list not found >>>${FILE_GPIO_SIGNALS}<<<
.endif

CFLAGS += -DHWT_test1

.include "${PATH_SRCS}/hal/board/arduinoMega/make.mk"

.else
.error Multiple inclusion of ${.PARSEDIR}/${.PARSEFILE}
.endif
