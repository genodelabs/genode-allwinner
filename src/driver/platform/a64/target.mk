TARGET   = a64_platform
REQUIRES = arm_v8

include $(call select_from_repositories,src/driver/platform/target.inc)
