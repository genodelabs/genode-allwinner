PORT_DIR := $(call port_dir,$(GENODE_DIR)/repos/dde_linux/ports/linux-firmware)

content: ucode_files LICENSE.wifi_drv a64_wifi_firmware.tar


.PHONY: ucode_files
ucode_files:
	cp $(PORT_DIR)/firmware/regulatory.db .
	cp $(PORT_DIR)/firmware/regulatory.db.p7s .

LICENSE.wifi_drv:
	cp $(PORT_DIR)/firmware/LICENSE.regulatory_db $@

a64_wifi_firmware.tar: ucode_files LICENSE.wifi_drv
	tar --mtime='2023-05-10 00:00Z' --remove-files \
	    -cf $@ -C . *.*
