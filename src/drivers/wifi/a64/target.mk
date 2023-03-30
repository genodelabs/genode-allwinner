TARGET  := a64_wifi_drv
DRIVER  := wifi 
SRC_CC  := main.cc wpa.cc
LIBS    := base a64_wifi
LIBS    += libc
LIBS    += wpa_supplicant
LIBS    += libcrypto libssl wpa_driver_nl80211

# For now use the 'base' repository to look up the 'pc' repository
# so we do not need to add that in the build.conf. This should be
# changed when the common parts of the wifi driver are moved to
# 'dde_linux'.
PC_REPO_DIR := $(BASE_DIR)/../pc
PC_SRC_DIR  := $(PC_REPO_DIR)/src/drivers/wifi/pc

INC_DIR += $(PC_SRC_DIR)

CC_CXX_WARN_STRICT :=

vpath %.cc $(PC_SRC_DIR)

BOARDS                 := pinephone
DTS_EXTRACT(pinephone) := --select mmc1
