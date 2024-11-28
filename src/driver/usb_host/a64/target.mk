TARGET  := a64_usb_host
DRIVER  := usb
REQUIRES = arm_v8a
LIBS     = base a64_lx_emul

INC_DIR  = $(PRG_DIR)

SRC_C += lx_emul/usb.c
SRC_C += lx_emul/a64/common_dummies.c
SRC_C += lx_emul/shadow/drivers/clk/clk.c

SRC_CC += main.cc
SRC_CC += lx_emul/shared_dma_buffer.cc
SRC_CC += lx_emul/random_dummy.cc

vpath lx_emul/a64/common_dummies.c $(REP_DIR)/src/lib
vpath lx_emul/a64/sched.c          $(REP_DIR)/src/lib

#
# Genode C-API backends
#

SRC_CC  += genode_c_api/usb.cc
GENODE_C_API_SRC_DIR := $(call select_from_repositories,src/lib/genode_c_api)
vpath % $(dir $(GENODE_C_API_SRC_DIR))

BOARDS := pine_a64lts pinephone

DTS_EXTRACT(pine_a64lts)  := --select ehci1
DTS_EXTRACT(pinephone)    := --select ehci1
