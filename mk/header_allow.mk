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
# Allowed source access to system-critical headers
################################################################################

CFLAGS_${PATH_SRCS}/system/sysCore/modules.c = \
	-DHAL_SYSTEM_CRITICAL_ALLOWED \
	-DTM_SYSTEM_CRITICAL_ALLOWED

CFLAGS_${PATH_SRCS}/system/sysCore/tm_scheduler.c = \
	-DHAL_SYSTEM_CRITICAL_ALLOWED \
	-DTM_SYSTEM_CRITICAL_ALLOWED

CFLAGS_${PATH_SRCS}/system/TaskMate.c = \
	-DHAL_SYSTEM_CRITICAL_ALLOWED \
	-DTM_SYSTEM_CRITICAL_ALLOWED

CFLAGS_${PATH_SRCS}/system/sysCore/tm_softwareTimeCounter.c = \
	-DTM_SYSTEM_CRITICAL_ALLOWED

CFLAGS_${PATH_SRCS}/system/sysCall/sc_hal.c = \
	-DTM_SYSTEM_CRITICAL_ALLOWED

CFLAGS_${PATH_SRCS}/system/sysCall/sc_modules.c = \
	-DTM_SYSTEM_CRITICAL_ALLOWED

CFLAGS_${PATH_SRCS}/system/services/commands/driver.c = \
	-DTM_SYSTEM_CRITICAL_ALLOWED

CFLAGS_${PATH_SRCS}/system/sysCore/gpio.c = \
	-DHAL_SYSTEM_CRITICAL_ALLOWED


# Check includes for system-critical features
_system_critical_check:
	@printf "\n%sChecking forbidden system critical includes ...%s\n" \
		"${COLOUR_TARGET_INFO}" "${COLOUR_RESET}"

	@awk ${COLOURS_AWK} -v PATH_SOURCES=${PATH_SRCS} -v h_check_log=${FILE_H_CHECK_LOG} \
		-f ${PATH_SCRIPTS}/header_allow.awk "${FILE_SYSTEM_HEADER_ALLOW_CONF}"

# Check direct includes against the architecture matrix
_architecture_include_check: ${FILE_ARCH_VALID_MATRIX} ${FILE_ARCH_CHECK_SCRIPT}
	@printf "\n%sChecking architecture direct includes ...%s\n" \
		"${COLOUR_TARGET_INFO}" "${COLOUR_RESET}"

	@if awk -v matrix_file="${FILE_ARCH_VALID_MATRIX}" -v path_sources="${PATH_SRCS}" \
		-f "${FILE_ARCH_CHECK_SCRIPT}" "${FILE_ARCH_VALID_MATRIX}" \
		${FILES_SRC} ${FILES_SRC_H} > "${FILE_ARCH_CHECK_LOG}"; then \
		cat "${FILE_ARCH_CHECK_LOG}"; \
	else \
		status=$$?; cat "${FILE_ARCH_CHECK_LOG}"; echo ">>> satus : " $$status; \
	fi

.PHONY: _system_critical_check _architecture_include_check
