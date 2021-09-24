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

#ifndef HISTREAMER_AUDIO_FFMPEG_DECODER_PLUGIN_H
#define HISTREAMER_AUDIO_FFMPEG_DECODER_PLUGIN_H

#include <functional>
#include <map>
#include "foundation/blocking_queue.h"
#include "plugin/interface/codec_plugin.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "libavcodec/avcodec.h"
#ifdef __cplusplus
};
#endif

namespace OHOS {
namespace Media {
namespace Plugin {
class AudioFfmpegDecoderPlugin : public CodecPlugin {
public:
    explicit AudioFfmpegDecoderPlugin(std::string name);

    ~AudioFfmpegDecoderPlugin() override = default;

    Status Init() override;

    Status Deinit() override;

    Status Prepare() override;

    Status Reset() override;

    Status Start() override;

    Status Stop() override;

    bool IsParameterSupported(Tag tag) override
    {
        return true;
    }

    Status GetParameter(Tag tag, ValueType& value) override;

    Status SetParameter(Tag tag, const ValueType& value) override;

    Status GetState(State& state) override
    {
        return Status::ERROR_UNIMPLEMENTED;
    }

    std::shared_ptr<Allocator> GetAllocator() override;

    Status SetCallback(const std::shared_ptr<Callback>& cb) override
    {
        return Status::OK;
    }

    Status QueueInputBuffer(const std::shared_ptr<Buffer>& inputBuffer, int32_t timeoutMs) override;

    Status QueueOutputBuffer(const std::shared_ptr<Buffer>& outputBuffers, int32_t timeoutMs) override;

    Status Flush() override;

    Status SetDataCallback(const std::weak_ptr<DataCallback>& dataCallback) override
    {
        dataCb_ = dataCallback;
        return Status::OK;
    }

private:
    void InitCodecContextExtraData();

    Status ResetLocked();

    template <typename T>
    bool FindInParameterMapThenAssignLocked(Tag tag, T& assign);

    Status SendBuffer(const std::shared_ptr<Buffer>& inputBuffer);

    Status SendBufferLocked(const std::shared_ptr<Buffer>& inputBuffer);

    void ReceiveFrameSucc(const std::shared_ptr<Buffer>& ioInfo, Status& status,
                          bool& receiveOneFrame, bool& notifyBufferDone);

    bool ReceiveBuffer(Status& err);
    void ReceiveBufferLocked(Status& status, const std::shared_ptr<Buffer>& ioInfo, bool& receiveOneFrame,
                             bool& notifyBufferDone);

    void NotifyInputBufferDone(const std::shared_ptr<Buffer>& input);

    void NotifyOutputBufferDone(const std::shared_ptr<Buffer>& output);

    std::string name_;
    std::shared_ptr<const AVCodec> avCodec_ {};
    std::map<Tag, ValueType> audioParameter_ {};
    std::vector<uint8_t> paddedBuffer_ {};
    size_t paddedBufferSize_ {0};
    std::shared_ptr<AVFrame> cachedFrame_ {};
    std::weak_ptr<DataCallback> dataCb_ {};

    mutable OSAL::Mutex lock_ {};
    State state_ {State::CREATED};
    std::shared_ptr<AVCodecContext> avCodecContext_ {};

    // outBufferQ有自己的锁保护 不要和lock_同时混用 否则可能导致死锁
    OHOS::Media::BlockingQueue<std::shared_ptr<Buffer>> outBufferQ_;
};
} // namespace Plugin
} // namespace Media
} // namespace OHOS

#endif // HISTREAMER_AUDIO_FFMPEG_DECODER_PLUGIN_H
