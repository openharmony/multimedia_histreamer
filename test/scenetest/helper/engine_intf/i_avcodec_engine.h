/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef IAVCODEC_ENGINE_H
#define IAVCODEC_ENGINE_H

#include <cstdint>
#include <string>
#include <memory>
#include <refbase.h>
#include "nocopyable.h"
#include "avcodec_common.h"
#include "avcodec_info.h"
#include "avsharedmemory.h"
#include "format.h"

namespace OHOS {
class Surface;

namespace Media {
class IAVCodecEngineObs : public std::enable_shared_from_this<IAVCodecEngineObs> {
public:
    virtual ~IAVCodecEngineObs() = default;

    virtual void OnError(int32_t errorType, int32_t errorCode) = 0;
    virtual void OnOutputFormatChanged(const Format &format) = 0;
    virtual void OnInputBufferAvailable(uint32_t index) = 0;
    virtual void OnOutputBufferAvailable(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag) = 0;
};

class IAVCodecEngine {
public:
    virtual ~IAVCodecEngine() = default;

    virtual int32_t Init(AVCodecType type, bool isMimeType, const std::string &name) = 0;
    virtual int32_t Configure(const Format &format) = 0;
    virtual int32_t Prepare() = 0;
    virtual int32_t Start() = 0;
    virtual int32_t Stop() = 0;
    virtual int32_t Flush() = 0;
    virtual int32_t Reset() = 0;
    virtual int32_t NotifyEos() = 0;
    virtual sptr<Surface> CreateInputSurface() = 0;
    virtual int32_t SetOutputSurface(const sptr<Surface> &surface) = 0;
    virtual std::shared_ptr<AVSharedMemory> GetInputBuffer(uint32_t index) = 0;
    virtual int32_t QueueInputBuffer(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag) = 0;
    virtual std::shared_ptr<AVSharedMemory> GetOutputBuffer(uint32_t index) = 0;
    virtual int32_t GetOutputFormat(Format &format) = 0;
    virtual int32_t ReleaseOutputBuffer(uint32_t index, bool render) = 0;
    virtual int32_t SetParameter(const Format &format) = 0;
    virtual int32_t SetObs(const std::weak_ptr<IAVCodecEngineObs> &obs) = 0;
};
} // namespace Media
} // namespace OHOS
#endif // IAVCODEC_ENGINE_H
