REQUIRES = hw

REP_INC_DIR += src/core/board/pine_a64lts

# add C++ sources
SRC_CC += kernel/vcpu_thread_on.cc
SRC_CC += spec/arm/gicv2.cc
SRC_CC += spec/arm/virtualization/gicv2.cc
SRC_CC += spec/arm/virtualization/platform_services.cc
SRC_CC += spec/arm/virtualization/vm_session_component.cc
SRC_CC += spec/arm_v8/kernel/vcpu.cc
SRC_CC += vm_session_common.cc
SRC_CC += vm_session_component.cc

#add assembly sources
SRC_S += spec/arm_v8/hypervisor.s

# include less specific configuration
include $(call select_from_repositories,lib/mk/spec/arm_v8/core-hw.inc)
