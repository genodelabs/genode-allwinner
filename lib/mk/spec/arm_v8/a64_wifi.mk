REQUIRES := arm_v8a

include $(call select_from_repositories,lib/mk/wifi.inc)

TARGET_LIB_DIR := $(REP_DIR)/src/lib/a64_wifi

LIBS    += a64_linux_generated a64_lx_emul

DRIVER                 := wifi
BOARDS                 := pinephone
DTS_EXTRACT(pinephone) := --select mmc1

INC_DIR += $(TARGET_LIB_DIR)

SRC_CC  += lx_emul/random.cc
SRC_CC  += dtb_helper.cc

SRC_C   += clock.c
SRC_C   += irq.c
SRC_C   += $(notdir $(wildcard $(TARGET_LIB_DIR)/generated_dummies.c))

SRC_C += lx_emul/shadow/drivers/char/random.c
SRC_C += lx_emul/shadow/fs/libfs.c
SRC_C += lx_emul/shadow/mm/page_alloc.c
SRC_C += lx_emul/shadow/mm/vmalloc.c

SRC_C += lx_emul/a64/common_dummies.c
SRC_C += lx_emul/a64/pmic.c
SRC_C += lx_emul/a64/pio-dummy.c
SRC_C += lx_emul/a64/r_pio.c

vpath lx_emul/a64/common_dummies.c $(REP_DIR)/src/lib
vpath lx_emul/a64/pmic.c           $(REP_DIR)/src/lib
vpath lx_emul/a64/pio-dummy.c      $(REP_DIR)/src/lib
vpath lx_emul/a64/r_pio.c          $(REP_DIR)/src/lib

#
# RTL8723cs extra flags obtained by instrumenting 'rtl8723cs/Makefile'
#
CC_C_OPT += \
            -I$(LX_SRC_DIR)/drivers/staging/rtl8723cs/include \
            -I$(LX_SRC_DIR)/drivers/staging/rtl8723cs/platform \
            -I$(LX_SRC_DIR)/drivers/staging/rtl8723cs/hal/btc \
            -I$(LX_SRC_DIR)/drivers/staging/rtl8723cs/hal/phydm \
            -I$(LX_SRC_DIR)/drivers/staging/rtl8723cs/core/crypto
CC_C_OPT += \
            -Wno-address-of-packed-member \
            -Wno-unused-variable \
            -Wno-date-time \
            -DCONFIG_RTW_CFGVENDOR_RSSIMONITOR \
            -DCONFIG_RTW_CFGVENDOR_CQM_THRESHOLD_EVT_LOW \
            -DCONFIG_RTL8703B \
            -DCONFIG_AP_MODE \
            -DCONFIG_P2P \
            -DRTW_IPS_MODE=0 \
            -DRTW_LPS_MODE=1 \
            -DCONFIG_POWER_SAVING \
            -DCONFIG_BT_COEXIST \
            -DCONFIG_TXPWR_BY_RATE=1 \
            -DCONFIG_TXPWR_BY_RATE_EN=1 \
            -DCONFIG_TXPWR_LIMIT=1 \
            -DCONFIG_TXPWR_LIMIT_EN=0 \
            -DCONFIG_RTW_ADAPTIVITY_EN=0 \
            -DCONFIG_RTW_ADAPTIVITY_MODE=0 \
            -DCONFIG_IEEE80211W \
            -DCONFIG_WOWLAN \
            -DRTW_WAKEUP_EVENT=0xf \
            -DRTW_SUSPEND_TYPE=0 \
            -DCONFIG_RTW_SDIO_PM_KEEP_POWER \
            -DCONFIG_LAYER2_ROAMING \
            -DCONFIG_ROAMING_FLAG=0x3 \
            -DCONFIG_GPIO_WAKEUP \
            -DHIGH_ACTIVE_DEV2HST=0 \
            -DHIGH_ACTIVE_HST2DEV=0 \
            -DCONFIG_RTW_SDIO_PM_KEEP_POWER \
            -DCONFIG_BR_EXT '-DCONFIG_BR_EXT_BRNAME="'br0'"' \
            -DCONFIG_TDLS \
            -DCONFIG_WIFI_MONITOR \
            -DCONFIG_RTW_NETIF_SG \
            -DCONFIG_RTW_UP_MAPPING_RULE=0 \
            -DDM_ODM_SUPPORT_TYPE=0x04 \
            -DCONFIG_LITTLE_ENDIAN \
            -DCONFIG_IOCTL_CFG80211 \
            -DRTW_USE_CFG80211_STA_EVENT \
            -DCONFIG_RTW_80211R \
            -DCONFIG_RESUME_IN_WORKQUEUE
CC_OPT_drivers/staging/rtl8723cs/core/rtw_debug += -Wno-builtin-macro-redefined \
                                                   -D__DATE__=\"0\" -D__TIME__=\"0\"

CC_OPT_net/mac80211/trace += -I$(LX_SRC_DIR)/net/mac80211
CC_OPT_net/wireless/trace += -I$(LX_SRC_DIR)/net/wireless

vpath %.c  $(TARGET_LIB_DIR)
vpath %.cc $(TARGET_LIB_DIR)
