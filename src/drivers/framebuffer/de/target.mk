DRIVER  := de
TARGET   = $(DRIVER)_fb_drv
REQUIRES = arm_v8a
LIBS     = base blit a64_lx_emul
SRC_CC  += main.cc
SRC_C   += fb.c lx_user.c

BOARDS := pinephone

DTS_EXTRACT(pinephone) := --select /backlight --select de --select dsi
