CUSTOM_TARGET_DEPS := scp.bin app.f

SHELL := /bin/bash

SCP_DIR := $(REP_DIR)/src/scp

SCP_LD_SCRIPT := $(SCP_DIR)/scp.ld

vpath % $(SCP_DIR)

# strip comments from app.f
app.f: pinephone/app.f
	$(MSG_CONVERT)$@
	$(VERBOSE)grep -v "^\\\\" $< > $@

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
