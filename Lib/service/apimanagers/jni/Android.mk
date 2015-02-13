LOCAL_PATH := $(call my-dir)

# Build the library
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libihamanagerjni
LOCAL_SRC_FILES := ihaManagerJni.cpp
LOCAL_SHARED_LIBRARIES := libutils libcutils libandroid_runtime libnativehelper libsecurityclient
#LOCAL_CFLAGS += -DDEBUG
LOCAL_C_INCLUDES +=				\
	frameworks/base/include			\
	system/core/include			\
	$(LOCAL_PATH)/../../inc
include $(BUILD_SHARED_LIBRARY)
