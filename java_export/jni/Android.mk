# Copyright (C) 2011 The Android Open Source Project
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

TOP_DIR :=$(LOCAL_PATH)/../..


include $(CLEAR_VARS)

LOCAL_CFLAGS += -DEV_UNIX -DEV_ANDROID


LOCAL_MODULE    := EVdirver
LOCAL_C_INCLUDES+=$(TOP_DIR)/ytimer
LOCAL_C_INCLUDES+=$(TOP_DIR)/yserialport
LOCAL_C_INCLUDES+=$(TOP_DIR)/general
LOCAL_C_INCLUDES+=$(TOP_DIR)/db_api
LOCAL_C_INCLUDES+=$(TOP_DIR)/cpp_export


LOCAL_SRC_FILES += $(LOCAL_PATH)/../com_easivend_evprotocol_EVprotocol.c
LOCAL_SRC_FILES += $(TOP_DIR)/ytimer/timer.c
LOCAL_SRC_FILES += $(TOP_DIR)/yserialport/unix_yocserialport.c
LOCAL_SRC_FILES += $(TOP_DIR)/yserialport/yoc_serialbase.c
LOCAL_SRC_FILES += $(TOP_DIR)/yserialport/yoc_serialport.c
LOCAL_SRC_FILES += $(TOP_DIR)/general/ev_config.c
LOCAL_SRC_FILES += $(TOP_DIR)/cpp_export/db_driver.c
LOCAL_SRC_FILES += $(TOP_DIR)/db_api/mdb_api.c
LOCAL_SRC_FILES += $(TOP_DIR)/db_api/bento_api.c
LOCAL_SRC_FILES += $(TOP_DIR)/db_api/column_api.c
LOCAL_SRC_FILES += $(TOP_DIR)/db_api/db_serial.c
LOCAL_SRC_FILES += $(TOP_DIR)/db_api/db_json.c
LOCAL_SRC_FILES += $(TOP_DIR)/general/cJSON.c
LOCAL_SRC_FILES += $(TOP_DIR)/general/LOGC.c

LOCAL_MODULE_FILENAME := libEVdirver
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)
