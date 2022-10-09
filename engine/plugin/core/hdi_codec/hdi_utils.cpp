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
#define HST_LOG_TAG "HdiUtils"

#include "hdi_utils.h"
#include "display_type.h"
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

const static std::unordered_map<OMX_STATETYPE, std::string> omxStateMap = {
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

const std::map<VideoPixelFormat, PixelFormat> pixelFormatMap = {
    {VideoPixelFormat::YUV420P, PIXEL_FMT_YCBCR_420_P},
    {VideoPixelFormat::NV12, PIXEL_FMT_YCBCR_420_SP},
    {VideoPixelFormat::NV21, PIXEL_FMT_YCRCB_420_SP},
    {VideoPixelFormat::YUYV422, PIXEL_FMT_YUYV_422_PKG},
    {VideoPixelFormat::YUV422P, PIXEL_FMT_YUV_422_I},
    {VideoPixelFormat::RGBA, PIXEL_FMT_RGBA_8888},
    {VideoPixelFormat::BGRA, PIXEL_FMT_BGRA_8888},
    {VideoPixelFormat::RGB24, PIXEL_FMT_RGB_888},
};

std::string OmxStateToString(OMX_STATETYPE state)
{
    auto iter = omxStateMap.find(state);
    if (iter != omxStateMap.end()) {
        return iter->second;
    }
    MEDIA_LOG_W("Not find value, maybe update the map");
    return "null";
}

std::string HdfStatus2String(int32_t status)
{
    auto it1 = hdfStatusMap.find(static_cast<HDF_STATUS>(status));
    auto it2 = omxStatusMap.find(static_cast<OMX_ERRORTYPE>(status));
    if (it1 != hdfStatusMap.end()) {
        return it1->second;
    } else if (it2 != omxStatusMap.end()) {
        return it2->second;
    }
    MEDIA_LOG_W("Not find value, maybe update the map");
    return "null";
}

std::string PortIndex2String(PortIndex portIndex)
{
    std::map<PortIndex, std::string> portIndexMap = {
        {PortIndex::PORT_INDEX_INPUT, "PORT_INDEX_INPUT"},
        {PortIndex::PORT_INDEX_OUTPUT, "PORT_INDEX_OUTPUT"},
    };
    auto it = portIndexMap.find(portIndex);
    if (it != portIndexMap.end()) {
        return it->second;
    }
    MEDIA_LOG_W("Not find value, maybe update the map");
    return "null";
}

template<typename T, typename U>
bool TranslatesByMap(const T& t, U& u, const std::pair<T, U>* transMap, size_t mapSize)
{
    for (size_t cnt = 0; cnt < mapSize; cnt++) {
        if (t == transMap[cnt].first) {
            u = transMap[cnt].second;
            return true;
        }
    }
    return false;
}

Status TranslateRets(const int32_t& ret)
{
    Status status = Status::ERROR_UNKNOWN;
    TranslatesByMap(ret, status, retStatusMap, sizeof(retStatusMap) / sizeof(retStatusMap[0]));
    return status;
}

/**
 * if T and U are the same types, always assigns directly and returns true
 *
 * @tparam T
 * @tparam U
 * @param src
 * @param dest
 * @return
 */
template<typename T, typename U,
        typename std::enable_if<std::is_same<typename std::decay<T>::type,
        typename std::decay<U>::type>::value, bool>::type = true>
bool Translates(const T& src, U& dest)
{
    dest = src;
    return true;
}

template<>
bool Translates(const ::CodecType& codecType, Plugin::CodecType& pluginCodecType)
{
    return TranslatesByMap(codecType, pluginCodecType, codecTypeMap, sizeof(codecTypeMap) / sizeof(codecTypeMap[0]));
}

uint64_t Translate2PluginFlagSet(uint32_t omxBufFlag)
{
    uint64_t ret = 0;
    if (omxBufFlag == OMX_BUFFERFLAG_EOS) {
        ret = BUFFER_FLAG_EOS;
    }
    return ret;
}

uint32_t Translate2omxFlagSet(uint64_t pluginFlags)
{
    uint32_t ret = 0;
    if (pluginFlags == BUFFER_FLAG_EOS) {
        ret = OMX_BUFFERFLAG_EOS;
    }
    return ret;
}

VideoPixelFormat ConvertPixelFormatFromHdi(int32_t HdiPixelFormat)
{
    // Histreamer pixel format to HDI pixel format
    auto iter = std::find_if(pixelFormatMap.begin(), pixelFormatMap.end(),
                             [&] (const std::pair<VideoPixelFormat, PixelFormat>& tmp) -> bool {
                                 return tmp.second == HdiPixelFormat;
                             });
    return iter == pixelFormatMap.end() ? VideoPixelFormat::UNKNOWN : iter->first;
}
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif