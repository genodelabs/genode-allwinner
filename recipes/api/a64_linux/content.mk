#
# Content hosted in the dde_linux repository
#

MIRRORED_FROM_DDE_LINUX := src/lib/lx_emul \
                           src/lib/lx_kit \
                           src/include/lx_emul \
                           src/include/lx_user \
                           src/include/spec/arm_v8/lx_kit \
                           src/include/lx_kit \
                           lib/import/import-lx_emul_common.inc

content: $(MIRRORED_FROM_DDE_LINUX)
$(MIRRORED_FROM_DDE_LINUX):
	mkdir -p $(dir $@); cp -r $(GENODE_DIR)/repos/dde_linux/$@ $(dir $@)


#
# Content hosted in the allwinner repository
#

MIRRORED_FROM_REP_DIR := lib/mk/spec/arm_v8/a64_linux_generated.mk \
                         src/a64_linux/target.inc \
                         lib/import/import-a64_lx_emul.mk \
                         lib/mk/a64_lx_emul.mk \
                         src/include/lx_emul/initcall_order.h \
                         src/include/lx_emul/reset.h \
                         src/lib/lx_emul/a64

content: $(MIRRORED_FROM_REP_DIR)
$(MIRRORED_FROM_REP_DIR):
	$(mirror_from_rep_dir)


#
# Content from the Linux source tree
#

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/a64_linux)
LX_REL_DIR := src/linux
LX_ABS_DIR := $(addsuffix /$(LX_REL_DIR),$(PORT_DIR))

# ingredients needed for creating a Linux build directory / generated headers
LX_FILES += Makefile \
            Kbuild \
            arch/arm64/Makefile \
            arch/arm64/boot/dts \
            arch/arm64/configs \
            arch/arm64/include/asm/Kbuild \
            arch/arm64/include/uapi/asm/Kbuild \
            arch/arm64/tools/Makefile \
            arch/arm64/tools/cpucaps \
            arch/arm64/tools/gen-cpucaps.awk \
            arch/arm64/tools/gen-sysreg.awk \
            arch/arm64/tools/sysreg \
            arch/x86/entry/syscalls/syscall_32.tbl \
            include/asm-generic/Kbuild \
            include/linux/compiler-version.h \
            include/linux/kbuild.h \
            include/linux/license.h \
            include/uapi/Kbuild \
            include/uapi/asm-generic/Kbuild \
            kernel/configs/tiny-base.config \
            kernel/configs/tiny.config \
            scripts/Kbuild.include \
            scripts/Makefile \
            scripts/Makefile.asm-generic \
            scripts/Makefile.build \
            scripts/Makefile.compiler \
            scripts/Makefile.defconf \
            scripts/Makefile.extrawarn \
            scripts/Makefile.host \
            scripts/Makefile.lib \
            scripts/as-version.sh \
            scripts/asn1_compiler.c \
            scripts/basic/Makefile \
            scripts/basic/fixdep.c \
            scripts/cc-version.sh \
            scripts/checksyscalls.sh \
            scripts/config \
            scripts/dtc \
            scripts/kconfig/merge_config.sh \
            scripts/ld-version.sh \
            scripts/mkcompile_h \
            scripts/min-tool-version.sh \
            scripts/mod \
            scripts/pahole-flags.sh \
            scripts/pahole-version.sh \
            scripts/remove-stale-files \
            scripts/setlocalversion \
            scripts/sorttable.c \
            scripts/sorttable.h \
            scripts/subarch.include \
            tools/include/tools

LX_SCRIPTS_KCONFIG_FILES := $(notdir $(wildcard $(LX_ABS_DIR)/scripts/kconfig/*.c)) \
                            $(notdir $(wildcard $(LX_ABS_DIR)/scripts/kconfig/*.h)) \
                            Makefile lexer.l parser.y
LX_FILES += $(addprefix scripts/kconfig/,$(LX_SCRIPTS_KCONFIG_FILES)) \

LX_FILES += $(shell cd $(LX_ABS_DIR); find -name "Kconfig*" -printf "%P\n")

# needed for generated/asm-offsets.h
LX_FILES += kernel/bounds.c \
            kernel/time/timeconst.bc \
            arch/arm64/kernel/asm-offsets.c \
            arch/arm64/include/asm/current.h \
            arch/arm64/include/asm/memory.h \
            arch/arm64/include/asm/irqflags.h \
            arch/arm64/include/asm/page.h \
            arch/arm64/include/asm/pgtable.h \
            arch/arm64/include/asm/mte.h \
            arch/arm64/include/asm/fixmap.h \
            arch/arm64/include/asm/boot.h \
            arch/arm64/include/asm/signal32.h \
            arch/arm64/include/asm/smp_plat.h \
            arch/arm64/include/asm/spinlock.h \
            arch/arm64/include/asm/suspend.h \
            include/uapi/linux/arm_sdei.h \
            include/acpi \
            include/asm-generic/current.h \
            include/asm-generic/memory_model.h \
            include/asm-generic/qrwlock.h \
            include/asm-generic/qspinlock.h \
            include/asm-generic/fixmap.h \
            include/asm-generic/pgtable_uffd.h \
            include/linux/asn1.h \
            include/linux/asn1_ber_bytecode.h \
            include/linux/arm_sdei.h \
            include/linux/arm-smccc.h \
            include/linux/cper.h \
            include/linux/page_table_check.h \
            include/linux/pgtable.h

# needed for gen_crc32table
LX_FILES += lib/gen_crc32table.c \
            lib/crc32.c

content: src/linux/include/linux/kvm_host.h
src/linux/include/linux/kvm_host.h: # cut dependencies from kvm via dummy header
	mkdir -p $(dir $@)
	touch $@

# prevent build of vdso as side effect of the linux-build directory preparation
content: src/linux/arch/arm64/kernel/vdso/Makefile
src/linux/arch/arm64/kernel/vdso/Makefile:
	mkdir -p $(dir $@)
	echo "default:" > $@
	echo "include/generated/vdso-offsets.h:" >> $@
	echo "arch/arm64/kernel/vdso/vdso.so:" >> $@

# add content listed in the repository's source.list or dep.list files
LX_FILE_LISTS := $(shell find -H $(REP_DIR) -name dep.list -or -name source.list)
LX_FILES += $(shell cat $(LX_FILE_LISTS))
LX_FILES := $(sort $(LX_FILES))
MIRRORED_FROM_PORT_DIR += $(addprefix $(LX_REL_DIR)/,$(LX_FILES))

content: $(MIRRORED_FROM_PORT_DIR)
$(MIRRORED_FROM_PORT_DIR):
	mkdir -p $(dir $@)
	cp -r $(addprefix $(PORT_DIR)/,$@) $@

content: LICENSE
LICENSE:
	cp $(PORT_DIR)/src/linux/COPYING $@
