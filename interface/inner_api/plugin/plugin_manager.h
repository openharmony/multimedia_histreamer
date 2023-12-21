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

#ifndef HISTREAMER_PLUGIN_MANAGER_H
#define HISTREAMER_PLUGIN_MANAGER_H

#include "cpp_ext/type_cast_ext.h"
#include "plugin_info.h"
#include "plugin/plugin_register.h"
#include "generic_plugin.h"

namespace OHOS {
namespace Media {
namespace Plugins {
class PluginManager {
public:
    PluginManager(const PluginManager&) = delete;
    PluginManager operator=(const PluginManager&) = delete;
    ~PluginManager() = default;
    static PluginManager& Instance()
    {
        static PluginManager impl;
        return impl;
    }
    std::shared_ptr<PluginRegister> GetPluginRegister()
    {
        return pluginRegister_;
    }

    std::vector<std::string> ListPlugins(PluginType pluginType, CodecMode preferredCodecMode = CodecMode::HARDWARE);

    std::shared_ptr<PluginInfo> GetPluginInfo(PluginType type, const std::string& name);

    std::shared_ptr<PluginBase> CreatePlugin(const std::string& name, PluginType type);

    template<typename T, typename U>
    std::shared_ptr<T> CreateGenericPlugin(const std::string& name)
    {
        return CreatePlugin<T, U>(name, PluginType::GENERIC_PLUGIN);
    }

    int32_t Sniffer(const std::string& name, std::shared_ptr<DataSource> source);

    void RegisterGenericPlugin(const GenericPluginDef& pluginDef);

    void RegisterGenericPlugins(const std::vector<GenericPluginDef>& vecPluginDef);
private:
    PluginManager();

    void Init();

    template<typename T, typename U>
    std::shared_ptr<T> CreatePlugin(const std::string& name, PluginType pluginType)
    {
        auto regInfo = pluginRegister_->GetPluginRegInfo(pluginType, name);
        if (!regInfo) {
            return {};
        }
        auto plugin = regInfo->creator(name);
        if (!plugin) {
            return {};
        }
        return std::shared_ptr<T>(plugin);
    }

private:
    std::shared_ptr<PluginRegister> pluginRegister_;
};
} // namespace Plugins
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PLUGIN_MANAGER_H
