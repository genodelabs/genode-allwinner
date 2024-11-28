DRIVER  := de
TARGET   = $(DRIVER)_fb
REQUIRES = arm_v8a
LIBS     = base blit a64_lx_emul
SRC_CC  += main.cc
SRC_CC  += lx_emul/pin.cc
SRC_CC  += lx_emul/random_dummy.cc
SRC_C   += fb.c lx_user.c clock.c reset.c
SRC_C   += lx_emul/a64/pio.c
SRC_C   += lx_emul/a64/pmic.c
SRC_C   += lx_emul/a64/r_intc.c
SRC_C   += lx_emul/a64/r_pio.c
SRC_C   += lx_emul/a64/common_dummies.c

vpath lx_emul/a64/pio.c            $(REP_DIR)/src/lib
vpath lx_emul/a64/pmic.c           $(REP_DIR)/src/lib
vpath lx_emul/a64/r_intc.c         $(REP_DIR)/src/lib
vpath lx_emul/a64/r_pio.c          $(REP_DIR)/src/lib
vpath lx_emul/a64/sched.c          $(REP_DIR)/src/lib
vpath lx_emul/a64/common_dummies.c $(REP_DIR)/src/lib

BOARDS := pinephone

DTS_EXTRACT(pinephone) := --select /backlight --select de --select dsi
