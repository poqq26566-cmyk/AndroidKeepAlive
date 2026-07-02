LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := keepalive
LOCAL_SRC_FILES := KeepAlive.c KeepAlive.cpp
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)
