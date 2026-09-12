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
# Validate direct includes against the architecture matrix
################################################################################

BEGIN {
	if (matrix_file == "") fatal("missing matrix_file variable")
	if (path_sources == "") path_sources = "srcs"
}

FILENAME == matrix_file {
	parseMatrixLine($0)
	next
}

!matrix_ready {
	validateMatrix()
	matrix_ready = 1
}

FNR == 1 { source_files++ }

{ checkInclude($0) }

END {
	if (fatal_error) exit 2
	if (!matrix_ready) validateMatrix()
	if (fatal_error) exit 2
	if (violations != 0)
	{
		printf("Architecture include check failed: %d architecture violation(s)\n", violations)
		exit 3
	}
	printf("Architecture include check passed: %d direct include(s) in %d file(s)\n",
		direct_includes, source_files)
}

function parseMatrixLine(line, cells, count, first, i, value)
{
	if (line !~ /^[ \t]*\|/) return
	count = split(line, cells, "|")
	if (count < 4)
	{
		fatalMatrix("invalid table row")
		return
	}
	first = trim(cells[2])
	if (first ~ /^-+$/) return
	if (matrix_columns == 0)
	{
		for (i = 3; i < count; i++)
		{
			value = trim(cells[i])
			if (value == "")
			{
				fatalMatrix("empty callee layer")
				return
			}
			callee[++matrix_columns] = value
		}
		return
	}
	if ((count - 3) != matrix_columns)
	{
		fatalMatrix("layer row has an invalid column count")
		return
	}
	if (first == "")
	{
		fatalMatrix("empty caller layer")
		return
	}
	if (first in caller_seen)
	{
		fatalMatrix("duplicate caller layer " first)
		return
	}
	caller_seen[first] = 1
	matrix_rows++
	for (i = 1; i <= matrix_columns; i++)
	{
		value = trim(cells[i + 2])
		if ((value != "Y") && (value != "N"))
		{
			fatalMatrix("invalid value " value " for " first " -> " callee[i])
			return
		}
		allowed[first, callee[i]] = (value == "Y")
	}
}

function validateMatrix(i)
{
	if (matrix_columns == 0) fatal("architecture matrix has no callee layer")
	if (matrix_rows == 0) fatal("architecture matrix has no caller layer")
	for (i = 1; i <= matrix_columns; i++)
	{
		if (!(callee[i] in caller_seen)) fatal("architecture matrix has no row for " callee[i])
	}
}

function checkInclude(line, include_path, caller_path, caller_layer, callee_layer)
{
	if (line !~ /^[ \t]*#[ \t]*include([ \t]|$)/) return

	caller_path = stripSourceRoot(FILENAME)
	caller_layer = pathLayer(caller_path)
	direct_includes++
	if (caller_layer == "") return

	if (line !~ /^[ \t]*#[ \t]*include[ \t]*[<"][^>"]+[>"]/)
	{
		printf("%s:%d: unverifiable direct include in %s: %s\n", FILENAME, FNR,
			caller_layer, trim(line))
		violations++
		return
	}

	include_path = line
	sub(/^[ \t]*#[ \t]*include[ \t]*[<"]/, "", include_path)
	sub(/[>"].*$/, "", include_path)
	include_path = resolveInclude(caller_path, include_path)
	callee_layer = pathLayer(include_path)
	if (callee_layer == "") return

	if (!((caller_layer, callee_layer) in allowed))
	{
		fatal("matrix has no rule for " caller_layer " -> " callee_layer)
		return
	}
	if (!allowed[caller_layer, callee_layer])
	{
		printf("%s:%d: forbidden direct include %s -> %s: %s\n", FILENAME, FNR,
			caller_layer, callee_layer, include_path)
		violations++
	}
}

function resolveInclude(caller_path, include_path, directory)
{
	include_path = stripSourceRoot(include_path)
	if (pathLayer(include_path) != "") return include_path
	directory = caller_path
	sub(/\/[^\/]*$/, "", directory)
	return normalizePath(directory "/" include_path)
}

function stripSourceRoot(path, root)
{
	path = normalizePath(path)
	root = normalizePath(path_sources)
	if (index(path, root "/") == 1) path = substr(path, length(root) + 2)
	else if (index(path, "srcs/") == 1) path = substr(path, 6)
	return path
}

function normalizePath(path, parts, stack, count, top, i, part, result)
{
	gsub(/\/\/+/, "/", path)
	count = split(path, parts, "/")
	for (i = 1; i <= count; i++)
	{
		part = parts[i]
		if ((part == "") || (part == ".")) continue
		if (part == "..")
		{
			if (top > 0) top--
			continue
		}
		stack[++top] = part
	}
	for (i = 1; i <= top; i++) result = result (result == "" ? "" : "/") stack[i]
	return result
}

function pathLayer(path)
{
	if (path ~ /^hal\//) return "hal"
	if (path ~ /^system\/sysCore\//) return "sysCore"
	if (path ~ /^system\/sysCall\//) return "sysCall"
	if (path ~ /^interfaces\//) return "interfaces"
	if (path ~ /^system\/services\//) return "services"
	if (path ~ /^user\/tasks\//) return "tasks"
	return ""
}

function trim(text)
{
	sub(/^[ \t]+/, "", text)
	sub(/[ \t]+$/, "", text)
	return text
}

function fatalMatrix(message) { fatal(sprintf("%s:%d: %s", FILENAME, FNR, message)) }

function fatal(message)
{
	if (!fatal_error) printf("arch_include.awk: %s\n", message)
	fatal_error = 1
}
