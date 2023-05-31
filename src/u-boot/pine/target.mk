TARGET   = pine_uboot
REQUIRES = arm_v8a

SUPPORTED_BOARDS := pine_a64lts pinephone

UBOOT_DEFCONFIG(pine_a64lts) := pine64-lts_defconfig
UBOOT_DEFCONFIG(pinephone)   := pinephone_defconfig

# enable fastboot on the PinePhone
UBOOT_ENABLE(pinephone) := USB_MUSB_GADGET USB_HOST USB_EHCI_HCD
UBOOT_OPTION(pinephone) := USBNET_DEVADDR="02:ba:fe:7b:59:37" \
                           USBNET_HOST_ADDR="02:ba:fe:7b:59:36"

# read boot config from SD-card
UBOOT_OPTION(pine_a64lts) += ENV_EXT4_INTERFACE="mmc"
UBOOT_OPTION(pine_a64lts) += ENV_EXT4_DEVICE_AND_PART="0:1"
UBOOT_OPTION(pine_a64lts) += ENV_EXT4_FILE="/boot/uboot.env"

UBOOT_OPTION(pinephone)   += ENV_EXT4_INTERFACE="mmc"
UBOOT_OPTION(pinephone)   += ENV_EXT4_DEVICE_AND_PART="0:1"
UBOOT_OPTION(pinephone)   += ENV_EXT4_FILE="/boot/uboot.env"

UBOOT_ENABLE_COMMON       += ENV_IS_IN_EXT4 CMD_EXT4_WRITE
UBOOT_DISABLE_COMMON      += ENV_IS_IN_FAT

# find mkfs.ext2 on Debian
PATH := /usr/sbin:$(PATH)

unexport BOARD
unexport MAKEFLAGS
unexport .SHELLFLAGS


#
# ARM Trusted Firmware
#

ARM_TRUSTED_FIRMWARE_DIR := $(call select_from_ports,pine_uboot)/arm_trusted_firmware

BL31_BIN_FILE = $(PWD)/$(PRG_REL_DIR)/arm_trusted_firmware/sun50i_a64/release/bl31.bin

$(BL31_BIN_FILE):
	$(VERBOSE) $(MAKE) $(VERBOSE_DIR) -C $(ARM_TRUSTED_FIRMWARE_DIR) \
	                   CROSS_COMPILE=$(CROSS_DEV_PREFIX) \
	                   BUILD_BASE=$(PWD)/$(PRG_REL_DIR)/arm_trusted_firmware \
	                   PLAT=sun50i_a64 DEBUG=0 LOG_LEVEL=0 \
	                   LDFLAGS=--no-warn-rwx-segments \
	                   bl31


#
# Firmware for system-control processor (SCP)
#
# The binary is built by lib/mk/scp-$(BOARD).mk
#

SCP_BIN_FILE := $(addsuffix /scp.bin,$(addprefix $(LIB_CACHE_DIR)/scp-,$(BOARD)))


#
# U-Boot
#

UBOOT_DIR        := $(call select_from_ports,pine_uboot)/uboot
UBOOT_BUILD_DIR   = $(PWD)/$(PRG_REL_DIR)/$(BOARD)
UBOOT_CONFIG_FILE = $(BOARD)/.config
UBOOT_BIN_FILE    = $(BOARD)/u-boot.bin

UBOOT_MAKE_ARGS = $(VERBOSE_DIR) -C $(UBOOT_DIR) \
                  CROSS_COMPILE=$(CROSS_DEV_PREFIX) \
                  O=$(UBOOT_BUILD_DIR) \
                  BL31=$(BL31_BIN_FILE) \
                  SCP=$(SCP_BIN_FILE)

$(UBOOT_CONFIG_FILE): $(BL31_BIN_FILE) $(SCP_BIN_FILE)
	$(VERBOSE) mkdir -p $(BOARD)
	$(VERBOSE) cp $(UBOOT_DIR)/configs/${UBOOT_DEFCONFIG(${BOARD})} $(BOARD)/.config
	$(VERBOSE) ( \
		true; \
		$(foreach I, $(UBOOT_ENABLE_COMMON)  ${UBOOT_ENABLE(${BOARD})},  echo 'CONFIG_$I=y';) \
		$(foreach I, $(UBOOT_DISABLE_COMMON) ${UBOOT_DISABLE(${BOARD})}, echo 'CONFIG_$I=n';) \
		$(foreach I, $(UBOOT_OPTION_COMMON)  ${UBOOT_OPTION(${BOARD})},  echo 'CONFIG_$I';) \
	) >> $(BOARD)/.config
	$(VERBOSE) $(MAKE) $(UBOOT_MAKE_ARGS) olddefconfig

$(UBOOT_CONFIG_FILE): $(MAKEFILE_LIST)

$(UBOOT_BIN_FILE): $(UBOOT_CONFIG_FILE)
	$(VERBOSE) $(MAKE) $(UBOOT_MAKE_ARGS)


#
# SD-card image
#
# https://linux-sunxi.org/Bootable_SD_card
#

SD_CARD_IMAGE_FILE = $(BOARD).img

$(SD_CARD_IMAGE_FILE) : $(UBOOT_BIN_FILE)

PART_OFFSET_MB   := 4
PART_SIZE_MB     := 12
PART_START_BYTES := $(shell echo "$(PART_OFFSET_MB)*1024*1024" | bc -l )
PART_START_BLOCK := $(shell echo "$(PART_OFFSET_MB)*2048" | bc -l )
PART_SIZE_BLOCK  := $(shell echo "$(PART_SIZE_MB)*2024" | bc -l )
PART_END_BLOCK   := $(shell echo "$(PART_START_BLOCK) + $(PART_SIZE_BLOCK)" | bc -l )

$(SD_CARD_IMAGE_FILE):
	$(MSG_CONVERT)SD-card image $(PRG_REL_DIR)/$@
	$(VERBOSE) dd if=/dev/zero of=$@.incomplete bs=1M count=16 2> /dev/null
	$(VERBOSE) mkfs.ext2 -q -F -L 'U-Boot' -N 0 \
	                     -E offset=$(PART_START_BYTES) \
	                     -m 5 -r 1 -t ext2 \
	                     $@.incomplete $(PART_SIZE_MB)M
	$(VERBOSE) ( \
		sgdisk -z $@.incomplete \
		sgdisk -j 2048 $@.incomplete \
		sgdisk --new=1:$(PART_START_BLOCK):$(PART_END_BLOCK) $@.incomplete \
		sgdisk --change-name=1:UBOOT $@.incomplete \
		sgdisk --hybrid $@.incomplete \
	) 2> /dev/null > /dev/null
	$(VERBOSE) dd if=$(BOARD)/u-boot-sunxi-with-spl.bin of=$@.incomplete \
	              bs=1024 seek=8 conv=notrunc 2> /dev/null
	$(VERBOSE) mv $@.incomplete $@


# trigger build only for supported boards
ifneq ($(filter $(SUPPORTED_BOARDS), $(BOARD)),)
LIBS += scp-$(BOARD)
$(TARGET) : $(SD_CARD_IMAGE_FILE)
endif

