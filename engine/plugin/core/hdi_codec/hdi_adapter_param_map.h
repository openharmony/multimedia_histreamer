/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if !defined(OHOS_LITE) && defined(VIDEO_SUPPORT)
#ifndef HISTREAMER_PLUGIN_CORE_HDI_ADAPTER_PARAMS_MAP_H
#define HISTREAMER_PLUGIN_CORE_HDI_ADAPTER_PARAMS_MAP_H

#include "hdf_base.h"

namespace OHOS {
namespace Media {
namespace Plugin {
const static std::pair<::CodecType, Plugin::CodecType> codecTypeMap[] = {
    {::CodecType::VIDEO_DECODER, Plugin::CodecType::VIDEO_DECODER},
    {::CodecType::VIDEO_ENCODER, Plugin::CodecType::VIDEO_ENCODER},
    {::CodecType::AUDIO_DECODER, Plugin::CodecType::AUDIO_DECODER},
    {::CodecType::AUDIO_ENCODER, Plugin::CodecType::AUDIO_ENCODER},
};

const static std::pair<int32_t, Status> retStatusMap[] = {
    {HDF_SUCCESS, Status::OK},
    {HDF_FAILURE, Status::ERROR_UNKNOWN},
    {HDF_ERR_NOT_SUPPORT, Status::ERROR_INVALID_OPERATION},
    {HDF_ERR_INVALID_PARAM, Status::ERROR_INVALID_PARAMETER},
    {HDF_ERR_NOT_SUPPORT, Status::ERROR_AGAIN},
    {HDF_ERR_MALLOC_FAIL, Status::ERROR_NO_MEMORY},
};

const static std::map<HDF_STATUS, std::string> hdfStatusMap = {
    {HDF_STATUS::HDF_SUCCESS, "HDF_SUCCESS"},
    {HDF_STATUS::HDF_FAILURE, "HDF_FAILURE"},
    {HDF_STATUS::HDF_ERR_NOT_SUPPORT, "HDF_ERR_NOT_SUPPORT"},
    {HDF_STATUS::HDF_ERR_INVALID_PARAM, "HDF_ERR_INVALID_PARAM"},
    {HDF_STATUS::HDF_ERR_INVALID_OBJECT, "HDF_ERR_INVALID_OBJECT"},
    {HDF_STATUS::HDF_ERR_MALLOC_FAIL, "HDF_ERR_MALLOC_FAIL"},
    {HDF_STATUS::HDF_ERR_TIMEOUT, "HDF_ERR_TIMEOUT"},
    {HDF_STATUS::HDF_ERR_THREAD_CREATE_FAIL, "HDF_ERR_THREAD_CREATE_FAIL"},
    {HDF_STATUS::HDF_ERR_QUEUE_FULL, "HDF_ERR_QUEUE_FULL"},
    {HDF_STATUS::HDF_ERR_DEVICE_BUSY, "HDF_ERR_DEVICE_BUSY"},
    {HDF_STATUS::HDF_ERR_IO, "HDF_ERR_IO"},
    {HDF_STATUS::HDF_ERR_BAD_FD, "HDF_ERR_BAD_FD"},
    {HDF_STATUS::HDF_ERR_NOPERM, "HDF_ERR_NOPERM"},
    {HDF_STATUS::HDF_BSP_ERR_OP, "HDF_BSP_ERR_OP"},
    {HDF_STATUS::HDF_ERR_BSP_PLT_API_ERR, "HDF_ERR_BSP_PLT_API_ERR"},
    {HDF_STATUS::HDF_PAL_ERR_DEV_CREATE, "HDF_PAL_ERR_DEV_CREATE"},
    {HDF_STATUS::HDF_PAL_ERR_INNER, "HDF_PAL_ERR_INNER"},
    {HDF_STATUS::HDF_DEV_ERR_NO_MEMORY, "HDF_DEV_ERR_NO_MEMORY"},
    {HDF_STATUS::HDF_DEV_ERR_NO_DEVICE, "HDF_DEV_ERR_NO_DEVICE"},
    {HDF_STATUS::HDF_DEV_ERR_NO_DEVICE_SERVICE, "HDF_DEV_ERR_NO_DEVICE_SERVICE"},
    {HDF_STATUS::HDF_DEV_ERR_DEV_INIT_FAIL, "HDF_DEV_ERR_DEV_INIT_FAIL"},
    {HDF_STATUS::HDF_DEV_ERR_PUBLISH_FAIL, "HDF_DEV_ERR_PUBLISH_FAIL"},
    {HDF_STATUS::HDF_DEV_ERR_ATTACHDEV_FAIL, "HDF_DEV_ERR_ATTACHDEV_FAIL"},
    {HDF_STATUS::HDF_DEV_ERR_NODATA, "HDF_DEV_ERR_NODATA"},
    {HDF_STATUS::HDF_DEV_ERR_NORANGE, "HDF_DEV_ERR_NORANGE"},
    {HDF_STATUS::HDF_DEV_ERR_OP, "HDF_DEV_ERR_OP"},
};

const static std::map<OMX_ERRORTYPE, std::string> omxStatusMap = {
    {OMX_ErrorPortUnresponsiveDuringStop, "OMX_ErrorPortUnresponsiveDuringStop"},
    {OMX_ErrorIncorrectStateTransition, "OMX_ErrorIncorrectStateTransition"},
    {OMX_ErrorIncorrectStateOperation, "OMX_ErrorIncorrectStateOperation"},
    {OMX_ErrorUnsupportedSetting, "OMX_ErrorUnsupportedSetting"},
    {OMX_ErrorUnsupportedIndex, "OMX_ErrorUnsupportedIndex"},
    {OMX_ErrorBadPortIndex, "OMX_ErrorBadPortIndex"},
    {OMX_ErrorPortUnpopulated, "OMX_ErrorPortUnpopulated"},
};

const static std::unordered_map<OMX_STATETYPE, std::string> omxStateToString = {
    {OMX_StateInvalid, "OMX_StateInvalid"},
    {OMX_StateLoaded, "OMX_StateLoaded"},
    {OMX_StateLoaded, "OMX_StateLoaded"},
    {OMX_StateIdle, "OMX_StateIdle"},
    {OMX_StateExecuting, "OMX_StateExecuting"},
    {OMX_StatePause, "OMX_StatePause"},
    {OMX_StateWaitForResources, "OMX_StateWaitForResources"},
    {OMX_StateKhronosExtensions, "OMX_StateKhronosExtensions"},
    {OMX_StateVendorStartUnused, "OMX_StateVendorStartUnused"},
    {OMX_StateMax, "OMX_StateMax"},
};
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PLUGIN_CORE_HDI_ADAPTER_PARAMS_MAP_H
#endif