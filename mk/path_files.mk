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
# Directories, paths, and files
################################################################################

# Final full HW path
PATH_BUILD_TARGET = ${PATH_BUILDS}/${VAL_HW_STACK:ts_}

# Targets
FILE_TARGET = ${PATH_BUILD_TARGET}/TaskMate
FILE_AUTOCODE_TARGET = ${PATH_BUILDS}/autoCode
FILE_PROGRAMS_CHECK_STAMP = ${PATH_BUILDS}/.programs_check_stamp
FILE_PROGRAMS_LIST = ${PATH_CONF}/programs-list.conf
FILE_PROGRAMS_CHECK_SCRIPT = ${PATH_SCRIPTS}/check_programs.sh
FILE_HARDWARE_TARGETS_CONF = ${PATH_CONF}/hardware-targets.conf
FILE_HARDWARE_TARGET_CHECK_SCRIPT = ${PATH_SCRIPTS}/hardware_target.awk

# USB
PATH_USBKEY = /media/usbkey
FILE_USBDEV = /dev/da0s1

# Build information
FILE_BUILD_INFO = ${PATH_BUILDS}/last_build_info.txt

FILE_CLOCRAW = ${PATH_BUILDS}/cloc_raw
FILE_CLOCDATA = ${PATH_BUILDS}/cloc_data
FILE_MEMRAW = ${PATH_BUILD_TARGET}/mem_raw
FILE_MEMDATA = ${PATH_BUILD_TARGET}/mem_data
FILE_TM_INFO = ${PATH_SRCS}/interfaces/tm_info.h

# Source directories
PATHS_SOURCES += ${PATH_SRCS}/system
PATHS_SOURCES += ${PATH_SRCS}/tm_libc
PATHS_SOURCES += ${PATH_SRCS}/interfaces
PATHS_SOURCES += ${PATH_SRCS}/hal/public
PATHS_SOURCES += ${PATH_SRCS}/hal/drivers
PATHS_SOURCES += ${PATH_SRCS}/user/tasks

# Log files
FILE_AUTOCODE_LOG = ${PATH_LOGS}/autoCode_log
FILE_RSYNC_LOG = ${PATH_LOGS}/rsync.log
FILE_H_CHECK_LOG = ${PATH_LOGS}/headers_check.log

# Scripts
FILE_H_ALLOW_CONF = ${PATH_CONF}/header_allow.conf

# Git
FILE_GIT_IGNORE = .gitignore
PATHS_GIT_ALLOWED = ${PATH_CONF} ${PATH_DOCS} ${PATH_MAKEFILES} ${PATH_SCRIPTS} ${PATH_SRCS}
FILES_GIT_ALLOWED.${PATH_CONF} = *.conf
FILES_GIT_ALLOWED.${PATH_DOCS} = *.c *.md *.txt *.png *.jpg Doxyfile
FILES_GIT_ALLOWED.${PATH_MAKEFILES} = *.mk *
FILES_GIT_ALLOWED.${PATH_SRCS} = *.c *.h *.rc *.err *.mk *.gpio *.list *.awk
FILES_GIT_ALLOWED.${PATH_SCRIPTS} = *.awk *.sh
FILES_GIT_ALLOWED = .clang-format .clang-tidy AGENTS.md\
	audit_todo CHANGELOG LICENSE Makefile README.md
FILES_GIT_EXCLUDED = \
	${FILE_TM_INFO}
	
# Tags
FILE_TAGS = .tags
FILE_TAGS_STAMP = ${PATH_BUILD_TARGET}/.tags_stamp

# USB key directory backup
VAL_TM_BACKUP_DIR != printf "/code/TaskMate/TaskMate_%s" ${VAL_TM_VERSION}
