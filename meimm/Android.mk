LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)


LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := meimm.c
LOCAL_MODULE    := libmeimm

LOCAL_COPY_HEADERS_TO := libmei
LOCAL_COPY_HEADERS := meimm.h
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_SHARED_LIBRARIES := libcutils

include $(BUILD_SHARED_LIBRARY)


