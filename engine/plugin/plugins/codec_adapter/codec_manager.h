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

#ifndef HISTREAMER_PLUGIN_PLUGINS_CODEC_MANAGER_H
#define HISTREAMER_PLUGIN_PLUGINS_CODEC_MANAGER_H

#include "plugin/interface/codec_plugin.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace CodecAdapter {
struct CodecCapability {
    PluginType pluginType;    ///< plugin type, For details, @see PluginType.
    std::string pluginMime;   ///< plugin mime
    std::string name;         ///< plugin name
    CapabilitySet inCaps{};   ///< Plug-in input capability, For details, @see Capability.
    CapabilitySet outCaps{};  ///< Plug-in output capability, For details, @see Capability.
};

class CodecManager {
public:
    virtual ~CodecManager() = default;

    virtual int32_t CreateComponent(const Plugin::Any& component, uint32_t& id, const std::string& name,
                                    const Plugin::Any& appData, const Plugin::Any& callbacks) = 0;

    virtual int32_t DestroyComponent(const Plugin::Any& component, uint32_t id) = 0;

    virtual Status RegisterCodecPlugins(const std::shared_ptr<OHOS::Media::Plugin::Register>& reg) = 0;

    virtual Status UnRegisterCodecPlugins() = 0;
};
} // namespace CodecAdapter
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PLUGIN_PLUGINS_CODEC_MANAGER_H
#endif