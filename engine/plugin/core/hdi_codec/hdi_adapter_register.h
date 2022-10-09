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
#ifndef HISTREAMER_PLUGIN_CORE_CODEC_ADAPTER_REGISTER_H
#define HISTREAMER_PLUGIN_CORE_CODEC_ADAPTER_REGISTER_H

namespace OHOS {
namespace Media {
namespace Plugin {
extern void RegisterOneCodecPackage(const std::shared_ptr<OHOS::Media::Plugin::Register>& reg,
                                    const std::string& packageName);
extern void UnRegisterOneCodecPackage(const std::string& packageName);

void RegisterHdiCodecPackages(std::shared_ptr<OHOS::Media::Plugin::Register> reg)
{
    RegisterOneCodecPackage(reg, "HdiAdapter");
}

void UnRegisterHdiCodecPackage()
{
    UnRegisterOneCodecPackage("HdiAdapter");
}
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PLUGIN_CORE_CODEC_ADAPTER_REGISTER_H
#endif