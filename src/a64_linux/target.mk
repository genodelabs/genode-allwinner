TARGET   := a64_linux
REQUIRES := arm_64

CUSTOM_TARGET_DEPS := kernel_build.phony

LX_DIR := $(call select_from_ports,a64_linux)/src/linux
PWD    := $(shell pwd)

LX_MK_ARGS = ARCH=arm64 CROSS_COMPILE=$(CROSS_DEV_PREFIX)

#
# Linux kernel configuration
#
# Start with 'make tinyconfig', enable/disable options via 'scripts/config',
# and resolve config dependencies via 'make olddefconfig'.
#

# kernel fundamentals
LX_ENABLE += TTY SERIAL_EARLYCON SERIAL_OF_PLATFORM PRINTK HAS_IOMEM

# initrd support
LX_ENABLE += BINFMT_ELF BLK_DEV_INITRD

# SoC
LX_ENABLE += ARCH_SUNXI

# UART device
LX_ENABLE += SERIAL_8250 $(addprefix SERIAL_8250_,16550A_VARIANTS DW CONSOLE)

# network infrastructure
LX_ENABLE += NET NETDEVICES ETHERNET

# network driver
LX_ENABLE += NET_VENDOR_STMICRO STMMAC_ETH STMMAC_PLATFORM DWMAC_SUN8I

# ethernet PHY
LX_ENABLE += OF_MDIO MDIO_DEVICE PHYLIB

# network protocols
LX_ENABLE += INET IP_PNP IP_PNP_DHCP

# slim down kernel by removing superfluous drivers
LX_DISABLE += $(addprefix NET_VENDOR_, ALACRITECH AMAZON AQUANTIA ARC BROADCOM \
                                       CADENCE CAVIUM CORTINA EZCHIP GOOGLE \
                                       HISILICON HUAWEI I825XX INTEL MARVELL \
                                       MICREL MICROCHIP MICROSEMI NATSEMI \
                                       NETRONOME NI 8390 PENSANDO QUALCOMM \
                                       RENESAS ROCKER SAMSUNG SEEQ SOLARFLARE \
                                       SMSC SOCIONEXT)
LX_DISABLE += WLAN

# filter for make output of kernel build system
BUILD_OUTPUT_FILTER = 2>&1 | sed "s/^/      [Linux]  /"

kernel_config.tag:
	$(MSG_CONFIG)Linux
	$(VERBOSE)$(MAKE) -C $(LX_DIR) O=$(PWD) $(LX_MK_ARGS) tinyconfig $(BUILD_OUTPUT_FILTER)
	$(VERBOSE)$(LX_DIR)/scripts/config $(addprefix --enable ,$(LX_ENABLE))
	$(VERBOSE)$(LX_DIR)/scripts/config $(addprefix --disable ,$(LX_DISABLE))
	$(VERBOSE)$(MAKE) $(LX_MK_ARGS) olddefconfig $(BUILD_OUTPUT_FILTER)
	$(VERBOSE)touch $@

# update Linux kernel config on makefile changes
kernel_config.tag: $(MAKEFILE_LIST)

kernel_build.phony: kernel_config.tag
	$(MSG_BUILD)Linux
	$(VERBOSE)$(MAKE) $(LX_MK_ARGS) dtbs Image $(BUILD_OUTPUT_FILTER)

