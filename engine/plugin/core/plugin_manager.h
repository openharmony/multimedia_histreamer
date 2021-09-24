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

#include "plugin_info.h"
#include "plugin_register.h"

#include "audio_sink.h"
#include "codec.h"
#include "demuxer.h"
#include "source.h"
#include "video_sink.h"

namespace OHOS {
namespace Media {
namespace Plugin {
class PluginManager {
public:
    PluginManager(const PluginManager&) = delete;
    PluginManager operator=(const PluginManager&) = delete;
    ~PluginManager();
    static PluginManager& Instance()
    {
        static PluginManager impl;
        return impl;
    }

    std::set<std::string> ListPlugins(PluginType type);

    std::shared_ptr<PluginInfo> GetPluginInfo(PluginType type, const std::string& name);

    std::shared_ptr<Source> CreateSourcePlugin(const std::string& name);

    std::shared_ptr<Demuxer> CreateDemuxerPlugin(const std::string& name);

    std::shared_ptr<Codec> CreateCodecPlugin(const std::string& name);

    std::shared_ptr<AudioSink> CreateAudioSinkPlugin(const std::string& name);

    std::shared_ptr<VideoSink> CreateVideoSinkPlugin(const std::string& name);

    int32_t Sniffer(const std::string& name, std::shared_ptr<DataSourceHelper> source);

private:
    PluginManager();

    void Init();

private:
    std::shared_ptr<PluginRegister> pluginRegister;
};
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PLUGIN_MANAGER_H
