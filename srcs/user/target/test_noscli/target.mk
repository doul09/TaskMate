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
# test_noscli hardware target, same as test1 without SCLI
################################################################################

# Registration in the global system
VAL_HW_STACK = test_noscli

PATH_TEST_NOSCLI = ${PATH_SRCS}/user/target/test_noscli
PATHS_SOURCES = ${PATH_TEST_NOSCLI}
FILES_HALINIT_HEADER = ${PATH_TEST_NOSCLI}/test_noscliInit.h
FILES_HALDEFINE = ${PATH_TEST_NOSCLI}/define.h
VAL_FUNCINIT = test_noscliInit

FILE_GPIO_SIGNALS = ${PATH_TEST_NOSCLI}/signals.gpio
.if !exists(${FILE_GPIO_SIGNALS})
.error GPIO signals list not found >>>${FILE_GPIO_SIGNALS}<<<
.endif

CFLAGS += -DHWT_test_noscli

.include "${PATH_SRCS}/hal/board/arduinoMega/make.mk"

.else
.error Multiple inclusion of ${.PARSEDIR}/${.PARSEFILE}
.endif
