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

#ifndef MEDIA_PIPELINE_VIDEO_DECODER_FILTER_H
#define MEDIA_PIPELINE_VIDEO_DECODER_FILTER_H

#ifdef VIDEO_SUPPORT

#include "utils/type_define.h"
#include "filters/codec/decoder_filter_base.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
class VideoDecoderFilter : public DecoderFilterBase {
public:
    explicit VideoDecoderFilter(const std::string &name);
    ~VideoDecoderFilter() override;

    ErrorCode Prepare() override;

    ErrorCode Start() override;

    ErrorCode Stop() override;

    void FlushStart() override;

    void FlushEnd() override;

    bool Negotiate(const std::string &inPort, const std::shared_ptr<const Plugin::Meta> &inMeta,
                   CapabilitySet &outCaps) override;

    ErrorCode PushData(const std::string &inPort, AVBufferPtr buffer) override;

    void OnInputBufferDone(const std::shared_ptr<AVBuffer> &buffer);

    void OnOutputBufferDone(const std::shared_ptr<AVBuffer> &buffer);

private:
    class DataCallbackImpl;

    struct VideoDecoderFormat {
        std::string mime;
        uint32_t width;
        uint32_t height;
        int64_t bitRate;
        uint32_t format;
        std::vector<uint8_t> codecConfig;
    };

    ErrorCode SetVideoDecoderFormat(const std::shared_ptr<const Plugin::Meta>& meta);

    ErrorCode AllocateOutputBuffers();

    ErrorCode InitPlugin();

    ErrorCode ConfigurePluginOutputBuffers();

    ErrorCode ConfigurePluginParams();

    ErrorCode ConfigurePlugin();

    ErrorCode Configure(const std::shared_ptr<const Plugin::Meta> &meta);

    void HandleFrame();

    void HandleOneFrame(const std::shared_ptr<AVBuffer> &data);

    void FinishFrame();

    bool isFlushing_ {false};
    VideoDecoderFormat vdecFormat_;
    std::shared_ptr<DataCallbackImpl> dataCallback_ {nullptr};

    std::shared_ptr<OHOS::Media::OSAL::Task> handleFrameTask_ {nullptr};
    std::shared_ptr<OHOS::Media::OSAL::Task> pushTask_ {nullptr};
    std::shared_ptr<BufferPool<AVBuffer>> outBufPool_  {nullptr};
    std::shared_ptr<OHOS::Media::BlockingQueue<AVBufferPtr>> inBufQue_  {nullptr};
    std::shared_ptr<OHOS::Media::BlockingQueue<AVBufferPtr>> outBufQue_  {nullptr};
};
}
}
}
#endif
#endif // MEDIA_PIPELINE_VIDEO_DECODER_FILTER_H