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
# Main Makefile
################################################################################

# Main directories
PATH_SRCS = srcs
PATH_BUILDS = build
PATH_LOGS = ${PATH_BUILDS}/log
PATH_MAKEFILES = mk
PATH_CONF = conf
PATH_DOCS = doc
PATH_SCRIPTS = scripts
PATH_TEST = test

# Build data
.include "${PATH_MAKEFILES}/options.mk"
.include "${PATH_MAKEFILES}/data.mk"
.include "${PATH_MAKEFILES}/colours.mk"

# Hardware target choice
.include "${PATH_MAKEFILES}/hardware_target.mk"

# File processing
.include "${PATH_MAKEFILES}/path_files.mk"
.include "${PATH_MAKEFILES}/sources.mk"
.include "${PATH_MAKEFILES}/header_allow.mk"

# Make global process
.include "${PATH_MAKEFILES}/autoCode.mk"
.include "${PATH_MAKEFILES}/build.mk"
.include "${PATH_MAKEFILES}/utils.mk"
.include "${PATH_MAKEFILES}/backup.mk"
.include "${PATH_MAKEFILES}/editors.mk"
.include "${PATH_TEST}/autoCode_test.mk"

# Architecture-specific Make logic
.include "${FILE_ARCH_CC}"
