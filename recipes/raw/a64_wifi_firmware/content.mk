PORT_DIR := $(call port_dir,$(GENODE_DIR)/repos/dde_linux/ports/linux-firmware)

content: ucode_files LICENSE.wifi_drv


.PHONY: ucode_files
ucode_files:
	cp $(PORT_DIR)/firmware/regulatory.db .
	cp $(PORT_DIR)/firmware/regulatory.db.p7s .

LICENSE.wifi_drv:
	cp $(PORT_DIR)/firmware/LICENSE.regulatory_db $@
