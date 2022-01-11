TARGET   = a64_platform_drv
REQUIRES = arm_v8
SRC_CC  += device.cc
SRC_CC  += device_component.cc
SRC_CC  += device_model_policy.cc
SRC_CC  += main.cc
SRC_CC  += session_component.cc
SRC_CC  += root.cc
LIBS     = base

#
# Determine location of generic platform-driver code.
# (either in os repository or in the a64_platform_drv src archive)
#
GENERIC_DIR := $(dir $(call select_from_repositories,src/drivers/platform/clock.h))

INC_DIR = $(PRG_DIR) $(GENERIC_DIR)

vpath main.cc $(PRG_DIR)
vpath %.cc $(GENERIC_DIR)
