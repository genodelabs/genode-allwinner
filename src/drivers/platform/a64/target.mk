TARGET   = a64_platform_drv
REQUIRES = arm_v8

include $(call select_from_repositories,src/drivers/platform/target.inc)
