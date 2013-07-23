# Build the components in this subtree
# only for platforms with trusted execution engine
ifeq ($(TARGET_BOARD_PLATFORM),baytrail)
LOCAL_PATH:= $(call my-dir)
include $(call all-subdir-makefiles)
endif
