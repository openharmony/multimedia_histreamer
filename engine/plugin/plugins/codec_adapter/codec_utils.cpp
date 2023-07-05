/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#if defined(VIDEO_SUPPORT)

#define HST_LOG_TAG "CodecUtils"

#include "codec_utils.h"
#include <map>
#include "display_type.h"
#include "foundation/log.h"
#include "hdf_base.h"
#include "OMX_Core.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace CodecAdapter {
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

const static std::map<OMX_ERRORTYPE, std::string> omxErrorTypeMap = {
    {OMX_ErrorNone, "OMX_ErrorNone"},
    {OMX_ErrorInsufficientResources, "OMX_ErrorInsufficientResources"},
    {OMX_ErrorUndefined, "OMX_ErrorUndefined"},
    {OMX_ErrorInvalidComponentName, "OMX_ErrorInvalidComponentName"},
    {OMX_ErrorComponentNotFound, "OMX_ErrorComponentNotFound"},
    {OMX_ErrorInvalidComponent, "OMX_ErrorInvalidComponent"},
    {OMX_ErrorBadParameter, "OMX_ErrorBadParameter"},
    {OMX_ErrorNotImplemented, "OMX_ErrorNotImplemented"},
    {OMX_ErrorUnderflow, "OMX_ErrorUnderflow"},
    {OMX_ErrorOverflow, "OMX_ErrorOverflow"},
    {OMX_ErrorHardware, "OMX_ErrorHardware"},
    {OMX_ErrorInvalidState, "OMX_ErrorInvalidState"},
    {OMX_ErrorStreamCorrupt, "OMX_ErrorStreamCorrupt"},
    {OMX_ErrorPortsNotCompatible, "OMX_ErrorPortsNotCompatible"},
    {OMX_ErrorResourcesLost, "OMX_ErrorResourcesLost"},
    {OMX_ErrorNoMore, "OMX_ErrorNoMore"},
    {OMX_ErrorVersionMismatch, "OMX_ErrorVersionMismatch"},
    {OMX_ErrorNotReady, "OMX_ErrorNotReady"},
    {OMX_ErrorTimeout, "OMX_ErrorTimeout"},
    {OMX_ErrorSameState, "OMX_ErrorSameState"},
    {OMX_ErrorResourcesPreempted, "OMX_ErrorResourcesPreempted"},
    {OMX_ErrorPortUnresponsiveDuringAllocation, "OMX_ErrorPortUnresponsiveDuringAllocation"},
    {OMX_ErrorPortUnresponsiveDuringDeallocation, "OMX_ErrorPortUnresponsiveDuringDeallocation"},
    {OMX_ErrorPortUnresponsiveDuringStop, "OMX_ErrorPortUnresponsiveDuringStop"},
    {OMX_ErrorIncorrectStateTransition, "OMX_ErrorIncorrectStateTransition"},
    {OMX_ErrorIncorrectStateOperation, "OMX_ErrorIncorrectStateOperation"},
    {OMX_ErrorUnsupportedSetting, "OMX_ErrorUnsupportedSetting"},
    {OMX_ErrorUnsupportedIndex, "OMX_ErrorUnsupportedIndex"},
    {OMX_ErrorBadPortIndex, "OMX_ErrorBadPortIndex"},
    {OMX_ErrorPortUnpopulated, "OMX_ErrorPortUnpopulated"},
    {OMX_ErrorComponentSuspended, "OMX_ErrorComponentSuspended"},
    {OMX_ErrorDynamicResourcesUnavailable, "OMX_ErrorDynamicResourcesUnavailable"},
    {OMX_ErrorMbErrorsInFrame, "OMX_ErrorMbErrorsInFrame"},
    {OMX_ErrorFormatNotDetected, "OMX_ErrorFormatNotDetected"},
    {OMX_ErrorContentPipeOpenFailed, "OMX_ErrorContentPipeOpenFailed"},
    {OMX_ErrorContentPipeCreationFailed, "OMX_ErrorContentPipeCreationFailed"},
    {OMX_ErrorSeperateTablesUsed, "OMX_ErrorSeperateTablesUsed"},
    {OMX_ErrorTunnelingUnsupported, "OMX_ErrorTunnelingUnsupported"},
    {OMX_ErrorKhronosExtensions, "OMX_ErrorKhronosExtensions"},
    {OMX_ErrorVendorStartUnused, "OMX_ErrorVendorStartUnused"},
    {OMX_ErrorMax, "OMX_ErrorMax"},
};

const static std::pair<int32_t, Status> retStatusMap[] = {
    {HDF_SUCCESS, Status::OK},
    {HDF_FAILURE, Status::ERROR_UNKNOWN},
    {HDF_ERR_NOT_SUPPORT, Status::ERROR_INVALID_OPERATION},
    {HDF_ERR_INVALID_PARAM, Status::ERROR_INVALID_PARAMETER},
    {HDF_ERR_MALLOC_FAIL, Status::ERROR_NO_MEMORY},
};

std::string HdfStatus2String(int32_t status)
{
    auto it = hdfStatusMap.find(static_cast<HDF_STATUS>(status));
    if (it != hdfStatusMap.end()) {
        return it->second;
    }
    MEDIA_LOG_E("Not find value: " PUBLIC_LOG_D32 " in hdfStatusMap", status);
    return "null";
}

std::string OmxErrorType2String(uint32_t errorType)
{
    auto it = omxErrorTypeMap.find(static_cast<OMX_ERRORTYPE>(errorType));
    if (it != omxErrorTypeMap.end()) {
        return it->second;
    }
    MEDIA_LOG_E("Not find value: " PUBLIC_LOG_U32 " in omxErrorTypeMap", errorType);
    return omxErrorTypeMap.at(OMX_ErrorNone);
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

Status TransHdiRetVal2Status(const int32_t& ret)
{
    Status status = Status::ERROR_UNKNOWN;
    TranslatesByMap(ret, status, retStatusMap, sizeof(retStatusMap) / sizeof(retStatusMap[0]));
    return status;
}

uint32_t Translate2omxFlagSet(uint64_t pluginFlags)
{
    uint32_t ret = 0;
    if (pluginFlags == BUFFER_FLAG_EOS) {
        ret = OMX_BUFFERFLAG_EOS;
    }
    return ret;
}

uint64_t Translate2PluginFlagSet(uint32_t omxBufFlag)
{
    uint64_t ret = 0;
    if (omxBufFlag == OMX_BUFFERFLAG_EOS) {
        ret = BUFFER_FLAG_EOS;
    }
    return ret;
}

static const std::map<std::string, OMX_VIDEO_CODINGTYPE> compressHstOmx = {
    {MEDIA_MIME_VIDEO_H264, OMX_VIDEO_CodingAVC},
    {MEDIA_MIME_VIDEO_H265, static_cast<OMX_VIDEO_CODINGTYPE>(CODEC_OMX_VIDEO_CodingHEVC)}
};

OMX_VIDEO_CODINGTYPE CodingTypeHstToHdi(const std::string& format)
{
    if (compressHstOmx.find(format) != compressHstOmx.end()) {
        return compressHstOmx.at(format);
    }
    return OMX_VIDEO_CodingUnused;
}

static const std::map<VideoPixelFormat, OMX_COLOR_FORMATTYPE> formatHstOmx = {
    {VideoPixelFormat::NV12, OMX_COLOR_FormatYUV420SemiPlanar},
    {VideoPixelFormat::NV21, OMX_COLOR_FormatYUV420SemiPlanar},
    {VideoPixelFormat::BGRA, OMX_COLOR_Format32bitBGRA8888},
    {VideoPixelFormat::RGBA, OMX_COLOR_Format32bitARGB8888},
    {VideoPixelFormat::YUV420P, OMX_COLOR_FormatYUV420Planar},
};

OMX_COLOR_FORMATTYPE FormatHstToOmx(const VideoPixelFormat format)
{
    if (formatHstOmx.find(format) != formatHstOmx.end()) {
        return formatHstOmx.at(format);
    }
    MEDIA_LOG_W("Unknow VideoPixelFormat: " PUBLIC_LOG_U32, static_cast<uint32_t>(format));
    return OMX_COLOR_FormatUnused;
}

static const std::map<OMX_STATETYPE, std::string> omxStateMap = {
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

std::string OmxStateToString(OMX_STATETYPE state)
{
    auto iter = omxStateMap.find(state);
    if (iter != omxStateMap.end()) {
        return iter->second;
    }
    MEDIA_LOG_W("Not find value, maybe update the map");
    return "null";
}

uint32_t GetOmxBufferType(const MemoryType& bufMemType, bool isInput)
{
    uint32_t bufferType;
    switch (bufMemType) {
        case MemoryType::SHARE_MEMORY:
            bufferType = CODEC_BUFFER_TYPE_AVSHARE_MEM_FD;
            break;
        case MemoryType::SURFACE_BUFFER:
            bufferType = CODEC_BUFFER_TYPE_HANDLE;
            if (isInput) {
                bufferType = CODEC_BUFFER_TYPE_DYNAMIC_HANDLE;
            }
            break;
        default:
            bufferType = CODEC_BUFFER_TYPE_INVALID;
            MEDIA_LOG_E("MemoryType Error");
    }
    return bufferType;
}
} // namespace CodecAdapter
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif