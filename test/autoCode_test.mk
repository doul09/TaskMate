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
# autoCode black-box tests
################################################################################

CFLAGS_AUTOCODE_TEST_SANITIZE = ${CFLAGS_AUTOCODE}
CFLAGS_AUTOCODE_TEST_SANITIZE += -fsanitize=address,undefined -fno-omit-frame-pointer

test_autoCode: test_autoCode_command_line .WAIT test_autoCode_options .WAIT \
	test_autoCode_errors .WAIT test_autoCode_initrc .WAIT test_autoCode_parse_tag .WAIT \
	test_autoCode_compare_replace
#help [test] Run the complete autoCode black-box test corpus.
	@printf "\n%sAll autoCode tests passed%s\n\n" \
		"${COLOUR_TARGET_INFO}" "${COLOUR_RESET}"
.PHONY: test_autoCode

test_autoCode_command_line: ${FILE_AUTOCODE_TARGET} ${FILE_AUTOCODE_TEST_SCRIPT}
#help [test] Test autoCode command-line validation.
	@${FILE_AUTOCODE_TEST_SCRIPT} command_line \
		"./${FILE_AUTOCODE_TARGET}" "${PATH_BUILD_AUTOCODE_TEST}"
.PHONY: test_autoCode_command_line

test_autoCode_options: ${FILE_AUTOCODE_TARGET} ${FILE_AUTOCODE_TEST_SCRIPT}
#help [test] Test autoCode configuration options.
	@${FILE_AUTOCODE_TEST_SCRIPT} options \
		"./${FILE_AUTOCODE_TARGET}" "${PATH_BUILD_AUTOCODE_TEST}"
.PHONY: test_autoCode_options

test_autoCode_errors: ${FILE_AUTOCODE_TARGET} ${FILE_AUTOCODE_TEST_SCRIPT}
#help [test] Test autoCode error files.
	@${FILE_AUTOCODE_TEST_SCRIPT} errors \
		"./${FILE_AUTOCODE_TARGET}" "${PATH_BUILD_AUTOCODE_TEST}"
.PHONY: test_autoCode_errors

test_autoCode_initrc: ${FILE_AUTOCODE_TARGET} ${FILE_AUTOCODE_TEST_SCRIPT}
#help [test] Test autoCode init.rc files.
	@${FILE_AUTOCODE_TEST_SCRIPT} initrc \
		"./${FILE_AUTOCODE_TARGET}" "${PATH_BUILD_AUTOCODE_TEST}"
.PHONY: test_autoCode_initrc

test_autoCode_parse_tag: ${FILE_AUTOCODE_TARGET} ${FILE_AUTOCODE_TEST_SCRIPT}
#help [test] Test autoCode tag parsing and tag inputs.
	@${FILE_AUTOCODE_TEST_SCRIPT} parse_tag \
		"./${FILE_AUTOCODE_TARGET}" "${PATH_BUILD_AUTOCODE_TEST}"
.PHONY: test_autoCode_parse_tag

test_autoCode_compare_replace: ${FILE_AUTOCODE_TARGET} ${FILE_AUTOCODE_TEST_SCRIPT}
#help [test] Test deferred autoCode comparison and replacement.
	@${FILE_AUTOCODE_TEST_SCRIPT} compare_replace \
		"./${FILE_AUTOCODE_TARGET}" "${PATH_BUILD_AUTOCODE_TEST}"
.PHONY: test_autoCode_compare_replace

${FILE_AUTOCODE_TEST_SANITIZE_TARGET}: ${FILES_AUTOCODE_SRC} ${FILES_AUTOCODE_SRC_H} \
										 ${FILE_ERROR_LEVEL}
	@printf "\n%sCompiling autoCode with sanitizers%s\n\n" \
		"${COLOUR_TARGET_INFO}" "${COLOUR_RESET}"
	clang ${CFLAGS_AUTOCODE_TEST_SANITIZE} ${FILES_AUTOCODE_SRC} \
		-o ${FILE_AUTOCODE_TEST_SANITIZE_TARGET}

test_autoCode_sanitize: ${FILE_AUTOCODE_TEST_SANITIZE_TARGET} ${FILE_AUTOCODE_TEST_SCRIPT}
#help [test] Run the complete autoCode corpus with ASan and UBSan.
	@ASAN_OPTIONS=detect_leaks=0 ${FILE_AUTOCODE_TEST_SCRIPT} all \
		"./${FILE_AUTOCODE_TEST_SANITIZE_TARGET}" "${PATH_BUILD_AUTOCODE_TEST}/sanitize"
.PHONY: test_autoCode_sanitize
