#
# Build the Client proxy library (libsecurityclient)
#
#----------------------------------------
# Build the client (proxy) library
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libsecurityclient
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := SecurityClient.cpp
LOCAL_SHARED_LIBRARIES := libutils libcutils libbinder
#LOCAL_CFLAGS += -DDEBUG
LOCAL_C_INCLUDES +=				\
	frameworks/base/include			\
	system/core/include			\
	$(LOCAL_PATH)/../inc
include $(BUILD_SHARED_LIBRARY)

#----------------------------------------
# Build the Security Service (securityserver)
#
include $(CLEAR_VARS)
LOCAL_MODULE := otpserver
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := SecurityClient.cpp SecurityService.cpp otpserver_main.cpp
LOCAL_SHARED_LIBRARIES :=			\
	libutils				\
	libcutils				\
	libbinder				\
	libiha
LOCAL_C_INCLUDES +=				\
	frameworks/base/include			\
	system/core/include			\
	$(LOCAL_PATH)/../inc
#LOCAL_CFLAGS += -DDEBUG
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := SepService
LOCAL_MODULE_TAGS := optional
LOCAL_REQUIRED_MODULES += otpserver
LOCAL_REQUIRED_MODULES += libsecurityclient
LOCAL_REQUIRED_MODULES += libihamanagerjni
LOCAL_REQUIRED_MODULES += com.intel.security.service.sepmanager
include $(BUILD_PHONY_PACKAGE)

