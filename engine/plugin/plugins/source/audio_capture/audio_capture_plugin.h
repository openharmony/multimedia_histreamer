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

#ifndef HISTREAMER_PIPELINE_AUDIO_CAPTURE_PLUGIN_H
#define HISTREAMER_PIPELINE_AUDIO_CAPTURE_PLUGIN_H
#ifdef RECORDER_SUPPORT
#include "plugin/common/plugin_types.h"
#include "plugin/interface/source_plugin.h"
#include "audio_capturer.h"

namespace OHOS {
namespace Media {
namespace Plugin {
class AudioCaptureAllocator : public Allocator {
public:
    AudioCaptureAllocator() = default;
    ~AudioCaptureAllocator() override = default;

    void* Alloc(size_t size) override;
    void Free(void* ptr) override; // NOLINT: void*
};

class AudioCapturePlugin : public SourcePlugin {
public:
    explicit AudioCapturePlugin(std::string name);
    ~AudioCapturePlugin() override;

    Status Init() override;
    Status Deinit() override;
    Status Prepare() override;
    Status Reset() override;
    Status Start() override;
    Status Stop() override;
    bool IsParameterSupported(Tag tag) override;
    Status GetParameter(Tag tag, ValueType& value) override;
    Status SetParameter(Tag tag, const ValueType& value) override;
    std::shared_ptr<Allocator> GetAllocator() override;
    Status SetCallback(const std::shared_ptr<Callback>& cb) override;
    Status SetSource(std::string& uri, std::shared_ptr<std::map<std::string, ValueType>> params = nullptr) override;
    Status Read(std::shared_ptr<Buffer>& buffer, size_t expectedLen) override;
    Status GetSize(size_t& size) override;
    bool IsSeekable() override;
    Status SeekTo(uint64_t offset) override;

private:
    bool IsSampleRateSupported(const uint32_t sampleRate);
    bool IsChannelNumSupported(const uint32_t chanNum);
    bool IsSampleFormatSupported(const OHOS::AudioStandard::AudioSampleFormat sampleFormat);
    Status GetAudioTime(uint64_t &audioTime);

    std::shared_ptr<AudioCaptureAllocator> mAllocator_ {nullptr};
    std::shared_ptr<OHOS::AudioStandard::AudioCapturer> audioCapturer_ {nullptr};
    AudioStandard::AudioCapturerParams capturerParams_;
    bool isStop_;
    int64_t bitRate_;
    size_t bufferSize_;
    uint64_t curTimestamp_;
    uint64_t stopTimestamp_;
    uint64_t totalPauseTime_;
};
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif
#endif // HISTREAMER_PIPELINE_AUDIO_CAPTURE_PLUGIN_H

