PORT_DIR := $(call port_dir,$(GENODE_DIR)/repos/dde_linux/ports/linux-firmware)

content: ucode_files LICENSE.wifi a64_wifi_firmware.tar


.PHONY: ucode_files
ucode_files:
	cp $(PORT_DIR)/firmware/regulatory.db .
	cp $(PORT_DIR)/firmware/regulatory.db.p7s .

LICENSE.wifi:
	cp $(PORT_DIR)/firmware/LICENSE.regulatory_db $@

a64_wifi_firmware.tar: ucode_files LICENSE.wifi
	tar --mtime='2023-05-10 00:00Z' --remove-files \
	    --owner=0 --group=0 --numeric-owner --mode='go=' \
	    -cf $@ -C . *.*
