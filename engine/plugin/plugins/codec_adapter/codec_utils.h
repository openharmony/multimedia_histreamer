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

#ifndef HISTREAMER_PLUGIN_PLUGINS_CODEC_UTILS_H
#define HISTREAMER_PLUGIN_PLUGINS_CODEC_UTILS_H

#include <iostream>
#include "codec_buffer_pool.h"
#include "display_type.h"
#include "OMX_Video.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace CodecAdapter {
std::string HdfStatus2String(int32_t status);

std::string OmxErrorType2String(uint32_t errorType);

Status TransHdiRetVal2Status(const int32_t& ret);

uint32_t Translate2omxFlagSet(uint64_t pluginFlags);

uint64_t Translate2PluginFlagSet(uint32_t omxBufFlag);
std::string OmxStateToString(OMX_STATETYPE state);

OMX_VIDEO_CODINGTYPE CodingTypeHstToHdi(const std::string& format);
OMX_COLOR_FORMATTYPE FormatHstToOmx(const VideoPixelFormat format);

uint32_t GetOmxBufferType(const MemoryType& bufMemType, bool isInput);
template <typename T>
inline void InitHdiParam(T& param, CompVerInfo& verInfo)
{
    memset_s(&param, sizeof(param), 0x0, sizeof(param));
    param.size = sizeof(param);
    param.version = verInfo.compVersion;
}

template <typename T>
inline void InitOmxParam(T& param, CompVerInfo& verInfo)
{
    memset_s(&param, sizeof(param), 0x0, sizeof(param));
    param.nSize = sizeof(param);
    param.nVersion = verInfo.compVersion;
}

template <typename T, typename U>
inline int32_t HdiSetParameter(T* component, uint32_t paramIndex, U& param)
{
    return component->SetParameter(component, paramIndex, reinterpret_cast<int8_t*>(&param), sizeof(param));
}

template <typename T, typename U>
inline int32_t HdiGetParameter(T* component, uint32_t paramIndex, U& param)
{
    return component->GetParameter(component, paramIndex, reinterpret_cast<int8_t*>(&param), sizeof(param));
}

template <typename T, typename U>
inline int32_t HdiSendCommand(T* component, OMX_COMMANDTYPE cmd, uint32_t param, U&& cmdData)
{
    return component->SendCommand(component, cmd, param, reinterpret_cast<int8_t*>(&cmdData), sizeof(cmdData));
}

template <typename T, typename U>
inline int32_t HdiFillThisBuffer(T* component, U* buffer)
{
    return component->FillThisBuffer(component, buffer);
}

template <typename T, typename U>
inline int32_t HdiEmptyThisBuffer(T* component, U* buffer)
{
    return component->EmptyThisBuffer(component, buffer);
}

} // namespace CodecAdapter
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PLUGIN_PLUGINS_CODEC_UTILS_H
#endif