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

#include "plugin/plugin_manager.h"
#include <utility>
#include "plugin/plugin_register.h"

namespace OHOS {
namespace Media {
namespace Plugin {
PluginManager::PluginManager()
{
    Init();
}

std::vector<std::string> PluginManager::ListPlugins(PluginType pluginType, CodecMode preferredCodecMode)
{
    return pluginRegister_->ListPlugins(pluginType, preferredCodecMode);
}

std::shared_ptr<PluginInfo> PluginManager::GetPluginInfo(PluginType type, const std::string& name)
{
    auto regInfo = pluginRegister_->GetPluginRegInfo(type, name);
    if (regInfo && regInfo->info && regInfo->info->pluginType == type) {
        return regInfo->info;
    }
    return {};
}

int32_t PluginManager::Sniffer(const std::string& name, std::shared_ptr<DataSource> source)
{
    if (!source) {
        return 0;
    }
    auto regInfo = pluginRegister_->GetPluginRegInfo(PluginType::DEMUXER, name);
    if (!regInfo) {
        return 0;
    }
    if (regInfo->info->pluginType == PluginType::DEMUXER) {
        return regInfo->sniffer(name, source);
    }
    return 0;
}

void PluginManager::Init()
{
    pluginRegister_ = std::make_shared<PluginRegister>();
    pluginRegister_->RegisterPlugins();
}

std::shared_ptr<PluginBase> PluginManager::CreatePlugin(const std::string& name, PluginType type)
{
    return CreatePlugin<PluginBase, PluginBase>(name, type);
}

void PluginManager::RegisterGenericPlugin(const GenericPluginDef& pluginDef)
{
    pluginRegister_->RegisterGenericPlugin(pluginDef);
}

void PluginManager::RegisterGenericPlugins(const std::vector<GenericPluginDef>& vecPluginDef)
{
    pluginRegister_->RegisterGenericPlugins(vecPluginDef);
}
} // namespace Plugin
} // namespace Media
} // namespace OHOS