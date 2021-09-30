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

#ifdef VIDEO_SUPPORT

#ifndef HISTREAMER_VIDEO_FFMPEG_DECODER_PLUGIN_H
#define HISTREAMER_VIDEO_FFMPEG_DECODER_PLUGIN_H

#include <functional>
#include <map>
#include "osal/thread/task.h"
#include "foundation/blocking_queue.h"
#include "plugin/interface/codec_plugin.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "libavcodec/avcodec.h"
#ifdef __cplusplus
};
#endif

#ifdef DUMP_RAW_DATA
#include <fstream>
#endif

namespace OHOS {
namespace Media {
namespace Plugin {
class VideoFfmpegDecoderPlugin : public CodecPlugin {
public:
    explicit VideoFfmpegDecoderPlugin(std::string name);
    ~VideoFfmpegDecoderPlugin() override = default;

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

    Status GetParameter(Tag tag, ValueType &value) override;
    Status SetParameter(Tag tag, const ValueType &value) override;

    std::shared_ptr<Allocator> GetAllocator() override;

    Status SetCallback(const std::shared_ptr<Callback> &cb) override
    {
        return Status::OK;
    }

    Status QueueInputBuffer(const std::shared_ptr<Buffer> &inputBuffer, int32_t timeoutMs) override;

    Status QueueOutputBuffer(const std::shared_ptr<Buffer> &outputBuffers, int32_t timeoutMs) override;

    Status Flush() override;

    Status SetDataCallback(const std::weak_ptr<DataCallback> &dataCallback) override
    {
        dataCb_ = dataCallback;
        return Status::OK;
    }

private:
    Status CreateCodecContext();
    void InitCodecContext();
    void DeinitCodecContext();
    void SetCodecExtraData();
    Status OpenCodecContext();
    Status CloseCodecContext();

    void ConfigCodecId();

    Status ResetLocked();

    template<typename T>
    void FindInParameterMapThenAssignLocked(Tag tag, T &assign);

    Status SendBufferLocked(const std::shared_ptr<Buffer> &inputBuffer);

    void CalculateFrameSizes(size_t &ySize, size_t &uvSize, size_t &frameSize);

    Status FillFrameBuffer(const std::shared_ptr<Buffer> &frameBuffer);

    Status ReceiveBufferLocked(const std::shared_ptr<Buffer> &frameBuffer);

    void CheckResolutionChange();

    void ReceiveBuffer();

#ifdef DUMP_RAW_DATA
    std::ofstream dumpData_;
    void DumpVideoRawOutData();
#endif

    void NotifyInputBufferDone(const std::shared_ptr<Buffer> &input);
    void NotifyOutputBufferDone(const std::shared_ptr<Buffer> &output);

    std::string name_;
    std::shared_ptr<const AVCodec> avCodec_;
    std::map<Tag, ValueType> videoDecParams_ {};
    std::vector<uint8_t> paddedBuffer_;
    size_t paddedBufferSize_ {0};
    std::shared_ptr<AVFrame> cachedFrame_;
    std::weak_ptr<DataCallback> dataCb_ {};

    uint32_t width_;
    uint32_t height_;
    VideoPixelFormat pixelFormat_;

    mutable OSAL::Mutex lock_ {};
    State state_ {State::CREATED};
    std::shared_ptr<AVCodecContext> avCodecContext_ {};
    OHOS::Media::BlockingQueue<std::shared_ptr<Buffer>> outBufferQ_;
    std::shared_ptr<OHOS::Media::OSAL::Task> decodeTask_;
};
}
}
}
#endif // HISTREAMER_VIDEO_FFMPEG_DECODER_PLUGIN_H
#endif
