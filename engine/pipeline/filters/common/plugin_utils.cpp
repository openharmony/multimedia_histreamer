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
const static std::map<Plugin::Status, ErrorCode> g_transTable = {
        {Plugin::Status::END_OF_STREAM, ErrorCode::END_OF_STREAM},
        {Plugin::Status::OK, ErrorCode::SUCCESS},
        {Plugin::Status::NO_ERROR, ErrorCode::SUCCESS},
        {Plugin::Status::ERROR_UNKNOWN, ErrorCode::ERROR_UNKNOWN},
        {Plugin::Status::ERROR_PLUGIN_ALREADY_EXISTS,ErrorCode::ERROR_UNKNOWN},
        {Plugin::Status::ERROR_INCOMPATIBLE_VERSION, ErrorCode::ERROR_UNKNOWN},
        {Plugin::Status::ERROR_NO_MEMORY, ErrorCode::ERROR_NO_MEMORY},
        {Plugin::Status::ERROR_WRONG_STATE,ErrorCode::ERROR_INVALID_OPERATION},
        {Plugin::Status::ERROR_UNIMPLEMENTED, ErrorCode::ERROR_UNIMPLEMENTED},
        {Plugin::Status::ERROR_INVALID_PARAMETER, ErrorCode::ERROR_INVALID_PARAMETER_VALUE},
        {Plugin::Status::ERROR_INVALID_DATA, ErrorCode::ERROR_UNKNOWN},
        {Plugin::Status::ERROR_MISMATCHED_TYPE, ErrorCode::ERROR_INVALID_PARAMETER_TYPE},
        {Plugin::Status::ERROR_TIMED_OUT, ErrorCode::ERROR_TIMED_OUT},
        {Plugin::Status::ERROR_UNSUPPORTED_FORMAT, ErrorCode::ERROR_UNSUPPORTED_FORMAT},
        {Plugin::Status::ERROR_NOT_ENOUGH_DATA,ErrorCode::ERROR_UNKNOWN},
        {Plugin::Status::ERROR_NOT_EXISTED, ErrorCode::ERROR_NOT_EXISTED}
};
/**
 * translate plugin error into pipeline error code
 * @param pluginError
 * @return
 */
OHOS::Media::ErrorCode TranslatePluginStatus(Plugin::Status pluginError)
{
    auto ite = g_transTable.find(pluginError);
    if (ite == g_transTable.end()) {
        return ErrorCode::ERROR_UNKNOWN;
    }
    return ite->second;
}
bool TranslateIntoParameter(const int& key, OHOS::Media::Plugin::Tag& tag)
{
    if (key < static_cast<int32_t>(OHOS::Media::Plugin::Tag::INVALID)) {
        return false;
    }
    tag = static_cast<OHOS::Media::Plugin::Tag>(key);
    return true;
}

std::vector<std::pair<std::shared_ptr<Plugin::PluginInfo>, Plugin::Capability>>
    FindAvailablePlugins(const Plugin::Capability& upStreamCaps, Plugin::PluginType pluginType)
{
    auto pluginNames = Plugin::PluginManager::Instance().ListPlugins(pluginType);
    std::vector<std::pair<std::shared_ptr<Plugin::PluginInfo>, Plugin::Capability>> infos;
    for (const auto & name : pluginNames) {
        auto tmpInfo = Plugin::PluginManager::Instance().GetPluginInfo(pluginType, name);
        Capability cap;
        if (ApplyCapabilitySet(upStreamCaps, tmpInfo->inCaps, cap)) {
            infos.emplace_back(tmpInfo, cap);
        }
    }
    return infos;
}

template <typename T>
ErrorCode FindPluginAndUpdate(const std::shared_ptr<const Plugin::Meta>& inMeta, Plugin::PluginType pluginType,
                              std::shared_ptr<T>& plugin, std::shared_ptr<Plugin::PluginInfo>& pluginInfo,
                              std::function<std::shared_ptr<T>(const std::string&)> pluginCreator)
{
    uint32_t maxRank = 0;
    std::shared_ptr<Plugin::PluginInfo> info;
    auto pluginNames = Plugin::PluginManager::Instance().ListPlugins(pluginType);
    for (const auto& name : pluginNames) {
        auto tmpInfo = Plugin::PluginManager::Instance().GetPluginInfo(pluginType, name);
        if (CompatibleWith(tmpInfo->inCaps, *inMeta) && tmpInfo->rank > maxRank) {
            info = tmpInfo;
        }
    }
    if (info == nullptr) {
        return ErrorCode::ERROR_UNSUPPORTED_FORMAT;
    }

    // try to reuse the plugin if their name are the same
    if (plugin != nullptr && pluginInfo != nullptr) {
        if (info->name == pluginInfo->name) {
            if (TranslatePluginStatus(plugin->Reset()) == ErrorCode::SUCCESS) {
                return ErrorCode::SUCCESS;
            }
        }
        plugin->Deinit();
    }
    plugin = pluginCreator(info->name);
    if (plugin == nullptr) {
        return ErrorCode::ERROR_UNSUPPORTED_FORMAT;
    }
    pluginInfo = info;
    return ErrorCode::SUCCESS;
}

template ErrorCode FindPluginAndUpdate(const std::shared_ptr<const Plugin::Meta>&, Plugin::PluginType,
                                       std::shared_ptr<Plugin::Codec>&, std::shared_ptr<Plugin::PluginInfo>&,
                                       std::function<std::shared_ptr<Plugin::Codec>(const std::string&)>);
template ErrorCode FindPluginAndUpdate(const std::shared_ptr<const Plugin::Meta>&, Plugin::PluginType,
                                       std::shared_ptr<Plugin::AudioSink>&, std::shared_ptr<Plugin::PluginInfo>&,
                                       std::function<std::shared_ptr<Plugin::AudioSink>(const std::string&)>);
template ErrorCode FindPluginAndUpdate(const std::shared_ptr<const Plugin::Meta>&, Plugin::PluginType,
                                       std::shared_ptr<Plugin::VideoSink>&, std::shared_ptr<Plugin::PluginInfo>&,
                                       std::function<std::shared_ptr<Plugin::VideoSink>(const std::string&)>);
} // namespace Pipeline
} // namespace Media
} // namespace OHOS
