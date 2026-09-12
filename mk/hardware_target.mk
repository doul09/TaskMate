################################################################################
#
# TaskMate Project
# (c) 2026 PRADERE Sebastien
#
# This file is part of TaskMate and is distributed under the BSD-2-Clause License.
# See the LICENSE file for full license terms.
#
################################################################################

################################################################################
# Hardware target selection and test
################################################################################

# Default target
VAL_TARGET ?= test1
FILE_TARGET_MK = ${PATH_SRCS}/user/target/${VAL_TARGET}/target.mk

.if !exists(${FILE_TARGET_MK})
.error Target not found >>>${VAL_TARGET}/target.mk<<<
.endif

.include "${FILE_TARGET_MK}"

# Check build hardware stack
_hardware_target_check: ${FILE_HARDWARE_TARGETS_CONF} ${FILE_HARDWARE_TARGET_CHECK_SCRIPT}
	@printf "\n%sChecking hardware target ...%s\n" \
		"${COLOUR_TARGET_INFO}" "${COLOUR_RESET}"
	@awk -v hardware_target="${VAL_HW_STACK}" \
		-f "${FILE_HARDWARE_TARGET_CHECK_SCRIPT}" "${FILE_HARDWARE_TARGETS_CONF}"
.PHONY: _hardware_target_check
