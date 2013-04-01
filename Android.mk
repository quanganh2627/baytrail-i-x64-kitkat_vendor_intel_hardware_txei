LOCAL_PATH:= $(call my-dir)

subdirs := $(addprefix $(LOCAL_PATH)/,$(addsuffix /Android.mk,  \
	Lib \
	App \
	))

include $(subdirs)
