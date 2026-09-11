################################################################################
#
# TaskMate Project
# (c) 2026 PRADERE Sebastien
#
# This file is part of TaskMate and is distributed under the BSD-2-Clause License.
# See the LICENSE file for full license terms.
#
################################################################################

.ifndef HWT_MK
HWT_MK = 1

# Registration in the global system
VAL_HW_STACK = test_noscli

PATH_TEST = ${PATH_SRCS}/user/target/test_noscli
PATHS_SOURCES = ${PATH_TEST}
FILES_HALINIT = ${PATH_TEST}/targetInit.h
FILES_HALDEFINE = ${PATH_TEST}/target_define.h

FILE_GPIO_SIGNALS = ${PATH_TEST}/signals.gpio
.if !exists(${FILE_GPIO_SIGNALS})
.error GPIO signals list not found >>>${FILE_GPIO_SIGNALS}<<<
.endif

CFLAGS += -DHWT_test_noscli

.include "${PATH_SRCS}/hal/board/arduinoMega/make.mk"

.else
.error Multiple inclusion of ${.PARSEDIR}/${.PARSEFILE}
.endif
