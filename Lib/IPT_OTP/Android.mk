# Build the components in this subtree
# only for platforms with trusted execution engine
ifeq ($(BUILD_WITH_SECURITY_FRAMEWORK), txei)
include $(call all-subdir-makefiles)
endif
