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

#ifdef RECORDER_SUPPORT

#ifndef HISTREAMER_PIPELINE_FILTER_AUDIO_ENCODER_H
#define HISTREAMER_PIPELINE_FILTER_AUDIO_ENCODER_H

#include "filters/codec/decoder_filter_base.h"
#include "plugin/common/plugin_tags.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
class AudioEncoderFilter : public DecoderFilterBase {
public:
    explicit AudioEncoderFilter(const std::string &name);
    ~AudioEncoderFilter() override;

    virtual ErrorCode SetAudioEncoder(int32_t sourceId, OHOS::Media::Plugin::AudioFormat encoder);

    ErrorCode Start() override;

    ErrorCode Stop() override;

    ErrorCode Prepare() override;

    bool Negotiate(const std::string& inPort, const std::shared_ptr<const Plugin::Capability>& upstreamCap,
                   Capability& upstreamNegotiatedCap) override;

    bool Configure(const std::string& inPort, const std::shared_ptr<const Plugin::Meta>& upstreamMeta) override;

    /**
     *
     * @param inPort
     * @param buffer
     * @param offset always ignore this parameter
     * @return
     */
    ErrorCode PushData(const std::string &inPort, AVBufferPtr buffer, int64_t offset) override;

    void FlushStart() override;

    void FlushEnd() override;

private:
    ErrorCode ConfigureToStartPluginLocked(const std::shared_ptr<const Plugin::Meta> &meta);

    ErrorCode HandleFrame(const std::shared_ptr<AVBuffer>& buffer);

    ErrorCode FinishFrame();

    ErrorCode Release();

private:
    std::shared_ptr<BufferPool<AVBuffer>> outBufferPool_ {};
    bool isFlushing_ {false};

    Capability capNegWithDownstream_;
    Capability capNegWithUpstream_;
};
}
}
}
#endif // HISTREAMER_PIPELINE_FILTER_AUDIO_ENCODER_H
#endif