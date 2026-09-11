################################################################################
#
# TaskMate Project
# (c) 2026 PRADERE Sebastien
#
# This file is part of TaskMate and is distributed under the BSD-2-Clause License.
# See the LICENSE file for full license terms.
#
################################################################################

# Output files
FILE_HEX = ${FILE_TARGET}.hex
FILE_ELF = ${FILE_TARGET}.elf
FILE_AVR8_PROGRAMS_LIST = ${PATH_AVR8}/programs.list
FILE_AVR8_PROGRAMS_CHECK_STAMP = ${PATH_BUILD_TARGET}/.avr8_programs_check_stamp
FILE_AVR8_MEMORY_DATA_SCRIPT = ${PATH_AVR8}/avr_memory_data.awk
FILE_AVR8_MEMORY_SHOW_SCRIPT = ${PATH_AVR8}/avr_memory_show.awk

.BEGIN: ${FILE_AVR8_PROGRAMS_CHECK_STAMP}
	
# Check AVR8 required programs once
${FILE_AVR8_PROGRAMS_CHECK_STAMP}: ${FILE_AVR8_PROGRAMS_LIST} ${FILE_PROGRAMS_CHECK_SCRIPT}
	@printf "%sChecking AVR8 programs ...%s\n" \
		"${COLOUR_TARGET_INFO}" "${COLOUR_RESET}"
	@${FILE_PROGRAMS_CHECK_SCRIPT} "${FILE_AVR8_PROGRAMS_LIST}"
	@mkdir -p "${PATH_BUILD_TARGET}"
	@touch "${FILE_AVR8_PROGRAMS_CHECK_STAMP}"

# Link
${FILE_TARGET}: ${FILES_OBJ}
	@printf "\n%sLinking%s\n\n" \
		"${COLOUR_TARGET_INFO}" "${COLOUR_RESET}"
	@${CC} ${CFLAGS} ${LDFLAGS} -o ${FILE_ELF} ${FILES_OBJ}
	@printf "\t *.o -> ${FILE_ELF}\n"

# Compiler wrapper
FILE_COMPILE_SRC = ${.TARGET:${PATH_BUILD_TARGET}/%.o=%.c}
# Compile
${FILES_OBJ}: ${FILE_COMPILE_SRC}
	@printf "\n%sCompilation ...%s\n\n" \
	    "${COLOUR_TARGET_INFO}" "${COLOUR_RESET}"
	@printf "source : <%s> -> <%s>\n" \
	    "${FILE_COMPILE_SRC}" "${.TARGET}"
	@mkdir -p "${.TARGET:H}"
	@${CC} ${CFLAGS} ${CFLAGS_${FILE_COMPILE_SRC}} \
	    -c "${FILE_COMPILE_SRC}" -o "${.TARGET}"

# HEX file generation
${FILE_HEX}: ${FILE_ELF}
	@avr-objcopy -O ihex -R .eeprom ${FILE_ELF} ${FILE_HEX}
	
upload: all _mcu_memory_show ${FILE_HEX}
#help [avr8] Upload firmware to the MCU via the Arduino board.
	@printf "\n%sUpload binary to AVR flash, build %i %s\n\n" \
		"${COLOUR_TARGET_INFO}" ${VAL_BUILD_CNT} "${COLOUR_RESET}"
	avrdude -c ${VAL_PROGRAMMER} -p ${VAL_MCU_SERIAL} -U flash:w:${FILE_HEX}:i -P ${VAL_PROGRAMMER_PORT} -D
.PHONY: upload

# Memory usage
_mcu_memory_data:
		@avr-size -G -d ${FILE_ELF} > ${FILE_MEMRAW}

		@awk -v flash_total_k="${VAL_FLASH_SIZE_K}" -v ram_total_k="${VAL_RAM_SIZE_K}" \
			-v output_file="${FILE_MEMDATA}" -f ${FILE_AVR8_MEMORY_DATA_SCRIPT} \
			"${FILE_MEMRAW}"
.PHONY: _mcu_memory_data

_mcu_memory_show: _mcu_memory_data
	@printf "\nStatic memory usage : \n"
	@cat ${FILE_MEMRAW}

	@printf "${COLOUR_WHITE_BOLD}Memory usage :\n"
	@awk -f ${FILE_AVR8_MEMORY_SHOW_SCRIPT} "${FILE_MEMDATA}"
	@printf "${COLOUR_RESET}"
.PHONY: _mcu_memory_show


dump: all ${FILE_HEX}
#help [avr8] Disassemble machine code in .hex and .elf
	@printf "\n%sGenerate debugging informations%s\n\n" \
		"${COLOUR_TARGET_INFO}" "${COLOUR_RESET}"
	avr-objdump -D -m ${VAL_MCU_ARCHTYPE} ${FILE_HEX} > "${PATH_BUILD_TARGET}/hex.txt"
	avr-objdump -D -m ${VAL_MCU_ARCHTYPE} ${FILE_ELF} > "${PATH_BUILD_TARGET}/elf.txt"
.PHONY: dump

modules_size: all
#help [avr8] List module sizes from largest to smallest.
	@printf "\n%sList module size%s\n\n" \
		"${COLOUR_TARGET_INFO}" "${COLOUR_RESET}"
	avr-size -G -d ${PATH_BUILD_TARGET}/TaskMate.elf
	avr-nm --format=bsd --size-sort -r ${PATH_BUILD_TARGET}/TaskMate.elf | head -20
.PHONY: modules_size
