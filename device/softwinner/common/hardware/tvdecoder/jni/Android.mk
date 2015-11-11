LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
#LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_TAGS := eng

LOCAL_SRC_FILES := softwinner_tv_TVDecoder.cpp

LOCAL_SHARED_LIBRARIES:= libutils libbinder libui liblog libcutils libnativehelper libtvdecoder_client

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE:= libtvdecoder_jni

include $(BUILD_SHARED_LIBRARY)
