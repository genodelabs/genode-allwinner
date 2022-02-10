INC_DIR += $(PRG_DIR)

SRC_C += dummies.c lx_emul.c
SRC_C += $(notdir $(wildcard $(PRG_DIR)/generated_dummies.c))

# lx_emul/initcall_order.h
INC_DIR += $(REP_DIR)/src/include

#
# Create symbol alias for jiffies, sharing the value of jiffies_64
#
LD_OPT += --defsym=jiffies=jiffies_64

#
# Lx_emul + Lx_kit definitions
#

SRC_CC  += lx_emul/alloc.cc
SRC_CC  += lx_emul/clock.cc
SRC_CC  += lx_emul/debug.cc
SRC_CC  += lx_emul/init.cc
SRC_CC  += lx_emul/io_mem.cc
SRC_CC  += lx_emul/irq.cc
SRC_CC  += lx_emul/log.cc
SRC_CC  += lx_emul/page_virt.cc
SRC_CC  += lx_emul/task.cc
SRC_CC  += lx_emul/time.cc

SRC_C   += lx_emul/clocksource.c
SRC_C   += lx_emul/irqchip.c
SRC_C   += lx_emul/start.c
SRC_C   += lx_emul/shadow/arch/arm64/kernel/smp.c
SRC_C   += lx_emul/shadow/arch/arm64/mm/ioremap.c
#SRC_C   += lx_emul/shadow/drivers/clk/clk.c
#SRC_C   += lx_emul/shadow/drivers/clk/clkdev.c
SRC_C   += lx_emul/shadow/fs/exec.c
SRC_C   += lx_emul/shadow/kernel/cpu.c
SRC_C   += lx_emul/shadow/kernel/dma/mapping.c
SRC_C   += lx_emul/shadow/kernel/exit.c
SRC_C   += lx_emul/shadow/kernel/fork.c
SRC_C   += lx_emul/shadow/kernel/irq/spurious.c
SRC_C   += lx_emul/shadow/kernel/pid.c
SRC_C   += lx_emul/shadow/kernel/printk/printk.c
SRC_C   += lx_emul/shadow/kernel/rcu/srcutree.c
SRC_C   += lx_emul/shadow/kernel/rcu/tree.c
SRC_C   += lx_emul/shadow/kernel/sched/core.c
SRC_C   += lx_emul/shadow/kernel/smp.c
SRC_C   += lx_emul/shadow/kernel/softirq.c
SRC_C   += lx_emul/shadow/kernel/stop_machine.c
SRC_C   += lx_emul/shadow/lib/devres.c
SRC_C   += lx_emul/shadow/lib/smp_processor_id.c
SRC_C   += lx_emul/shadow/mm/memblock.c
SRC_C   += lx_emul/shadow/mm/percpu.c
SRC_C   += lx_emul/shadow/mm/slab_common.c
SRC_C   += lx_emul/shadow/mm/slub.c
SRC_C   += lx_emul/spec/arm/start.c
SRC_C   += lx_emul/virt_to_page.c

SRC_CC  += lx_kit/console.cc
SRC_CC  += lx_kit/device.cc
SRC_CC  += lx_kit/env.cc
SRC_CC  += lx_kit/init.cc
SRC_CC  += lx_kit/memory.cc
SRC_CC  += lx_kit/scheduler.cc
SRC_CC  += lx_kit/task.cc
SRC_CC  += lx_kit/timeout.cc
SRC_S   += lx_kit/spec/arm_64/setjmp.S

# determine location of lx_emul / lx_kit headers by querying lx_emul/init.h
_LX_EMUL_INIT_H := $(call select_from_repositories,src/include/lx_emul/init.h)
DDE_LINUX_SRC_INC_DIR := $(_LX_EMUL_INIT_H:/lx_emul/init.h=)
SHADOW_INC_DIR := $(addsuffix /lx_emul/shadow, $(DDE_LINUX_SRC_INC_DIR))

INC_DIR += $(DDE_LINUX_SRC_INC_DIR)
INC_DIR += $(DDE_LINUX_SRC_INC_DIR)/spec/arm_64
INC_DIR += $(DDE_LINUX_SRC_INC_DIR)/spec/arm_64/lx_emul/shadow
INC_DIR += $(SHADOW_INC_DIR)
INC_DIR += $(REP_DIR)/src/include/lx_generated

DDE_LINUX_SRC_LIB_DIR := $(DDE_LINUX_SRC_INC_DIR:/include=/lib)

vpath % $(DDE_LINUX_SRC_LIB_DIR)


#
# Linux kernel definitions
#

LIBS += a64_linux_generated

LX_SRC_DIR := $(call select_from_ports,a64_linux)/src/linux
ifeq ($(wildcard $(LX_SRC_DIR)),)
LX_SRC_DIR := $(call select_from_repositories,src/linux)
endif

LX_GEN_DIR := $(LIB_CACHE_DIR)/a64_linux_generated

INC_DIR += $(LX_SRC_DIR)/arch/arm64/include
INC_DIR += $(LX_GEN_DIR)/arch/arm64/include/generated
INC_DIR += $(LX_SRC_DIR)/include
INC_DIR += $(LX_GEN_DIR)/include
INC_DIR += $(LX_SRC_DIR)/arch/arm64/include/uapi
INC_DIR += $(LX_GEN_DIR)/arch/arm64/include/generated/uapi
INC_DIR += $(LX_SRC_DIR)/include/uapi
INC_DIR += $(LX_GEN_DIR)/include/generated/uapi
INC_DIR += $(LX_SRC_DIR)/scripts/dtc/libfdt

CC_C_OPT += -std=gnu89 -include $(LX_SRC_DIR)/include/linux/kconfig.h
CC_C_OPT += -include $(LX_SRC_DIR)/include/linux/compiler_types.h
CC_C_OPT += -D__KERNEL__ -DCONFIG_CC_HAS_K_CONSTRAINT=1
CC_C_OPT += -DKASAN_SHADOW_SCALE_SHIFT=3
CC_C_OPT += -Wall -Wundef -Werror=strict-prototypes -Wno-trigraphs
CC_C_OPT += -Werror=implicit-function-declaration -Werror=implicit-int
CC_C_OPT += -Wno-format-security -Wno-psabi
CC_C_OPT += -Wno-frame-address -Wno-format-truncation -Wno-format-overflow
CC_C_OPT += -Wframe-larger-than=2048 -Wno-unused-but-set-variable -Wimplicit-fallthrough
CC_C_OPT += -Wno-unused-const-variable -Wdeclaration-after-statement -Wvla
CC_C_OPT += -Wno-pointer-sign -Wno-stringop-truncation -Wno-array-bounds -Wno-stringop-overflow
CC_C_OPT += -Wno-restrict -Wno-maybe-uninitialized -Werror=date-time
CC_C_OPT += -Werror=incompatible-pointer-types -Werror=designated-init
CC_C_OPT += -Wno-packed-not-aligned

# avoid link errors whenever the build config's CC_OLEVEL is set to -O0
override CC_OLEVEL := -O2

LX_SRC   = $(shell grep ".*\.c" $(PRG_DIR)/source.list)
SRC_S   += $(shell grep ".*\.S" $(PRG_DIR)/source.list)
SRC_C   += $(LX_SRC)
SRC_S   += $(LX_ASM:$(LX_SRC_DIR)/%=%)

vpath %.c $(LX_SRC_DIR)
vpath %.S $(LX_SRC_DIR)
vpath %.S $(LX_GEN_DIR)

CUSTOM_TARGET_DEPS += $(PRG_DIR)/source.list

# Define per-compilation-unit CC_OPT defines needed by MODULE* macros in Linux
define CC_OPT_LX_RULES =
CC_OPT_$(1) = -DKBUILD_MODFILE='"$(1)"' -DKBUILD_BASENAME='"$(notdir $(1))"' -DKBUILD_MODNAME='"$(notdir $(1))"'
endef

$(foreach file,$(LX_SRC),$(eval $(call CC_OPT_LX_RULES,$(file:%.c=%))))

$(eval $(call CC_OPT_LX_RULES,generated_dummies))
$(eval $(call CC_OPT_LX_RULES,dummies))


# Turn off some warnings
CC_OPT_drivers/gpu/drm/drm_plane_helper += -Wno-uninitialized
CC_OPT_drivers/gpu/drm/sun4i/sun4i_frontend += -DPHYS_OFFSET=0


#
# Generate crc32table.h header
#

crc32table.h: gen_crc32table
	./gen_crc32table > $@

lib/crc32.c: crc32table.h

gen_crc32table: $(LX_SRC_DIR)/lib/gen_crc32table.c
	$(HOST_CC) -I$(LX_GEN_DIR)/include $< -o $@


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
# Force rebuild whenever shadow headers appear or change
#
# Shadow headers are not handled well by the regular dependency-file mechanism
# and ccache.
#
# As new appearing shadow headers (e.g., when switching branches) are not
# covered by .d files, no rebuild is issued for existing object files that
# actually depend on the just appeared header. Specifying all shadow headers
# as global dependencies forces the rebuild of all potentially affected object
# files in such a situation.
#

GLOBAL_DEPS += $(wildcard $(addsuffix /linux/*.h,$(SHADOW_INC_DIR))) \
               $(wildcard $(addsuffix /asm/*.h,$(SHADOW_INC_DIR)))
