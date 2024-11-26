# check for installation of device-tree compiler
$(call check_tool,dtc)

LIBS    += a64_linux_generated

INC_DIR += $(PRG_DIR)

SRC_C   += dummies.c lx_emul.c
SRC_C   += $(notdir $(wildcard $(PRG_DIR)/generated_dummies.c))

# lx_emul/initcall_order.h
INC_DIR += $(REP_DIR)/src/include

LX_SRC_DIR := $(call select_from_ports,a64_linux)/src/linux
ifeq ($(wildcard $(LX_SRC_DIR)),)
LX_SRC_DIR := $(call select_from_repositories,src/linux)
endif

LX_GEN_DIR := $(LIB_CACHE_DIR)/a64_linux_generated

include $(call select_from_repositories,lib/import/import-lx_emul_common.inc)

INC_DIR += $(LX_SRC_DIR)/scripts/dtc/libfdt

#
# Additional Lx_emul + Lx_kit definitions
#

SRC_CC  += lx_emul/clock.cc
SRC_CC  += lx_emul/io_mem.cc
SRC_CC  += lx_emul/io_port.cc
SRC_CC  += lx_emul/irq.cc
SRC_C   += lx_emul/shadow/arch/arm64/kernel/smp.c
SRC_C   += lx_emul/shadow/arch/arm64/mm/ioremap.c
SRC_C   += lx_emul/shadow/drivers/char/random.c
SRC_C   += lx_emul/shadow/kernel/dma/mapping.c
SRC_C   += lx_emul/shadow/kernel/irq/spurious.c
SRC_C   += lx_emul/shadow/kernel/rcu/srcutree.c
SRC_C   += lx_emul/shadow/kernel/rcu/tree.c
SRC_C   += lx_emul/shadow/kernel/smp.c
SRC_C   += lx_emul/shadow/kernel/stop_machine.c
SRC_C   += lx_emul/shadow/lib/devres.c
SRC_C   += lx_emul/shadow/lib/smp_processor_id.c
SRC_C   += lx_emul/shadow/mm/page_alloc.c
SRC_CC  += lx_kit/device.cc
SRC_CC  += lx_kit/memory_dma.cc


# Turn off some warnings
CC_OPT_drivers/gpu/drm/drm_plane_helper += -Wno-uninitialized

#
# Generate driver-specific device-tree binary data
#
# The rules below use the tool/dts/extract tool to generate a device tree
# containing the driver parameters for a given board.
#
# The resulting dtb file is named <driver>-<board>.dtb
#

DTS_PATH(pine_a64lts) := arch/arm64/boot/dts/allwinner/sun50i-a64-pine64-lts.dts
DTS_PATH(pinephone)   := arch/arm64/boot/dts/allwinner/sun50i-a64-pinephone-1.2.dts

CUSTOM_TARGET_DEPS += $(addprefix $(INSTALL_DIR)/$(DRIVER)-,$(addsuffix .dtb,$(BOARDS)))

$(INSTALL_DIR)/%.dtb: %.dtb
	$(VERBOSE)cp -f $< $@

%.dtb: %.dts
	$(VERBOSE)dtc -Idts $< > $@

# dependencies of driver-specifc dts files from board's dts files
$(foreach B,$(BOARDS),$(eval $(DRIVER)-$B.dts: $(LX_SRC_DIR)/${DTS_PATH($B)}))

# dependencies of driver-specifc dts files from target-description files
$(foreach B,$(BOARDS),$(eval $(DRIVER)-$B.dts: $(MAKEFILE_LIST)))

$(DRIVER)-%.dts:
	$(VERBOSE)$(CROSS_DEV_PREFIX)cpp -I$(LX_SRC_DIR)/include \
	          -x assembler-with-cpp -MMD -P $(LX_SRC_DIR)/${DTS_PATH($*)} |\
	    $(BASE_DIR)/../../tool/dts/extract ${DTS_EXTRACT($*)} - |\
	    grep -v "/omit-if-no-ref/" > $@

#
# Declare driver target and dtb files as build artifacts
#
BUILD_ARTIFACTS := $(TARGET) $(addprefix $(DRIVER)-,$(addsuffix .dtb,$(BOARDS)))
