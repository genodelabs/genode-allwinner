CUSTOM_TARGET_DEPS := scp.bin

SHELL := /bin/bash

scp.bin:
	$(VERBOSE)ln -s /dev/null $@
