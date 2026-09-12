#!/bin/sh

################################################################################
#
# TaskMate Project
# (c) 2026 PRADERE Sebastien
#
# This file is part of TaskMate and is distributed under the BSD-2-Clause License.
# See the LICENSE file for full license terms.
#
################################################################################

set -u

VAL_STAGE=$1
FILE_AUTOCODE=$2
PATH_WORK_ROOT=$3
VAL_TEST_COUNT=0

writeInitrcVersion()
{
	printf '%s\n' '!set_version_major 1' '!set_version_minor 3'
}

fail()
{
	printf 'autoCode test failure: %s\n' "$1" >&2
	exit 1
}

stageBegin()
{
	PATH_STAGE_WORK="${PATH_WORK_ROOT}/$1"
	if [ -d "${PATH_STAGE_WORK}" ]; then
		find "${PATH_STAGE_WORK}" -depth -delete || fail "cannot clean ${PATH_STAGE_WORK}"
	fi
	mkdir -p "${PATH_STAGE_WORK}" || fail "cannot create ${PATH_STAGE_WORK}"
	printf '\nautoCode test stage: %s\n' "$1"
}

writeTags()
{
	FILE_TAGS=$1
	: > "${FILE_TAGS}"
	for VAL_TAG in threads_alloc drivers_alloc error_enum error_catalog hal_define hal_init \
		hal_fxinit modules_count modules_list gpio_signals
	do
		printf '%s\n%s\n%s\n' "// [autoCode_tag] ${VAL_TAG}" \
			"stale generated data" "// [/tag]" >> "${FILE_TAGS}"
	done
}

writeConfig()
{
	printf '%s\n' \
		"--error_count 1000" \
		"--errors ${PATH_CASE}/errors.list" \
		"--initrc ${PATH_CASE}/initrc.list" \
		"--parsetag ${PATH_CASE}/tags.list" \
		"--halinit ${PATH_CASE}/hal_init.list" \
		"--funcinit ${PATH_CASE}/func_init.list" \
		"--haldefine ${PATH_CASE}/hal_define.list" \
		"--gpio_signals ${PATH_CASE}/signals.gpio" > "${PATH_CASE}/autoCode.conf"
}

caseBegin()
{
	PATH_CASE="${PATH_STAGE_WORK}/$1"
	mkdir -p "${PATH_CASE}" || fail "cannot create ${PATH_CASE}"
	printf '%s\n' 'ERR_TEST "" FLOW' > "${PATH_CASE}/errors.err"
	printf '%s\n' "${PATH_CASE}/errors.err" > "${PATH_CASE}/errors.list"
	writeInitrcVersion > "${PATH_CASE}/init.rc"
	printf '%s\n' 'system -type service -run core' >> "${PATH_CASE}/init.rc"
	printf '%s\n' "${PATH_CASE}/init.rc" > "${PATH_CASE}/initrc.list"
	writeTags "${PATH_CASE}/tags.c"
	printf '%s\n' "${PATH_CASE}/tags.c" > "${PATH_CASE}/tags.list"
	: > "${PATH_CASE}/hal_init.list"
	printf '%s\n' 'hal_avr8Init' 'hal_atmega2560Init' 'hal_arduinoMegaInit' > "${PATH_CASE}/func_init.list"
	: > "${PATH_CASE}/hal_define.list"
	printf '%s\n' 'GPIO_SIGNAL_TEST' > "${PATH_CASE}/signals.gpio"
	writeConfig
}

logContains()
{
	if ! grep -F -q -- "$2" "${PATH_STAGE_WORK}/$1.log"; then
		fail "$1: missing diagnostic <$2>"
	fi
}

expectFailure()
{
	VAL_NAME=$1
	VAL_PATTERN=$2
	shift 2
	if "$@" > "${PATH_STAGE_WORK}/${VAL_NAME}.log" 2>&1; then
		fail "${VAL_NAME}: command unexpectedly succeeded"
	fi
	logContains "${VAL_NAME}" "${VAL_PATTERN}"
	VAL_TEST_COUNT=$((VAL_TEST_COUNT + 1))
}

expectSuccess()
{
	VAL_NAME=$1
	shift
	if ! "$@" > "${PATH_STAGE_WORK}/${VAL_NAME}.log" 2>&1; then
		fail "${VAL_NAME}: command failed"
	fi
	VAL_TEST_COUNT=$((VAL_TEST_COUNT + 1))
}

assertNoTemporaryFiles()
{
	if find "${PATH_STAGE_WORK}" -type f -name '*.tmp' -print | grep -q .; then
		fail "temporary autoCode files remain in ${PATH_STAGE_WORK}"
	fi
}

runCommandLineTests()
{
	stageBegin command_line
	expectFailure no_argument "autoCode bad argc" "${FILE_AUTOCODE}"
	expectFailure too_many_arguments "autoCode bad argc" \
		"${FILE_AUTOCODE}" unused.conf extra.conf
}

runOptionFailure()
{
	expectFailure "$1" "$2" "${FILE_AUTOCODE}" "${PATH_CASE}/autoCode.conf"
}

runOptionTests()
{
	stageBegin options
	expectFailure missing_configuration "opening file" \
		"${FILE_AUTOCODE}" "${PATH_STAGE_WORK}/missing.conf"

	caseBegin wrong_token_count
	printf '%s\n' '--error_count' > "${PATH_CASE}/autoCode.conf"
	runOptionFailure wrong_token_count "wrong token count"

	caseBegin unknown_option
	printf '%s\n' '--unknown value' > "${PATH_CASE}/autoCode.conf"
	runOptionFailure unknown_option "unknown option"

	caseBegin all_required_missing
	: > "${PATH_CASE}/autoCode.conf"
	runOptionFailure all_required_missing \
		"required autoCode option --error_count is not set"
	logContains all_required_missing "required autoCode option --gpio_signals is not set"

	caseBegin all_required_duplicate
	cp "${PATH_CASE}/autoCode.conf" "${PATH_CASE}/duplicate.conf"
	cat "${PATH_CASE}/duplicate.conf" >> "${PATH_CASE}/autoCode.conf"
	runOptionFailure all_required_duplicate \
		"required autoCode option --error_count is multiple set"
	logContains all_required_duplicate "required autoCode option --gpio_signals is multiple set"

	for VAL_VALUE in -1 invalid 1x 4294967296
	do
		VAL_NAME=$(printf '%s' "${VAL_VALUE}" | tr -c '[:alnum:]' '_')
		caseBegin "invalid_error_count_${VAL_NAME}"
		sed "s/--error_count 1000/--error_count ${VAL_VALUE}/" \
			"${PATH_CASE}/autoCode.conf" > "${PATH_CASE}/changed.conf"
		mv "${PATH_CASE}/changed.conf" "${PATH_CASE}/autoCode.conf"
		runOptionFailure "invalid_error_count_${VAL_NAME}" "invalid --error_count value"
	done

	caseBegin unterminated_option
	printf '%s\n' '--error_count "1000' > "${PATH_CASE}/autoCode.conf"
	runOptionFailure unterminated_option "unterminated string"

	caseBegin long_option_line
	awk 'BEGIN { printf "--errors "; for (i = 0; i < 260; i++) printf "x"; \
		printf "\n" }' > "${PATH_CASE}/autoCode.conf"
	runOptionFailure long_option_line "reading file"
}

runErrorTests()
{
	stageBegin errors
	caseBegin missing_error_list
	sed "s|${PATH_CASE}/errors.list|${PATH_CASE}/missing.list|" \
		"${PATH_CASE}/autoCode.conf" > "${PATH_CASE}/changed.conf"
	mv "${PATH_CASE}/changed.conf" "${PATH_CASE}/autoCode.conf"
	expectFailure missing_error_list "opening file" \
		"${FILE_AUTOCODE}" "${PATH_CASE}/autoCode.conf"

	caseBegin missing_error_file
	printf '%s\n' "${PATH_CASE}/missing.err" > "${PATH_CASE}/errors.list"
	expectFailure missing_error_file "opening file" \
		"${FILE_AUTOCODE}" "${PATH_CASE}/autoCode.conf"

	caseBegin malformed_errors
	printf '%s\n' \
		'ERR_FIELDS "message"' \
		'ERR_DUPLICATE "first" WARN' \
		'ERR_DUPLICATE "second" WARN' \
		'ERR_FLOW_MESSAGE "must be empty" FLOW' \
		'ERR_LEVEL "message" INVALID' \
		'ERR_UNTERMINATED "message WARN' > "${PATH_CASE}/errors.err"
	VAL_INDEX=0
	while [ "${VAL_INDEX}" -le 256 ]
	do
		printf 'ERR_LIMIT_%03d "message" WARN\n' "${VAL_INDEX}" \
			>> "${PATH_CASE}/errors.err"
		VAL_INDEX=$((VAL_INDEX + 1))
	done
	expectFailure malformed_errors "wrong token count != 3" \
		"${FILE_AUTOCODE}" "${PATH_CASE}/autoCode.conf"
	for VAL_PATTERN in "Duplicate error name" "FLOW error message must be empty" \
		"wrong error level argument" "unterminated string" "Too many errors"
	do
		logContains malformed_errors "${VAL_PATTERN}"
	done

	caseBegin long_error_line
	awk 'BEGIN { printf "ERR_LONG \""; for (i = 0; i < 260; i++) printf "x"; \
		printf "\" WARN\n" }' > "${PATH_CASE}/errors.err"
	expectFailure long_error_line "reading file" \
		"${FILE_AUTOCODE}" "${PATH_CASE}/autoCode.conf"

	caseBegin long_error_list_line
	awk 'BEGIN { for (i = 0; i < 260; i++) printf "x"; printf "\n" }' \
		> "${PATH_CASE}/errors.list"
	expectFailure long_error_list_line "reading file" \
		"${FILE_AUTOCODE}" "${PATH_CASE}/autoCode.conf"
}

runInitrcTests()
{
	stageBegin initrc
	caseBegin missing_initrc_list
	sed "s|${PATH_CASE}/initrc.list|${PATH_CASE}/missing.list|" \
		"${PATH_CASE}/autoCode.conf" > "${PATH_CASE}/changed.conf"
	mv "${PATH_CASE}/changed.conf" "${PATH_CASE}/autoCode.conf"
	expectFailure missing_initrc_list "opening file" \
		"${FILE_AUTOCODE}" "${PATH_CASE}/autoCode.conf"

	caseBegin missing_initrc_file
	printf '%s\n' "${PATH_CASE}/missing.rc" > "${PATH_CASE}/initrc.list"
	expectFailure missing_initrc_file "opening file" \
		"${FILE_AUTOCODE}" "${PATH_CASE}/autoCode.conf"

	caseBegin missing_initrc_version
	printf '%s\n' 'system -type service -run core' > "${PATH_CASE}/init.rc"
	expectFailure missing_initrc_version "first init.rc line" \
		"${FILE_AUTOCODE}" "${PATH_CASE}/autoCode.conf"

	caseBegin empty_initrc
	: > "${PATH_CASE}/init.rc"
	expectFailure empty_initrc "missing !set_version_major 1" \
		"${FILE_AUTOCODE}" "${PATH_CASE}/autoCode.conf"

	caseBegin comment_before_initrc_version
	printf '%s\n' '# version must be the physical file header' > "${PATH_CASE}/init.rc"
	writeInitrcVersion >> "${PATH_CASE}/init.rc"
	expectFailure comment_before_initrc_version "first init.rc line" \
		"${FILE_AUTOCODE}" "${PATH_CASE}/autoCode.conf"

	caseBegin missing_minor_initrc_version
	printf '%s\n' '!set_version_major 1' > "${PATH_CASE}/init.rc"
	expectFailure missing_minor_initrc_version "missing !set_version_minor 3" \
		"${FILE_AUTOCODE}" "${PATH_CASE}/autoCode.conf"

	caseBegin module_before_minor_initrc_version
	printf '%s\n' '!set_version_major 1' 'system -type service -run core' \
		> "${PATH_CASE}/init.rc"
	expectFailure module_before_minor_initrc_version "second init.rc line" \
		"${FILE_AUTOCODE}" "${PATH_CASE}/autoCode.conf"

	caseBegin wrong_major_initrc_version
	printf '%s\n' '!set_version_major 0' '!set_version_minor 3' \
		> "${PATH_CASE}/init.rc"
	expectFailure wrong_major_initrc_version "unsupported init.rc major syntax version" \
		"${FILE_AUTOCODE}" "${PATH_CASE}/autoCode.conf"

	caseBegin wrong_minor_initrc_version
	printf '%s\n' '!set_version_major 1' '!set_version_minor 2' \
		> "${PATH_CASE}/init.rc"
	expectFailure wrong_minor_initrc_version "unsupported init.rc minor syntax version" \
		"${FILE_AUTOCODE}" "${PATH_CASE}/autoCode.conf"

	caseBegin wrong_initrc_version_order
	printf '%s\n' '!set_version_minor 3' '!set_version_major 1' \
		> "${PATH_CASE}/init.rc"
	expectFailure wrong_initrc_version_order "first init.rc line" \
		"${FILE_AUTOCODE}" "${PATH_CASE}/autoCode.conf"

	caseBegin late_initrc_version
	writeInitrcVersion > "${PATH_CASE}/init.rc"
	printf '%s\n' 'system -type service -run core' '!set_version_minor 3' \
		>> "${PATH_CASE}/init.rc"
	expectFailure late_initrc_version "init.rc version command outside header" \
		"${FILE_AUTOCODE}" "${PATH_CASE}/autoCode.conf"

	caseBegin malformed_initrc
	writeInitrcVersion > "${PATH_CASE}/init.rc"
	printf '%s\n' \
		'system -type service -run core' \
		'few -type user' \
		'badcmd -bad value -run user' \
		'badrun -type user -run invalid' \
		'badtype -type invalid -run user' \
		'badi2c -type driver -run core -i2c invalid' \
		'abcdefghijklmnopqrstuvwxyzabcdef -type user -run user' \
		'duplicate -type user -run user' \
		'duplicate -type user -run user' \
		'no_run -type user -type user' \
		'no_type -run user -run user' \
		'i2c_multi -type driver -i2c 1 -i2c 2' \
		'i2c_thread -type user -run user -i2c 1' \
		'unterminated -type "user -run user' >> "${PATH_CASE}/init.rc"
	VAL_INDEX=0
	while [ "${VAL_INDEX}" -le 256 ]
	do
		printf 'driver_%03d -type driver -run core\n' "${VAL_INDEX}" \
			>> "${PATH_CASE}/init.rc"
		VAL_INDEX=$((VAL_INDEX + 1))
	done
	expectFailure malformed_initrc "wrong token count" \
		"${FILE_AUTOCODE}" "${PATH_CASE}/autoCode.conf"
	for VAL_PATTERN in "unknown command" "unknown data" "Name too long" "duplicate name" \
		"-run option is not set" "-run option is multiple set" \
		"-type option is not set" "-type option is multiple set" \
		"-i2c option is multiple set" "-i2c option is only valid for drivers" \
		"too much modules" "unterminated string"
	do
		logContains malformed_initrc "${VAL_PATTERN}"
	done

	caseBegin long_initrc_line
	awk 'BEGIN { for (i = 0; i < 260; i++) printf "x"; printf "\n" }' \
		> "${PATH_CASE}/init.rc"
	expectFailure long_initrc_line "reading file" \
		"${FILE_AUTOCODE}" "${PATH_CASE}/autoCode.conf"

	caseBegin long_initrc_list_line
	awk 'BEGIN { for (i = 0; i < 260; i++) printf "x"; printf "\n" }' \
		> "${PATH_CASE}/initrc.list"
	expectFailure long_initrc_list_line "reading file" \
		"${FILE_AUTOCODE}" "${PATH_CASE}/autoCode.conf"
}

runTagCase()
{
	expectFailure "$1" "$2" "${FILE_AUTOCODE}" "${PATH_CASE}/autoCode.conf"
	assertNoTemporaryFiles
}

runParseTagTests()
{
	stageBegin parse_tag
	caseBegin missing_tag_list
	sed "s|${PATH_CASE}/tags.list|${PATH_CASE}/missing.list|" \
		"${PATH_CASE}/autoCode.conf" > "${PATH_CASE}/changed.conf"
	mv "${PATH_CASE}/changed.conf" "${PATH_CASE}/autoCode.conf"
	runTagCase missing_tag_list "opening file"

	caseBegin missing_tag_file
	printf '%s\n' "${PATH_CASE}/missing.c" > "${PATH_CASE}/tags.list"
	runTagCase missing_tag_file "opening file"

	caseBegin malformed_tag
	printf '%s\n' '// [autoCode_tag]' > "${PATH_CASE}/tags.c"
	runTagCase malformed_tag "token count != 3"

	caseBegin nested_tag
	printf '%s\n' '// [autoCode_tag] threads_alloc' \
		'// [autoCode_tag] drivers_alloc' '// [/tag]' > "${PATH_CASE}/tags.c"
	runTagCase nested_tag "Start new tag section without previous end tag"

	caseBegin missing_end_tag
	printf '%s\n' '// [autoCode_tag] threads_alloc' > "${PATH_CASE}/tags.c"
	runTagCase missing_end_tag "missing end tag"

	caseBegin unknown_tag
	writeTags "${PATH_CASE}/tags.c"
	printf '%s\n' '// [autoCode_tag] unknown' '// [/tag]' >> "${PATH_CASE}/tags.c"
	runTagCase unknown_tag "unknown tag"

	caseBegin missing_required_tags
	printf '%s\n' 'no autoCode tags' > "${PATH_CASE}/tags.c"
	runTagCase missing_required_tags "required autoCode tag threads_alloc is not set"

	caseBegin duplicate_tag
	writeTags "${PATH_CASE}/tags.c"
	printf '%s\n' '// [autoCode_tag] error_enum' '// [/tag]' >> "${PATH_CASE}/tags.c"
	runTagCase duplicate_tag "required autoCode tag error_enum is multiple set"

	for VAL_INPUT in hal_define.list hal_init.list func_init.list signals.gpio
	do
		VAL_NAME=$(printf '%s' "${VAL_INPUT}" | tr '.' '_')
		caseBegin "missing_${VAL_NAME}"
		find "${PATH_CASE}" -depth -name "${VAL_INPUT}" -delete
		runTagCase "missing_${VAL_NAME}" "opening file"
	done

	caseBegin invalid_gpio
	printf '%s\n' 'GPIO_SIGNAL_TEST extra' > "${PATH_CASE}/signals.gpio"
	runTagCase invalid_gpio "wrong token count"

	caseBegin invalid_func_init
	printf '%s\n' 'hal_avr8Init extra' > "${PATH_CASE}/func_init.list"
	runTagCase invalid_func_init "wrong token count"

	caseBegin missing_system_thread
	writeInitrcVersion > "${PATH_CASE}/init.rc"
	printf '%s\n' 'task -type user -run user' >> "${PATH_CASE}/init.rc"
	runTagCase missing_system_thread "thread system was not found"

	caseBegin unterminated_tag_line
	printf '%s\n' '"unterminated' > "${PATH_CASE}/tags.c"
	for VAL_TAG in threads_alloc drivers_alloc error_enum error_catalog hal_define hal_init \
		hal_fxinit modules_count modules_list gpio_signals
	do
		printf '%s\n%s\n' "// [autoCode_tag] ${VAL_TAG}" "// [/tag]" \
			>> "${PATH_CASE}/tags.c"
	done
	runTagCase unterminated_tag_line "unterminated string"

	for VAL_INPUT in tags.c hal_define.list hal_init.list func_init.list signals.gpio
	do
		VAL_NAME=$(printf '%s' "${VAL_INPUT}" | tr '.' '_')
		caseBegin "long_${VAL_NAME}"
		awk 'BEGIN { for (i = 0; i < 260; i++) printf "x"; printf "\n" }' \
			> "${PATH_CASE}/${VAL_INPUT}"
		runTagCase "long_${VAL_NAME}" "reading file"
	done
}

runCompareReplaceTests()
{
	stageBegin compare_replace
	caseBegin stable_generation
	expectSuccess initial_generation "${FILE_AUTOCODE}" "${PATH_CASE}/autoCode.conf"
	expectSuccess unchanged_generation "${FILE_AUTOCODE}" "${PATH_CASE}/autoCode.conf"
	logContains unchanged_generation "0 updated, 1 unchanged"
	for VAL_FUNC in hal_avr8Init hal_atmega2560Init hal_arduinoMegaInit
	do
		if ! grep -F -q -- "${VAL_FUNC}();" "${PATH_CASE}/tags.c"; then
			fail "generated init call missing for ${VAL_FUNC}"
		fi
	done

	sed 's/#define MOD_DRIVER_COUNT 0/#define MOD_DRIVER_COUNT 99/' \
		"${PATH_CASE}/tags.c" > "${PATH_CASE}/changed.c"
	mv "${PATH_CASE}/changed.c" "${PATH_CASE}/tags.c"
	expectSuccess changed_generation "${FILE_AUTOCODE}" "${PATH_CASE}/autoCode.conf"
	logContains changed_generation "1 updated, 0 unchanged"
	if grep -q '#define MOD_DRIVER_COUNT 99' "${PATH_CASE}/tags.c"; then
		fail "changed generation did not restore generated content"
	fi
	assertNoTemporaryFiles

	caseBegin atomic_failure
	printf '%s\n' '// [autoCode_tag] threads_alloc' 'ORIGINAL_SENTINEL' '// [/tag]' \
		> "${PATH_CASE}/first.c"
	: > "${PATH_CASE}/second.c"
	for VAL_TAG in drivers_alloc error_enum error_catalog hal_define hal_init hal_fxinit \
		modules_count modules_list
	do
		printf '%s\n%s\n' "// [autoCode_tag] ${VAL_TAG}" "// [/tag]" \
			>> "${PATH_CASE}/second.c"
	done
	printf '%s\n' '// [autoCode_tag] gpio_signals' >> "${PATH_CASE}/second.c"
	printf '%s\n%s\n' "${PATH_CASE}/first.c" "${PATH_CASE}/second.c" \
		> "${PATH_CASE}/tags.list"
	cp "${PATH_CASE}/first.c" "${PATH_CASE}/first.expected"
	runTagCase atomic_failure "missing end tag"
	if ! cmp -s "${PATH_CASE}/first.expected" "${PATH_CASE}/first.c"; then
		fail "a failed generation modified an earlier destination"
	fi
}

runStage()
{
	case "$1" in
		command_line) runCommandLineTests ;;
		options) runOptionTests ;;
		errors) runErrorTests ;;
		initrc) runInitrcTests ;;
		parse_tag) runParseTagTests ;;
		compare_replace) runCompareReplaceTests ;;
		*) fail "unknown stage <$1>" ;;
	esac
}

if [ ! -x "${FILE_AUTOCODE}" ]; then
	fail "autoCode executable not found: ${FILE_AUTOCODE}"
fi

if [ "${VAL_STAGE}" = all ]; then
	for VAL_SELECTED_STAGE in command_line options errors initrc parse_tag compare_replace
	do
		runStage "${VAL_SELECTED_STAGE}"
	done
else
	runStage "${VAL_STAGE}"
fi

printf 'autoCode tests passed: %s case(s)\n' "${VAL_TEST_COUNT}"
