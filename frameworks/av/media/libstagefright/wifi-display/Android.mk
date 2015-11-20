LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
        ANetworkSession.cpp             \
        MediaSender.cpp                 \
        Parameters.cpp                  \
        ParsedMessage.cpp               \
        rtp/RTPSender.cpp               \
        source/Converter.cpp            \
        source/MediaPuller.cpp          \
        source/PlaybackSession.cpp      \
        source/RepeaterSource.cpp       \
        source/TSPacketizer.cpp         \
        source/WifiDisplaySource.cpp    \
        VideoFormats.cpp                \
        wfdsink/XLinearRegression.cpp   \
        wfdsink/RTPSource.cpp           \
        wfdsink/RTPJitterBuffer.cpp     \
        wfdsink/WfdSink.cpp             \
        wfdsink/MiracastSink.cpp        \

LOCAL_C_INCLUDES:= \
        $(TOP)/frameworks/av/media/libstagefright \
        $(TOP)/frameworks/native/include/media/openmax \
        $(TOP)/frameworks/av/media/libstagefright/mpeg2ts \

LOCAL_SHARED_LIBRARIES:= \
        libbinder                       \
        libcutils                       \
        liblog                          \
        libgui                          \
        libmedia                        \
        libstagefright                  \
        libstagefright_foundation       \
        libui                           \
        libutils                        \

LOCAL_MODULE:= libstagefright_wfd

LOCAL_MODULE_TAGS:= optional

include $(BUILD_SHARED_LIBRARY)
