TARGET   := pinephone_camera
DRIVER   := camera
REQUIRES := arm_v8a
LIBS     := base a64_lx_emul

INC_DIR := $(PRG_DIR)
INC_DIR += $(PRG_DIR)/include/lx_emul/shadow

SRC_C := lx_user.c
SRC_C += clock.c

SRC_C += yuv_rgb.c

SRC_C += lx_emul/a64/common_dummies.c
SRC_C += lx_emul/a64/pio.c
SRC_C += lx_emul/a64/pmic.c
SRC_C += lx_emul/a64/ccu.c
SRC_C += lx_emul/a64/r_pio.c
SRC_C += lx_emul/shadow/mm/page_alloc.c

SRC_CC += gui.cc
SRC_CC += lx_emul/pin.cc
SRC_CC += lx_emul/shared_dma_buffer.cc
SRC_CC += lx_emul/random_dummy.cc
SRC_CC += main.cc

CC_OPT_drivers/media/i2c/ov5640 += -Wno-unused-function

# MBUS address quirk
CC_OPT_drivers/media/common/videobuf2/videobuf2-dma-contig += -Ddma_alloc_attrs=quirk_dma_alloc_attrs

vpath lx_emul/a64/ccu.c            $(REP_DIR)/src/lib
vpath lx_emul/a64/common_dummies.c $(REP_DIR)/src/lib
vpath lx_emul/a64/pio.c            $(REP_DIR)/src/lib
vpath lx_emul/a64/pmic.c           $(REP_DIR)/src/lib
vpath lx_emul/a64/r_pio.c          $(REP_DIR)/src/lib
vpath lx_emul/a64/sched.c          $(REP_DIR)/src/lib

BOARDS := pinephone

DTS_EXTRACT(pinephone) := --select gc2145 --select ov5640
