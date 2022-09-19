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
#include "hdi_adapter_param_map.h"

namespace OHOS {
namespace Media {
namespace Plugin {
std::string TransHdfStatus2String(int32_t status)
{
    auto it1 = hdfStatusMap.find(static_cast<HDF_STATUS>(status));
    auto it2 = omxStatusMap.find(static_cast<OMX_ERRORTYPE>(status));
    if (it1 != hdfStatusMap.end()) {
        return it1->second;
    } else if (it2 != omxStatusMap.end()) {
        return it2->second;
    }
    return "null";
}

std::string TransPortIndex2String(PortIndex portIndex)
{
    std::map<PortIndex, std::string> portIndexMap = {
        {PortIndex::PORT_INDEX_INPUT, "PORT_INDEX_INPUT"},
        {PortIndex::PORT_INDEX_OUTPUT, "PORT_INDEX_OUTPUT"},
    };
    auto it = portIndexMap.find(portIndex);
    if (it != portIndexMap.end()) {
        return it->second;
    }
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
    std::map<VideoPixelFormat, PixelFormat> pixelFormatMap = { // 是否可以改为常量
        {VideoPixelFormat::NV21, PIXEL_FMT_YCBCR_420_SP}, // 需要补充完整
        {VideoPixelFormat::NV12, PIXEL_FMT_YCRCB_420_SP},
    };
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