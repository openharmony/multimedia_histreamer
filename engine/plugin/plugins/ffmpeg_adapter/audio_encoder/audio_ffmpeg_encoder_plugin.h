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
#ifndef HISTREAMER_AUDIO_FFMPEG_ENCODER_PLUGIN_H
#define HISTREAMER_AUDIO_FFMPEG_ENCODER_PLUGIN_H

#ifdef RECORDER_SUPPORT

#include <functional>
#include <map>
#include "utils/blocking_queue.h"
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
class AudioFfmpegEncoderPlugin : public CodecPlugin {
public:
    explicit AudioFfmpegEncoderPlugin(std::string name);

    ~AudioFfmpegEncoderPlugin() override;

    Status Init() override;

    Status Deinit() override;

    Status Prepare() override;

    Status Reset() override;

    Status Start() override;

    Status Stop() override;

    bool IsParameterSupported(Tag tag) override
    {
        if (tag == Tag::REQUIRED_OUT_BUFFER_CNT) {
            return true;
        }
        return false;
    }

    Status GetParameter(Tag tag, ValueType& value) override;

    Status SetParameter(Tag tag, const ValueType& value) override;

    std::shared_ptr<Allocator> GetAllocator() override;

    Status SetCallback(Callback* cb) override
    {
        return Status::OK;
    }

    Status QueueInputBuffer(const std::shared_ptr<Buffer>& inputBuffer, int32_t timeoutMs) override;

    Status DequeueInputBuffer(std::shared_ptr<Buffer>& inputBuffer, int32_t timeoutMs) override;

    Status QueueOutputBuffer(const std::shared_ptr<Buffer>& outputBuffer, int32_t timeoutMs) override;

    Status DequeueOutputBuffer(std::shared_ptr<Buffer>& outputBuffer, int32_t timeoutMs) override;

    Status Flush() override;

    Status SetDataCallback(const std::weak_ptr<DataCallback>& dataCallback) override
    {
        return Status::OK;
    }

private:
    Status ResetLocked();

    Status DeInitLocked();

    template <typename T>
    bool FindInParameterMapThenAssignLocked(Tag tag, T& assign);

    void InitInputFrame();

    Status SendBufferLocked(const std::shared_ptr<Buffer>& inputBuffer);

    Status ReceiveFrameSucc(const std::shared_ptr<Buffer>& ioInfo, std::shared_ptr<AVPacket> packet);

    Status ReceiveBuffer();

    Status ReceiveBufferLocked(const std::shared_ptr<Buffer>& ioInfo);

    void InitCacheFrame();

    mutable OSAL::Mutex parameterMutex_ {};
    std::map<Tag, ValueType> audioParameter_ {};

    mutable OSAL::Mutex avMutex_ {};
    std::shared_ptr<const AVCodec> avCodec_ {nullptr};
    std::shared_ptr<AVCodecContext> avCodecContext_ {nullptr};
    std::shared_ptr<Buffer> cachedBuffer_ {nullptr};
    AVFrame* cachedFrame_ {nullptr};
    std::shared_ptr<Buffer> outBuffer_ {nullptr};
    uint64_t prev_pts_;
};
} // namespace Plugin
} // namespace Media
} // namespace OHOS

#endif
#endif // HISTREAMER_AUDIO_FFMPEG_ENCODER_PLUGIN_H
