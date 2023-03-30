REQUIRES := arm_v8a

ifeq ($(CONTRIB_DIR),)
PC_REPO_LIB_DIR  := $(REP_DIR)/src/lib/pc
ORIGINAL_LIB_DIR := $(REP_DIR)/src/lib/wifi
else
# For now use the 'base' repository to look up the 'pc' repository
# so we do not need to add that in the build.conf. This should be
# changed back to 'select_from_repositories' when the common parts
# of the wifi library are moved to 'dde_linux'.
PC_REPO_DIR      := $(BASE_DIR)/../pc
PC_REPO_LIB_DIR  := $(PC_REPO_DIR)/src/lib/pc
ORIGINAL_LIB_DIR := $(PC_REPO_DIR)/src/lib/wifi
endif


TARGET_LIB_DIR   := $(REP_DIR)/src/lib/a64_wifi

DRIVER := wifi
BOARDS                 := pinephone
DTS_EXTRACT(pinephone) := --select mmc1


SHARED_LIB := yes

LD_OPT  += --version-script=$(ORIGINAL_LIB_DIR)/symbol.map

LIBS    += base jitterentropy a64_linux_generated a64_lx_emul
INC_DIR += $(TARGET_LIB_DIR)
INC_DIR += $(ORIGINAL_LIB_DIR)
SRC_CC  += wlan.cc
SRC_CC  += firmware.cc
SRC_CC  += socket_call.cc
SRC_CC  += lx_emul/random.cc
SRC_CC  += dtb_helper.cc

SRC_C   += clock.c
SRC_C   += dummies.c
SRC_C   += lx_emul.c
SRC_C   += lx_user.c
SRC_C   += uplink.c
SRC_C   += lx_socket_call.c
SRC_C   += $(notdir $(wildcard $(REP_DIR)/src/lib/a64_wifi/generated_dummies.c))


SRC_C += lx_emul/shadow/mm/page_alloc.c
SRC_C += lx_emul/a64/common_dummies.c
SRC_C += lx_emul/a64/pmic.c

vpath lx_emul/a64/common_dummies.c $(REP_DIR)/src/lib
vpath lx_emul/a64/pmic.c           $(REP_DIR)/src/lib
vpath lx_emul/a64/sched.c          $(REP_DIR)/src/lib

SRC_C   += lx_emul/shadow/mm/page_alloc.c

#
# The following block of compilation units is so far part of the
# '*_lx_emul' library. Some parts could be merged into the generic
# DDE Linux part.
#

SRC_C   += lx_emul/mapping.c
SRC_C   += lx_emul/page_alloc.c
SRC_C   += lx_emul/sched_core.c
SRC_C   += lx_emul/vmalloc.c
SRC_C   += lx_emul/shadow/fs/libfs.c
SRC_C   += lx_emul/shadow/drivers/char/random.c
SRC_C   += lx_emul/shadow/mm/dmapool.c

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

CC_C_OPT += -DCONFIG_RFKILL_INPUT

CC_OPT_lx_socket_call     += -DKBUILD_MODNAME='"lx_socket_call"'
CC_OPT_dummies            += -DKBUILD_MODNAME='"dummies"'
CC_OPT_generated_dummies  += -DKBUILD_MODNAME='"generated_dummies"'
CC_OPT_net/mac80211/trace += -I$(LX_SRC_DIR)/net/mac80211
CC_OPT_net/wireless/trace += -I$(LX_SRC_DIR)/net/wireless

vpath %.c  $(TARGET_LIB_DIR)
vpath %.cc $(TARGET_LIB_DIR)
vpath %.c  $(ORIGINAL_LIB_DIR)
vpath %.cc $(ORIGINAL_LIB_DIR)

vpath %.c $(PC_REPO_LIB_DIR)

$(LIB).lib.so: $(ORIGINAL_LIB_DIR)/symbol.map

#
# Genode C-API backends
#

SRC_CC  += genode_c_api/uplink.cc

vpath genode_c_api/uplink.cc $(subst /genode_c_api,,$(call select_from_repositories,src/lib/genode_c_api))
