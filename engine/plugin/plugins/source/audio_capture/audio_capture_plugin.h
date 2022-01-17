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

#ifndef HISTREAMER_AUDIO_CAPTURE_PLUGIN_H
#define HISTREAMER_AUDIO_CAPTURE_PLUGIN_H

#include "plugin/common/plugin_types.h"
#include "plugin/interface/source_plugin.h"
#include "audio_capturer.h"

namespace OHOS {
namespace Media {
namespace AuCapturePlugin {
class AudioCaptureAllocator : public Plugin::Allocator {
public:
    AudioCaptureAllocator() = default;
    ~AudioCaptureAllocator() override = default;

    void* Alloc(size_t size) override;
    void Free(void* ptr) override; // NOLINT: void*
};

class AudioCapturePlugin : public Plugin::SourcePlugin {
public:
    explicit AudioCapturePlugin(std::string name);
    ~AudioCapturePlugin() override;

    Plugin::Status Init() override;
    Plugin::Status Deinit() override;
    Plugin::Status Prepare() override;
    Plugin::Status Reset() override;
    Plugin::Status Start() override;
    Plugin::Status Stop() override;
    bool IsParameterSupported(Plugin::Tag tag) override;
    Plugin::Status GetParameter(Plugin::Tag tag, Plugin::ValueType& value) override;
    Plugin::Status SetParameter(Plugin::Tag tag, const Plugin::ValueType& value) override;
    std::shared_ptr<Plugin::Allocator> GetAllocator() override;
    Plugin::Status SetCallback(Plugin::Callback* cb) override;
    Plugin::Status SetSource(std::shared_ptr<Plugin::MediaSource> source) override;
    Plugin::Status Read(std::shared_ptr<Plugin::Buffer>& buffer, size_t expectedLen) override;
    Plugin::Status GetSize(size_t& size) override;
    bool IsSeekable() override;
    Plugin::Status SeekTo(uint64_t offset) override;

private:
    bool AssignSampleRateIfSupported(uint32_t sampleRate);
    bool AssignChannelNumIfSupported(uint32_t channelNum);
    bool AssignSampleFmtIfSupported(OHOS::Media::Plugin::AudioSampleFormat sampleFormat);
    Plugin::Status GetAudioTime(uint64_t &audioTimeNs);

    std::shared_ptr<AudioCaptureAllocator> mAllocator_ {nullptr};
    std::shared_ptr<OHOS::AudioStandard::AudioCapturer> audioCapturer_ {nullptr};
    AudioStandard::AudioCapturerParams capturerParams_ {};
    bool isStop_ {false};
    int64_t bitRate_ {0};
    size_t bufferSize_ {0};
    uint64_t curTimestampNs_ {0};
    uint64_t stopTimestampNs_ {0};
    uint64_t totalPauseTimeNs_ {0};
};
} // namespace AuCapturePlugin
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_AUDIO_CAPTURE_PLUGIN_H

