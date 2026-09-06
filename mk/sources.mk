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
# Sources
################################################################################

OPT_FIND_EXCLUDE = ! -path '*/.*'

# Source files and objects
FILES_SRC != find ${PATHS_SOURCES} ${OPT_FIND_EXCLUDE} -type f -name "*.c"
FILES_SRC_H != find ${PATHS_SOURCES} ${OPT_FIND_EXCLUDE} -type f -name "*.h"
FILES_DRIVER_INTERFACES != find ${PATH_SRCS}/interfaces ${OPT_FIND_EXCLUDE} -type f \
	-name "drv_*.h" | sort

FILES_OBJ = ${FILES_SRC:%.c=${PATH_BUILD_TARGET}/%.o}

# Dependency files
FILES_DEP = ${FILES_OBJ:.o=.d}
FILE_DEPS_ALL = ${PATH_BUILD_TARGET}/.deps.d

.sinclude "${FILE_DEPS_ALL}"

# autoCode
FILES_AUTOCODE_SRC != find ${PATH_SRCS}/autoCode ${OPT_FIND_EXCLUDE} -type f -name "*.c"
FILES_AUTOCODE_SRC_H != find ${PATH_SRCS}/autoCode ${OPT_FIND_EXCLUDE} -type f -name "*.h"

VAL_DATE_TIME != date +"%Y_%m_%d_%H:%M:%S"

FILES_INITRC != find ${PATHS_SOURCES} ${OPT_FIND_EXCLUDE} -type f -name "*.rc"

# Global error
FILES_ERROR != find ${PATHS_SOURCES} ${OPT_FIND_EXCLUDE}  -type f -name "*.err" | sort
FILES_ERROR += ${PATH_SRCS}/hal/drivers_errors.err

# Documentation files
FILES_DOC != find ${PATH_DOCS} ${OPT_FIND_EXCLUDE} -type f -name "*.md"; \
		find ${PATH_DOCS} ${OPT_FIND_EXCLUDE} -type f -name "*.txt"

# Makefiles
FILES_MK_MK != find  ./${PATH_MAKEFILES} ${OPT_FIND_EXCLUDE} -type f -name "*.mk"
FILES_MK_HAL != find ./${PATH_SRCS}/hal ${OPT_FIND_EXCLUDE} -type f -name "*.mk"

FILES_MK = ./Makefile ${FILES_MK_MK} ${FILES_MK_HAL}
