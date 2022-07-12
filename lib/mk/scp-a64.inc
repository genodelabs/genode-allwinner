#
# The variable SRC_F must point to the board-specific Forth source codes
# (relative to the src/scp/ directory) before including this file. The files
# are concatenated into one app.f file. So their order is important.
#

CUSTOM_TARGET_DEPS := scp.bin

SHELL := /bin/bash

SCP_DIR := $(REP_DIR)/src/scp

SCP_LD_SCRIPT := $(SCP_DIR)/scp.ld

vpath % $(SCP_DIR)

# concatenate all SRC_F sources into one app.f file with comments stripped away
app.f: $(SRC_F)
	$(MSG_CONVERT)$@
	$(VERBOSE)cat $(addprefix $(SCP_DIR)/,$(SRC_F)) | grep -v "^\\\\" > $@

scp.o: scp.s
	$(MSG_ASSEM)$@
	$(VERBOSE)or1k-elf-as -I $(SCP_DIR)/ar100 -I. $< -o $@

scp.o: app.f ar100/uart.inc

scp.elf: scp.o
	$(MSG_LINK)$@
	$(VERBOSE)or1k-elf-ld -nostdlib -z max-page-size=0x10 -T$(SCP_LD_SCRIPT) $< -o $@

scp.elf: $(SCP_LD_SCRIPT)

scp.bin: scp.elf
	$(MSG_CONVERT)$@
	$(VERBOSE)or1k-elf-objcopy -Obinary --only-section=.main --reverse-bytes 4 $< $@

app.f scp.o scp.elf scp.bin : $(MAKEFILE_LIST)
