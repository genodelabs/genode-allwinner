DRIVER  := emac
TARGET   = $(DRIVER)_nic_drv
REQUIRES = arm_v8a
LIBS     = base a64_lx_emul
SRC_CC  += main.cc
SRC_C    = lx_user.c pmic.c
SRC_C   += lx_emul/shadow/drivers/clk/clk.c
SRC_C   += lx_emul/shadow/drivers/clk/clkdev.c
SRC_C   += lx_emul/shadow/mm/page_alloc.c
SRC_C   += lx_emul/shadow/kernel/softirq.c
SRC_C   += lx_emul/a64/sched.c
SRC_C   += lx_emul/a64/common_dummies.c

vpath lx_emul/a64/sched.c          $(REP_DIR)/src/lib
vpath lx_emul/a64/common_dummies.c $(REP_DIR)/src/lib

SRC_CC  += genode_c_api/uplink.cc
GENODE_C_API_SRC_DIR := $(call select_from_repositories,src/lib/genode_c_api)
vpath % $(dir $(GENODE_C_API_SRC_DIR))

# for including 'net-sysfs.h'
CC_OPT_dummies += -I$(LX_SRC_DIR)/net/core

BOARDS := pine_a64lts

DTS_EXTRACT(pine_a64lts) := --select emac


