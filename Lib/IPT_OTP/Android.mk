# Build the components in this subtree
# only for platforms with trusted execution engine
ifeq ($(TARGET_BOARD_PLATFORM),baytrail)
include $(call all-subdir-makefiles)
endif
