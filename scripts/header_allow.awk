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
# Parse and check allowed system includes
################################################################################

BEGIN {
	state = "outside"
	block_id = 0
}

(NF == 0) || (/^[[:space:]]*#/) { next }

{ line = $0 }


state == "outside" {
	if (line ~ /^[ \t]*\{[ \t]*$/)
	{
		block_id++
		state = "in_block"
		block_has_source_file[block_id] = 0
		block_has_allow[block_id] = 0
		next
	}

	error("unexpected content outside block")
}

state == "in_block" {
	if (line ~ /^[ \t]*\}[ \t]*$/)
	{
		if (!block_has_source_file[block_id])
			error("missing source_file in block")
		if (!block_has_allow[block_id])
			error("missing allow block")
		state = "outside"
		next
	}

	if (line ~ /^[ \t]*source_file[ \t]+[^ \t{}][^{}]*$/)
	{
		if (block_has_source_file[block_id])
			error("duplicate source_file in block")

		file = line
		sub(/^[ \t]*source_file[ \t]+/, "", file)
		sub(/[ \t]+$/, "", file)

		source_file[block_id] = file
		block_has_source_file[block_id] = 1
		next
	}

	if (line ~ /^[ \t]*allow[ \t]*\{[ \t]*$/)
	{
		if (block_has_allow[block_id])
			error("duplicate allow block")

		state = "in_allow"
		block_has_allow[block_id] = 1
		next
	}

	error("invalid statement inside block <" $1 ">")
}

state == "in_allow" {
	if (line ~ /^[ \t]*\}[ \t]*$/)
	{
		state = "in_block"
		next
	}

	if (line ~ /^[ \t]*\{[ \t]*$/)
		error("unexpected '{' inside allow block")

	file = line
	sub(/^[ \t]+/, "", file)
	sub(/[ \t]+$/, "", file)

	if (file == "") error("empty entry in allow block")

	allow[block_id, PATH_SOURCES "/" file] = 1
	allow_count[block_id]++
	next
}

END {
	if (failed) exit 1

	if (state == "in_block")
		error_end("missing closing '}' for block")

	if (state == "in_allow")
		error_end("missing closing '}' for allow block")

	for (i = 1; i <= block_id; i++)
	{
		if (!block_has_source_file[i])
			error_end("block " i ": missing source_file")
		if (!block_has_allow[i])
			error_end("block " i ": missing allow block")
		if (allow_count[i] == 0)
			error_end("block " i ": empty allow block")
	}

	scan_failed = 0
	print "Headers allow check report" > h_check_log
	for (i = 1; i <= block_id; i++)
	{
		check_source_file(i)
	}
	if (scan_failed) exit 3
}

function error(msg) {
	failed = 1
	printf("header_allow.awk [%s:%d] parse error : %s\n", FILENAME, NR, msg)
	exit 1
}

function error_end(msg) {
	printf("header_allow.awk [%s] parse error: %s\n",FILENAME, msg)
	exit 2
}

function trim(t)
{
	sub(/^[ \t]+/, "", t)
	sub(/[ \t]+$/, "", t)
	return t
}

function check_source_file(block, cmd, file, found_any)
{
	found_any = 0

	printf("Checking source file %s ...\n", source_file[block])
	printf("\nChecking source file %s ...\n", source_file[block]) > h_check_log

	cmd = "grep -R -l \"" source_file[block] "\" \"" PATH_SOURCES "\" 2>/dev/null"

	while ((cmd | getline file) > 0)
	{
		found_any = 1
		file = trim(file)

		if ((block, file) in allow)
		{
			printf("[  OK  ] %s\n", file) > h_check_log
		}
		else
		{
			printf("[%s FAIL %s] Forbidden include detected in: %s\n", COLOUR_FAIL, COLOUR_RESET, file)
			printf("[ >>>FAIL<<< ] Forbidden include detected in: %s\n", file) > h_check_log
			scan_failed = 1
		}
	}
	close(cmd)

	if (!found_any)
		printf("[ INFO ] No file matched source file: %s\n", source_file[block])
}
