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

#ifndef HISTREAMER_AU_SERVER_SINK_PLUGIN_H
#define HISTREAMER_AU_SERVER_SINK_PLUGIN_H

#include <atomic>
#include <memory>

#include "audio_renderer.h"
#include "audio_info.h"
#include "plugin/common/plugin_audio_tags.h"
#include "plugin/interface/audio_sink_plugin.h"
#include "timestamp.h"

namespace OHOS {
namespace Media {
namespace AuSrSinkPlugin {
class AudioServerSinkPlugin : public Plugin::AudioSinkPlugin {
public:
    explicit AudioServerSinkPlugin(std::string name);

    ~AudioServerSinkPlugin() override;

    Plugin::Status Init() override;

    Plugin::Status Deinit() override;

    Plugin::Status Prepare() override;

    Plugin::Status Reset() override;

    Plugin::Status Start() override;

    Plugin::Status Stop() override;

    bool IsParameterSupported(Plugin::Tag tag) override
    {
        return true;
    }

    Plugin::Status GetParameter(Plugin::Tag tag, Plugin::ValueType& value) override;

    Plugin::Status SetParameter(Plugin::Tag tag, const Plugin::ValueType& value) override;

    std::shared_ptr<Plugin::Allocator> GetAllocator() override
    {
        return nullptr;
    }

    Plugin::Status SetCallback(Plugin::Callback* cb) override
    {
        return Plugin::Status::OK;
    }

    Plugin::Status GetMute(bool& mute) override
    {
        return Plugin::Status::OK;
    }

    Plugin::Status SetMute(bool mute) override
    {
        return Plugin::Status::OK;
    }

    Plugin::Status GetVolume(float& volume) override;

    Plugin::Status SetVolume(float volume) override;

    Plugin::Status GetSpeed(float& speed) override
    {
        return Plugin::Status::OK;
    }

    Plugin::Status SetSpeed(float speed) override
    {
        return Plugin::Status::OK;
    }

    Plugin::Status Pause() override;

    Plugin::Status Resume() override
    {
        return Plugin::Status::OK;
    }

    Plugin::Status GetLatency(uint64_t& ms) override;

    Plugin::Status GetFrameSize(size_t& size) override
    {
        return Plugin::Status::OK;
    }

    Plugin::Status GetFrameCount(uint32_t& count) override
    {
        return Plugin::Status::OK;
    }

    Plugin::Status Write(const std::shared_ptr<Plugin::Buffer>& input) override;

    Plugin::Status Flush() override;

private:
    bool AssignSampleRateIfSupported(uint32_t sampleRate);
    bool AssignChannelNumIfSupported(uint32_t channelNum);
    bool AssignSampleFmtIfSupported(Plugin::AudioSampleFormat sampleFormat);

    std::unique_ptr<AudioStandard::AudioRenderer> audioRenderer_;
    AudioStandard::AudioRendererParams rendererParams_ {};
    int64_t bitRate_ {0};
};
} // namespace AuSrSinkPlugin
} // namespace Media
} // namespace OHOS
#endif
