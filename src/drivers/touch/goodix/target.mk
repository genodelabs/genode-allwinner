DRIVER  := goodix
TARGET   = $(DRIVER)_touch_drv
REQUIRES = arm_v8a
LIBS     = base a64_lx_emul
SRC_CC  += main.cc
SRC_C   += lx_user.c input.c
SRC_C   += lx_emul/a64/pio.c
SRC_C   += lx_emul/pin.c
SRC_C   += lx_emul/shadow/mm/page_alloc.c

vpath lx_emul/a64/pio.c $(REP_DIR)/src/lib

SRC_CC  += genode_c_api/event.cc
GENODE_C_API_SRC_DIR := $(call select_from_repositories,src/lib/genode_c_api)
vpath % $(dir $(GENODE_C_API_SRC_DIR))

BOARDS := pinephone

DTS_EXTRACT(pinephone) := --select /soc/i2c@1c2ac00/touchscreen@5d
