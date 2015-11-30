LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
#LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_TAGS := eng

LOCAL_SRC_FILES := TVDecoderClient.cpp ITVDecoderService.cpp

LOCAL_SHARED_LIBRARIES += \
	libbinder \
       	libcutils \
       	libutils \
       	libgui

ifeq ($(DLOPEN_LIBSECCAMERA),1)
LOCAL_SHARED_LIBRARIES+= libdl
endif

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE:= libtvdecoder_client

include $(BUILD_SHARED_LIBRARY)
