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

#ifndef HISTREAMER_HDI_SINK_H
#define HISTREAMER_HDI_SINK_H

#include <atomic>

#include "audio_types.h"
#include "foundation/osal/thread/mutex.h"
#include "foundation/osal/thread/condition_variable.h"
#include "foundation/osal/thread/task.h"
#include "interface/audio_sink_plugin.h"
#include "audio_manager.h"

struct AudioAdapter;
struct AudioRender;
struct AudioPort;
namespace OHOS {
namespace Media {
namespace HosLitePlugin {
class RingBuffer;

class HdiSink : public std::enable_shared_from_this<HdiSink>, public OHOS::Media::Plugin::AudioSinkPlugin {
public:
    explicit HdiSink(std::string name);

    ~HdiSink() override = default;

    Media::Plugin::Status Init() override;

    Media::Plugin::Status Deinit() override;

    Media::Plugin::Status Prepare() override;

    Media::Plugin::Status Reset() override;

    Media::Plugin::Status Start() override;

    Media::Plugin::Status Stop() override;

    bool IsParameterSupported(Media::Plugin::Tag tag) override;

    Media::Plugin::Status GetParameter(Media::Plugin::Tag tag, Media::Plugin::ValueType &value) override;

    Media::Plugin::Status
    SetParameter(Media::Plugin::Tag tag, const Media::Plugin::ValueType &value) override;

    std::shared_ptr<OHOS::Media::Plugin::Allocator> GetAllocator() override;

    Media::Plugin::Status SetCallback(const std::shared_ptr<OHOS::Media::Plugin::Callback> &cb) override;

    Media::Plugin::Status GetMute(bool &mute) override;

    Media::Plugin::Status SetMute(bool mute) override;

    Media::Plugin::Status GetVolume(float &volume) override;

    Media::Plugin::Status SetVolume(float volume) override;

    Media::Plugin::Status GetSpeed(float &speed) override;

    Media::Plugin::Status SetSpeed(float speed) override;

    Media::Plugin::Status Pause() override;

    Media::Plugin::Status Resume() override;

    Media::Plugin::Status GetLatency(uint64_t &ms) override;

    Media::Plugin::Status GetFrameSize(size_t &size) override;

    Media::Plugin::Status GetFrameCount(uint32_t &count) override;

    Media::Plugin::Status Write(const std::shared_ptr<Media::Plugin::Buffer> &input) override;

    Media::Plugin::Status Flush() override;

private:
    Media::Plugin::Status ReleaseRender();

    void DoRender();

private:
    const std::string adapterName_;

    std::atomic<OHOS::Media::Plugin::State> pluginState_ {OHOS::Media::Plugin::State::CREATED};

    OHOS::Media::OSAL::Mutex renderMutex_ {};

    AudioManager* audioManager_ {nullptr};
    AudioAdapterDescriptor adapterDescriptor_ {};
    AudioAdapter* audioAdapter_ {nullptr};
    AudioRender* audioRender_ {nullptr};
    AudioPort audioPort_ {};
    AudioDeviceDescriptor deviceDescriptor_ {};
    AudioSampleAttributes sampleAttributes_ {};
    AudioChannelMask channelMask_ {AUDIO_CHANNEL_MONO};

    std::weak_ptr<OHOS::Media::Plugin::Callback> eventCallback_ {};

    std::shared_ptr<RingBuffer> ringBuffer_ {};
    std::shared_ptr<OHOS::Media::OSAL::Task> renderThread_ {};
};
}
}
}
#endif
