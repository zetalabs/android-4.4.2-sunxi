# Copyright (C) 2010 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)

#########################

include $(CLEAR_VARS)
LOCAL_MODULE := libjni_eglfence_awgallery.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_CLASS := lib
include $(BUILD_PREBUILT)

#######################

include $(CLEAR_VARS)
LOCAL_MODULE := libjni_mosaic.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_CLASS := lib
include $(BUILD_PREBUILT)

#######################

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := eng optional
LOCAL_PREBUILT_LIBS := libcheckfile.so
include $(BUILD_MULTI_PREBUILT)

#########################################
# GooglePinyin
#########################################
include $(CLEAR_VARS)

LOCAL_PREBUILT_LIBS := libjni_hmm_shared_engine.so \
	libjni_googlepinyinime_latinime_5.so \
	libjni_googlepinyinime_5.so \
	libjni_delight.so \
	libgnustl_shared.so

LOCAL_MODULE_TAGS := optional

include $(BUILD_MULTI_PREBUILT)
