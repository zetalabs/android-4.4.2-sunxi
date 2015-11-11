LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
#LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_TAGS := eng

LOCAL_SRC_FILES := TVDecoderService.cpp

LOCAL_C_INCLUDES:= $(TARGET_HARDWARE_INCLUDE)

LOCAL_SHARED_LIBRARIES:= \
	libutils \
	libbinder \
	libui \
	liblog \
	libcutils \
	libgui \
	libCedarX \
	libcedarxosal \
	libhardware \
	libmedia \
	libtvdecoder_client

ifeq ($(DLOPEN_LIBSECCAMERA),1)
LOCAL_SHARED_LIBRARIES+= libdl
endif


LOCAL_MODULE:= libtvdecoderservice

include $(BUILD_SHARED_LIBRARY)
