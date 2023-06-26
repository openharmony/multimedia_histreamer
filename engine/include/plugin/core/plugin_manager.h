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

#ifndef HISTREAMER_PLUGIN_MANAGER_H
#define HISTREAMER_PLUGIN_MANAGER_H

#include "plugin/common/type_cast_ext.h"
#include "plugin/core/audio_sink.h"
#include "plugin/core/codec.h"
#include "plugin/core/demuxer.h"
#include "plugin/core/muxer.h"
#include "plugin/core/output_sink.h"
#include "plugin/core/plugin_info.h"
#include "plugin/core/plugin_register.h"
#include "plugin/core/source.h"
#include "plugin/core/video_sink.h"
#include "plugin/interface/generic_plugin.h"

namespace OHOS {
namespace Media {
namespace Plugin {
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

    std::vector<std::string> ListPlugins(PluginType pluginType, CodecMode preferredCodecMode = CodecMode::HARDWARE);

    std::shared_ptr<PluginInfo> GetPluginInfo(PluginType type, const std::string& name);

    std::shared_ptr<Source> CreateSourcePlugin(const std::string& name);

    std::shared_ptr<Demuxer> CreateDemuxerPlugin(const std::string& name);

    std::shared_ptr<Muxer> CreateMuxerPlugin(const std::string& name);

    std::shared_ptr<Codec> CreateCodecPlugin(const std::string& name, PluginType type);

    std::shared_ptr<AudioSink> CreateAudioSinkPlugin(const std::string& name);

    std::shared_ptr<VideoSink> CreateVideoSinkPlugin(const std::string& name);

    std::shared_ptr<OutputSink> CreateOutputSinkPlugin(const std::string& name);

    template<typename T, typename U>
    std::shared_ptr<T> CreateGenericPlugin(const std::string& name)
    {
        return CreatePlugin<T, U>(name, PluginType::GENERIC_PLUGIN);
    }

    int32_t Sniffer(const std::string& name, std::shared_ptr<DataSourceHelper> source);

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
        auto plugin = ReinterpretPointerCast<U>(regInfo->creator(name));
        if (!plugin) {
            return {};
        }
        return std::shared_ptr<T>(
                new T(regInfo->packageDef->pkgVersion, regInfo->info->apiVersion, plugin));
    }

private:
    std::shared_ptr<PluginRegister> pluginRegister_;
};
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PLUGIN_MANAGER_H
