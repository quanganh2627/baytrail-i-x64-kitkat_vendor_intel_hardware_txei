# Build the components in this subtree
# only for platforms with trusted execution engine
ifneq ($(filter $(TARGET_BOARD_PLATFORM),baytrail cherrytrail),)
LOCAL_PATH:= $(call my-dir)
include $(call all-subdir-makefiles)
endif
