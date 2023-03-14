/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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

#ifndef HISTREAMER_PIPELINE_FILTER_CODEC_MODE_H
#define HISTREAMER_PIPELINE_FILTER_CODEC_MODE_H

#include <iostream>
#include "pipeline/core/error_code.h"
#include "pipeline/core/filter.h"
#include "pipeline/core/type_define.h"
#include "plugin/core/codec.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
enum struct FilterCodecMode : uint32_t {
    AUDIO_SYNC_DECODER = 1,
    AUDIO_ASYNC_DECODER,
    AUDIO_SYNC_ENCODER,
    AUDIO_ASYNC_ENCODER,
    VIDEO_SYNC_DECODER,
    VIDEO_ASYNC_DECODER,
    VIDEO_SYNC_ENCODER,
    VIDEO_ASYNC_ENCODER,
};
class CodecMode {
public:
    explicit CodecMode(std::string name) : codecName_(std::move(name)) {}
    virtual ~CodecMode() = default;

    bool Init(std::shared_ptr<Plugin::Codec>& plugin, std::vector<POutPort>& outPorts);

    virtual ErrorCode Configure();

    virtual ErrorCode PushData(const std::string &inPort, const AVBufferPtr& buffer, int64_t offset) = 0;

    virtual ErrorCode Stop() = 0;

    virtual void FlushStart() = 0;

    virtual void FlushEnd() = 0;

    virtual void OnOutputBufferDone(const std::shared_ptr<Plugin::Buffer>& output) = 0;

    virtual ErrorCode Prepare();

    virtual ErrorCode Release();

    void SetBufferPoolSize(uint32_t inBufPoolSize, uint32_t outBufPoolSize);

    void CreateOutBufferPool(std::shared_ptr<Allocator>& outAllocator,
                             uint32_t bufferCnt, uint32_t bufferSize, Plugin::BufferMetaType bufferMetaType);

protected:
    uint32_t GetInBufferPoolSize() const;
    uint32_t GetOutBufferPoolSize() const;

    std::shared_ptr<Plugin::Codec> plugin_ {nullptr};
    std::vector<POutPort> outPorts_ {};
    std::shared_ptr<BufferPool<AVBuffer>> outBufPool_ {nullptr};
    std::string codecName_ {};

private:
    uint32_t inBufPoolSize_ {0};
    uint32_t outBufPoolSize_ {0};
};
} // namespace Pipeline
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PIPELINE_FILTER_CODEC_MODE_H
