# Copyright 2006 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    libsec_rilspy.c

LOCAL_SHARED_LIBRARIES := \
    libcutils libril

# for asprinf
LOCAL_CFLAGS := -D_GNU_SOURCE

LOCAL_PRELINK_MODULE := false
LOCAL_C_INCLUDES := $(KERNEL_HEADERS)

ifeq (foo,foo)
  #build shared library
  LOCAL_SHARED_LIBRARIES += \
      libdl
  LOCAL_LDLIBS += -lpthread -ldl
  LOCAL_CFLAGS += -DRIL_SHLIB
  LOCAL_MODULE:= libsec_rilspy
  include $(BUILD_SHARED_LIBRARY)
else
  #build executable
  LOCAL_SHARED_LIBRARIES += \
      libril libdl
  LOCAL_MODULE:= libsec_ril_wrapper
  include $(BUILD_EXECUTABLE)
endif
