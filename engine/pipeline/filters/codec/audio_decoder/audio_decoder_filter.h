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

#ifndef HISTREAMER_PIPELINE_FILTER_AUDIO_DECODER_H
#define HISTREAMER_PIPELINE_FILTER_AUDIO_DECODER_H

#include "filters/codec/decoder_filter_base.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
class AudioDecoderFilter : public DecoderFilterBase {
public:
    explicit AudioDecoderFilter(const std::string &name);
    ~AudioDecoderFilter() override;

    ErrorCode Start() override;

    ErrorCode Stop() override;

    ErrorCode Prepare() override;

    bool Negotiate(const std::string &inPort, const std::shared_ptr<const Plugin::Meta> &inMeta,
                   CapabilitySet &outCaps) override;

    ErrorCode PushData(const std::string &inPort, AVBufferPtr buffer) override;

    void FlushStart() override;

    void FlushEnd() override;

private:
    class DataCallbackImpl;

    ErrorCode ConfigureToStartPluginLocked(const std::shared_ptr<const Plugin::Meta> &meta);

    ErrorCode ConfigureWithMetaLocked(const std::shared_ptr<const Plugin::Meta> &meta);

    void HandleFrame();

    void HandleOneFrame(const std::shared_ptr<AVBuffer> &data);

    void FinishFrame();

    ErrorCode Release();

    // callbacks
    void OnInputBufferDone(const std::shared_ptr<AVBuffer> &buffer);

    void OnOutputBufferDone(const std::shared_ptr<AVBuffer> &buffer);

private:
    ErrorCode QueueAllBufferInPoolToPluginLocked();

    std::shared_ptr<OHOS::Media::BlockingQueue<OHOS::Media::AVBufferPtr>> inBufferQ_;
    std::shared_ptr<OHOS::Media::BlockingQueue<OHOS::Media::AVBufferPtr>> outBufferQ_; // PCM data
    std::shared_ptr<OHOS::Media::OSAL::Task> handleFrameTask_ {}; // dequeue from es bufferQ then enqueue to plugin
    // this task will dequeue from the plugin and then push to downstream
    std::shared_ptr<OHOS::Media::OSAL::Task> pushTask_ {nullptr};
    std::shared_ptr<BufferPool<AVBuffer>> outBufferPool_ {};
    bool isFlushing_ {false};

    std::shared_ptr<DataCallbackImpl> dataCallback_ {nullptr};
};
}
}
}
#endif // HISTREAMER_PIPELINE_FILTER_AUDIO_DECODER_H
