#
# Linux kernel configuration
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

