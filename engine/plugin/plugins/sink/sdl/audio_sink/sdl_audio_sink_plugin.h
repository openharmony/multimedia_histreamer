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

#ifndef HISTREAMER_SDL_AU_SINK_PLUGIN_H
#define HISTREAMER_SDL_AU_SINK_PLUGIN_H

#include <atomic>
#include "SDL.h"
#include "plugin/interface/audio_sink_plugin.h"
#include "plugin/common/plugin_audio_tags.h"
#include "plugin/plugins/sink/sdl/ring_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
#ifdef __cplusplus
};
#endif

namespace OHOS {
namespace Media {
namespace Plugin {
class SdlAudioSinkPlugin : public std::enable_shared_from_this<SdlAudioSinkPlugin>, public AudioSinkPlugin {
public:
    explicit SdlAudioSinkPlugin(std::string name);
    ~SdlAudioSinkPlugin() override = default;

    Status Init() override;

    Status Deinit() override;

    Status Prepare() override;

    Status Reset() override;

    Status Start() override;

    Status Stop() override;

    bool IsParameterSupported(Tag tag) override;

    Status GetParameter(Tag tag, ValueType &value) override;

    Status SetParameter(Tag tag, const ValueType &value) override;

    std::shared_ptr<Allocator> GetAllocator() override;

    Status SetCallback(const std::shared_ptr<Callback> &cb) override;

    // audio sink

    Status GetMute(bool &mute) override;

    Status SetMute(bool mute) override;

    Status GetVolume(float &volume) override;

    Status SetVolume(float volume) override;

    Status GetSpeed(float &speed) override;

    Status SetSpeed(float speed) override;

    Status Pause() override;

    Status Resume() override;

    Status GetLatency(uint64_t &ms) override;

    Status GetFrameSize(size_t &size) override;

    Status GetFrameCount(uint32_t &count) override;

    Status Write(const std::shared_ptr<Buffer> &input) override;

    Status Flush() override;

private:
    void AudioCallback(void* userdata, uint8_t* stream, int len);

    std::vector<uint8_t> transformCache_ {};
    std::vector<uint8_t> mixCache_ {};
    std::unique_ptr<RingBuffer> rb {};
    size_t avFrameSize_ {};
    SDL_AudioSpec wantedSpec_ {};
    SDL_AudioDeviceID deviceId_ {};
    int32_t channels_ {-1};
    int32_t sampleRate_ {-1};
    int32_t samplesPerFrame_ {-1};
    int64_t channelMask_ {0};
    AudioSampleFormat audioFormat_ {AudioSampleFormat::U8};
    std::shared_ptr<SwrContext> swrCtx_ {nullptr};
};
}
}
}
#endif // MEDIA_PIPELINE_SDL_AU_SINK_PLUGIN_H
