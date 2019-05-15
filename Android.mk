#
# Copyright (C) 2019 GlobalLogic
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

################################################################################
# Android ipl image pack tools for host                                        #
################################################################################
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS += $(CFLAGS) -Wno-packed -DPACK_IPL_EMCC

LOCAL_SRC_FILES += host.c common.c

LOCAL_C_INCLUDES += $(LOCAL_PATH)
# Include offsets from IPLs for making it always consistant with pack_ipl_emmc
LOCAL_C_INCLUDES += ./device/renesas/bootloaders/ipl/tools/dummy_create/

LOCAL_STATIC_LIBRARIES := libcrypto_static

LOCAL_CXX_STL := libc++_static

LOCAL_FORCE_STATIC_EXECUTABLE := true

LOCAL_MODULE := pack_ipl_emmc

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_PATH := $(HOST_ROOT_OUT)

include $(BUILD_HOST_EXECUTABLE)
