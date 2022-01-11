SRC_DIR = src/drivers/platform

include $(GENODE_DIR)/repos/base/recipes/src/content.inc

GENERIC_DIR := $(GENODE_DIR)/repos/os/$(SRC_DIR)

GENERIC_FILES := $(notdir $(wildcard $(GENERIC_DIR)/*.h)) \
                 device.cc session_component.cc root.cc \
                 device_component.cc device_model_policy.cc

MIRROR_FROM_OS_DIR := $(addprefix $(SRC_DIR)/,$(GENERIC_FILES))

content: $(MIRROR_FROM_OS_DIR)

$(MIRROR_FROM_OS_DIR):
	mkdir -p $(dir $@)
	cp -r $(GENODE_DIR)/repos/os/$@ $@
