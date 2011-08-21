LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    rilspy_helper.c

LOCAL_PRELINK_MODULE := false
LOCAL_C_INCLUDES := $(KERNEL_HEADERS)

LOCAL_MODULE_TAGS := optional

#build executable
LOCAL_MODULE:= rilspy_helper
include $(BUILD_EXECUTABLE)
