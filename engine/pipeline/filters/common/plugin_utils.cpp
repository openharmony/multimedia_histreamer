/*
 * Copyright (c) 2021-2021 Huawei Device Co., Ltd.
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

#include "plugin_utils.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
/**
 * translate plugin error into pipeline error code
 * @param pluginError
 * @return
 */
OHOS::Media::ErrorCode TranslatePluginStatus(Plugin::Status pluginError)
{
    if (pluginError != OHOS::Media::Plugin::Status::OK) {
        return OHOS::Media::ErrorCode::UNKNOWN_ERROR;
    }
    return OHOS::Media::ErrorCode::SUCCESS;
}
bool TranslateIntoParameter(const int &key, OHOS::Media::Plugin::Tag &tag)
{
    if (key < static_cast<int32_t>(OHOS::Media::Plugin::Tag::INVALID)) {
        return false;
    }
    tag = static_cast<OHOS::Media::Plugin::Tag>(key);
    return true;
}

template <typename T>
ErrorCode FindPluginAndUpdate(const std::shared_ptr<const OHOS::Media::Meta> &inMeta,
    Plugin::PluginType pluginType, std::shared_ptr<T>& plugin, std::shared_ptr<Plugin::PluginInfo>& pluginInfo,
    std::function<std::shared_ptr<T>(const std::string&)> pluginCreator)
{
    uint32_t maxRank = 0;
    std::shared_ptr<Plugin::PluginInfo> info;
    auto pluginNames = Plugin::PluginManager::Instance().ListPlugins(pluginType);
    for (const auto &name:pluginNames) {
        auto tmpInfo = Plugin::PluginManager::Instance().GetPluginInfo(pluginType, name);
        if (CompatibleWith(tmpInfo->inCaps, *inMeta) && tmpInfo->rank > maxRank) {
            info = tmpInfo;
        }
    }
    if (info == nullptr) {
        return PLUGIN_NOT_FOUND;
    }

    // try to reuse the plugin if their name are the same
    if (plugin != nullptr && pluginInfo != nullptr) {
        if (info->name == pluginInfo->name) {
            if (TranslatePluginStatus(plugin->Reset()) == SUCCESS) {
                return SUCCESS;
            }
        }
        plugin->Deinit();
    }
    plugin = pluginCreator(info->name);
    if (plugin == nullptr) {
        return PLUGIN_NOT_FOUND;
    }
    pluginInfo = info;
    return SUCCESS;
}

template
ErrorCode FindPluginAndUpdate(const std::shared_ptr<const OHOS::Media::Meta>&, Plugin::PluginType,
    std::shared_ptr<Plugin::Codec>&, std::shared_ptr<Plugin::PluginInfo>&,
    std::function<std::shared_ptr<Plugin::Codec>(const std::string&)>);
template
ErrorCode FindPluginAndUpdate(const std::shared_ptr<const OHOS::Media::Meta>&, Plugin::PluginType,
    std::shared_ptr<Plugin::AudioSink>&, std::shared_ptr<Plugin::PluginInfo>&,
    std::function<std::shared_ptr<Plugin::AudioSink>(const std::string&)>);
}
}
}
