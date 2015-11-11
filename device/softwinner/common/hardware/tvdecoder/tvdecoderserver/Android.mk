LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	main_tvdecoderserver.cpp 

LOCAL_MODULE_TAGS := eng

LOCAL_SHARED_LIBRARIES := \
	libutils \
	libbinder \
	libtvdecoderservice


LOCAL_MODULE:= tvdecoderserver

include $(BUILD_EXECUTABLE)
