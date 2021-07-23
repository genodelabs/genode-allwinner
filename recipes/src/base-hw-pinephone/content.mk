CONTENT += lib/mk/spec/arm_v8/bootstrap-hw-pine_a64lts.mk \
           lib/mk/spec/arm_v8/core-hw-pine_a64lts.mk \
           src/bootstrap/board/pine_a64lts \
           src/core/board/pine_a64lts

include $(GENODE_DIR)/repos/base-hw/recipes/src/base-hw_content.inc

