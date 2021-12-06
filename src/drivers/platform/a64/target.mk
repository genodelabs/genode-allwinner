TARGET   = a64_platform_drv
REQUIRES = arm_v8
SRC_CC  += device.cc
SRC_CC  += device_component.cc
SRC_CC  += device_model_policy.cc
SRC_CC  += main.cc
SRC_CC  += session_component.cc
SRC_CC  += root.cc
LIBS     = base

PLATFORM_ARM_DIR := $(call select_from_repositories,src/drivers/platform/spec/arm)
INC_DIR           = $(PRG_DIR) $(PLATFORM_ARM_DIR)

vpath main.cc $(PRG_DIR)
vpath %.cc $(PLATFORM_ARM_DIR)
