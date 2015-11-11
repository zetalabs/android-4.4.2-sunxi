LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= tvdecoderclienttest.cpp

LOCAL_MODULE:= tvdecoderclienttest

LOCAL_MODULE_TAGS := tests

LOCAL_CFLAGS :=

LOCAL_SHARED_LIBRARIES += \
		libbinder \
                libcutils \
                libutils \
                libtvdecoder_client

include $(BUILD_EXECUTABLE)
