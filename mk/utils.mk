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
# Utility
################################################################################

clean:
#help [global] Remove all build files.
	@printf "\n%sRemove files%s\n\n" \
		"${COLOUR_CLEAN}" "${COLOUR_RESET}"
	@printf "${COLOUR_CLEAN_SOFT}"
	@printf "${PATH_BUILD_TARGET}/**/*.o ${PATH_BUILD_TARGET}/**/*.d ${PATH_BUILD_TARGET}/TaskMate*\n"
	@printf "{FILE_AUTOCODE_TARGET} {PATH_BUILD_TARGET}/.autoCode_stamp* {PATH_BUILD_TARGET}/autoCode_*\n"
	@printf "${COLOUR_RESET}"	
	@if [ -d "${PATH_BUILD_TARGET}" ]; then \
		find "${PATH_BUILD_TARGET}" -type f \( -name "*.o" -o -name "*.d" \) -delete; \
	fi
	@rm -f ${PATH_BUILD_TARGET}/TaskMate*
	@rm -f ${FILE_AUTOCODE_TARGET} ${PATH_BUILD_TARGET}/.autoCode_stamp* ${PATH_BUILD_TARGET}/autoCode_*
.PHONY: clean

doc:
#help [global] Generate Doxygen documentation. Configuration file /doc/Doxyfile
	@printf "\n%sMake Doxygen documentation%s\n\n" \
		"${COLOUR_TARGET_INFO}" "${COLOUR_RESET}"
	doxygen ${PATH_DOCS}/Doxyfile
.PHONY: doc

_cloc_data:
	# Use :T because cloc does not handle a full path with the --exclude-dir option
	@cloc * --exclude-dir=${PATH_BUILDS:T} --exclude-lang=D --exclude-ext=rc,md,txt > ${FILE_CLOCRAW}
	@printf "\n" >> ${FILE_CLOCRAW}
	@cloc * --exclude-dir=${PATH_BUILDS:T} --exclude-lang=D,make --exclude-ext=rc,c,h,awk >> ${FILE_CLOCRAW}
	@awk -v file="${FILE_CLOCDATA}" -f ${PATH_SCRIPTS}/cloc_data.awk ${FILE_CLOCRAW}
.PHONY: _cloc_data

cloc: _cloc_data
#help [global] Count lines of code.
	@printf "\n%sCount lines of codes%s\n\n" \
		"${COLOUR_TARGET_INFO}" "${COLOUR_RESET}"

	@cat ${FILE_CLOCRAW}
	@printf "${COLOUR_WHITE_BOLD}\nTotal loc and ratio :\n"
	@awk -f ${PATH_SCRIPTS}/cloc_show.awk "${FILE_CLOCDATA}"
	@printf "${COLOUR_RESET}"
.PHONY: cloc

note:
#help [global] Look for TODO / FIX / HACK comments in code.
	@printf "\n%sLook for TODO / FIX / HACK%s\n\n" \
		"${COLOUR_TARGET_INFO}" "${COLOUR_RESET}"
	@grep -r -n -i -E 'TODO|FIX|HACK|enum' ${FILES_SRC} ${FILES_SRC_H} ${FILES_AUTOCODE_SRC} ${FILES_AUTOCODE_SRC_H}

.PHONY: note

cppcheck:
#help [global] cppcheck static code analysis for autoCode and TaskMate.
	@printf "\n%scppcheck static analysis%s\n\n" \
		"${COLOUR_TARGET_INFO}" "${COLOUR_RESET}"
	@cppcheck -I/root/code/TaskMate/TaskMate_current/ \
		--enable=all --inconclusive --force \
		--suppress=missingIncludeSystem \
		--suppress=missingInclude \
		--check-level=exhaustive \
		${FILES_SRC} \
		${FILES_AUTOCODE_SRC}
.PHONY: cppcheck

format:
#help [global] Format code with clang-format, configuration /.clang-format.
	@printf "%sAuto formatting code%s\n\n" \
		"${COLOUR_TARGET_INFO}" "${COLOUR_RESET}"
	clang-format19 -i ${FILES_SRC} ${FILES_SRC_H} ${FILES_AUTOCODE_SRC}
.PHONY: format

tidy_autoCode:
#help [global] tidy static code analysis for autoCode, configuration /.clang-tidy.
	@printf "\n%sTidy autoCode static code test%s\n\n" \
		"${COLOUR_TARGET_INFO}" "${COLOUR_RESET}"
	@clang-tidy19 $(FILES_AUTOCODE_SRC) ${FILES_AUTOCODE_SRC_H} --\
	-I/root/code/TaskMate/TaskMate_current/ \
	-I/root/code/TaskMate/TaskMate_current/${PATH_SRCS}/
.PHONY:tidy_autoCode

help:
#help [global] List all utility targets, not the system ones.
	@printf "%sPrint all utility targets%s\n\n" \
		"${COLOUR_TARGET_INFO}" "${COLOUR_RESET}"
	@awk ${COLOURS_AWK} -f ${PATH_SCRIPTS}/make_help.awk ${FILES_MK}
.PHONY: help
