PORT_DIR := $(call port_dir,$(GENODE_DIR)/repos/dde_linux/ports/linux-firmware)

content: ucode_files LICENSE.wifi a64_wifi_firmware.tar


.PHONY: ucode_files
ucode_files:
	cp $(PORT_DIR)/firmware/regulatory.db .
	cp $(PORT_DIR)/firmware/regulatory.db.p7s .

LICENSE.wifi:
	cp $(PORT_DIR)/firmware/LICENSE.regulatory_db $@

include $(GENODE_DIR)/repos/base/recipes/content.inc

a64_wifi_firmware.tar: ucode_files LICENSE.wifi
	$(TAR) --remove-files -cf $@ -C . *.*
