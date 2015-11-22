#
# Copyright (C) 2013 Google Inc.
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
#

LOCAL_PATH := $(call my-dir)

# =============================================

include $(CLEAR_VARS)

LOCAL_MODULE := c_fst

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/usr/srec/en-US
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/srec/en-US
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE := clg

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/usr/srec/en-US
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/srec/en-US
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE := commands.abnf

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/usr/srec/en-US
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/srec/en-US
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE := compile_grammar.config

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/usr/srec/en-US
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/srec/en-US
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE := contacts.abnf

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/usr/srec/en-US
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/srec/en-US
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE := dict

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/usr/srec/en-US
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/srec/en-US
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE := dictation.config

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/usr/srec/en-US
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/srec/en-US
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE := dnn

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/usr/srec/en-US
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/srec/en-US
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE := endpointer_dictation.config

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/usr/srec/en-US
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/srec/en-US
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE := endpointer_voicesearch.config

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/usr/srec/en-US
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/srec/en-US
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE := ep_acoustic_model

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/usr/srec/en-US
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/srec/en-US
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE := g2p_fst

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/usr/srec/en-US
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/srec/en-US
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE := grammar.config

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/usr/srec/en-US
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/srec/en-US
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE := hclg_shotword

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/usr/srec/en-US
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/srec/en-US
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE := hmm_symbols

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/usr/srec/en-US
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/srec/en-US
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE := hmmlist

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/usr/srec/en-US
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/srec/en-US
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE := hotword.config

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/usr/srec/en-US
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/srec/en-US
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE := hotword_classifier

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/usr/srec/en-US
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/srec/en-US
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE := hotword_normalizer

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/usr/srec/en-US
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/srec/en-US
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE := hotword_prompt.txt

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/usr/srec/en-US
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/srec/en-US
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE := hotword_word_symbols

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/usr/srec/en-US
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/srec/en-US
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE := metadata

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/usr/srec/en-US
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/srec/en-US
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE := norm_fst

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/usr/srec/en-US
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/srec/en-US
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE := normalizer

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/usr/srec/en-US
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/srec/en-US
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE := offensive_word_normalizer

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/usr/srec/en-US
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/srec/en-US
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE := phone_state_map

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/usr/srec/en-US
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/srec/en-US
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE := phonelist

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/usr/srec/en-US
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/srec/en-US
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE := rescoring_lm

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/usr/srec/en-US
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/srec/en-US
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE := wordlist

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/usr/srec/en-US
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/srec/en-US
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# =============================================
