DRIVER  := lima
TARGET   = $(DRIVER)_gpu_drv
REQUIRES = arm_v8a
LIBS     = base a64_lx_emul
INC_DIR += $(PRG_DIR)/include/lx_emul/shadow
SRC_CC  += main.cc
SRC_CC  += emul.cc
SRC_CC  += lx_emul/random_dummy.cc
SRC_CC  += lx_emul/shared_dma_buffer.cc
SRC_C   += lx_emul.c
SRC_C   += lx_user.c
SRC_C   += clock.c
SRC_C   += lx_emul/a64/common_dummies.c

vpath lx_emul/a64/sched.c          $(REP_DIR)/src/lib
vpath lx_emul/a64/common_dummies.c $(REP_DIR)/src/lib

BOARDS := pine_a64lts pinephone

DTS_EXTRACT(pine_a64lts) := --select mali
DTS_EXTRACT(pinephone)   := --select mali
