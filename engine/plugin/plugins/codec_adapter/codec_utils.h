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

#if !defined(OHOS_LITE) && defined(VIDEO_SUPPORT)

#ifndef HISTREAMER_PLUGIN_PLUGINS_CODEC_UTILS_H
#define HISTREAMER_PLUGIN_PLUGINS_CODEC_UTILS_H

#include <iostream>
#include "codec_buffer_pool.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace CodecAdapter {
std::string HdfStatus2String(int32_t status);

Status TransHdiRetVal2Status(const int32_t& ret);

uint32_t Translate2omxFlagSet(uint64_t pluginFlags);

uint64_t Translate2PluginFlagSet(uint32_t omxBufFlag);

template <typename T>
inline void InitHdiParam(T& param, CompVerInfo& verInfo)
{
    memset_s(&param, sizeof(param), 0x0, sizeof(param));
    param.size = sizeof(param);
    param.version.s.nVersionMajor = verInfo.compVersion.s.nVersionMajor;
}
} // namespace CodecAdapter
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PLUGIN_PLUGINS_CODEC_UTILS_H
#endif