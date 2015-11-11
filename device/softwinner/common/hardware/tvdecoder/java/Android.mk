LOCAL_PATH:= $(call my-dir)

# the library
# ============================================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_MODULE:= softwinner.tv
LOCAL_MODULE_TAGS := eng

#LOCAL_JAVA_LIBRARIES := softwinner.tv.TVDecoder

include $(BUILD_STATIC_JAVA_LIBRARY)