/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef NATIVE_AVMAGIC_H
#define NATIVE_AVMAGIC_H

#include <refbase.h>
#include "avcodec_common.h"
#include "avcodec_info.h"
#include "avsharedmemory.h"
#include "avbuffer.h"
// #include "avbufferqueue.h"
#include "format.h"

#define AV_MAGIC(a, b, c, d) (((a) << 24) + ((b) << 16) + ((c) << 8) + ((d) << 0))

enum class AVMagic {
    AVCODEC_MAGIC_VIDEO_DECODER = AV_MAGIC('V', 'D', 'E', 'C'),
    AVCODEC_MAGIC_VIDEO_ENCODER = AV_MAGIC('V', 'E', 'N', 'C'),
    AVCODEC_MAGIC_AUDIO_DECODER = AV_MAGIC('A', 'D', 'E', 'C'),
    AVCODEC_MAGIC_AUDIO_ENCODER = AV_MAGIC('A', 'E', 'N', 'C'),
    AVCODEC_MAGIC_AVMUXER = AV_MAGIC('M', 'U', 'X', 'R'),
    AVCODEC_MAGIC_AVDEMUXER = AV_MAGIC('D', 'M', 'U', 'X'),
    AVCODEC_MAGIC_AVSOURCE = AV_MAGIC('S', 'O', 'U', 'C'),
    AVCODEC_MAGIC_AVBUFFER = AV_MAGIC('B', 'B', 'U', 'F'),
    // AVCODEC_MAGIC_AVBUFFER_QUEUE = AV_MAGIC('Q', 'Q', 'U', 'E'),
    AVCODEC_MAGIC_FORMAT = AV_MAGIC('F', 'R', 'M', 'T'),
    AVCODEC_MAGIC_SHARED_MEMORY = AV_MAGIC('S', 'M', 'E', 'M'),
};

struct AVObjectMagic : public OHOS::RefBase {
    explicit AVObjectMagic(enum AVMagic m) : magic_(m) {}
    virtual ~AVObjectMagic() = default;
    enum AVMagic magic_;
};

struct OH_AVFormat : public AVObjectMagic {
    OH_AVFormat();
    explicit OH_AVFormat(const OHOS::MediaAVCodec::Format &fmt);
    ~OH_AVFormat() override;
    OHOS::MediaAVCodec::Format format_;
    char *outString_ = nullptr;
    char *dumpInfo_ = nullptr;
};

struct OH_AVMemory : public AVObjectMagic {
    explicit OH_AVMemory(const std::shared_ptr<OHOS::MediaAVCodec::AVSharedMemory> &mem);
    ~OH_AVMemory() override;
    bool IsEqualMemory(const std::shared_ptr<OHOS::MediaAVCodec::AVSharedMemory> &mem);
    const std::shared_ptr<OHOS::MediaAVCodec::AVSharedMemory> memory_;
    bool isUserCreated = false;
};

struct OH_AVBuffer : public AVObjectMagic {
    explicit OH_AVBuffer(const std::shared_ptr<OHOS::MediaAVCodec::AVBuffer> &buf);
    virtual ~OH_AVBuffer() = default;
    bool IsEqualBuffer(const std::shared_ptr<OHOS::MediaAVCodec::AVBuffer> &buf);
    std::shared_ptr<OHOS::MediaAVCodec::AVBuffer> buffer_;
    bool isUserCreated = false;
};

// struct OH_AVBufferQueue : public AVObjectMagic {
//     explicit OH_AVBufferQueue(const std::shared_ptr<OHOS::MediaAVCodec::AVBufferQueue> &bufferQueue);
//     virtual ~OH_AVBufferQueue() = default;
//     const std::shared_ptr<OHOS::MediaAVCodec::AVBufferQueue> bufferQueue_;
//     std::shared_ptr<OHOS::MediaAVCodec::AVBufferQueueCallback> callback_;
// };

struct OH_AVCodec : public AVObjectMagic {
    explicit OH_AVCodec(enum AVMagic m) : AVObjectMagic(m) {}
    virtual ~OH_AVCodec() = default;
};

struct OH_AVCapability : public OHOS::RefBase {
    explicit OH_AVCapability(OHOS::MediaAVCodec::CapabilityData *capabilityData);
    ~OH_AVCapability() override;
    OHOS::MediaAVCodec::CapabilityData *capabilityData_;
};

struct OH_AVMuxer : public AVObjectMagic {
    explicit OH_AVMuxer(enum AVMagic m) : AVObjectMagic(m) {}
    virtual ~OH_AVMuxer() = default;
};

struct OH_AVDemuxer : public AVObjectMagic {
    explicit OH_AVDemuxer(enum AVMagic m) : AVObjectMagic(m) {}
    virtual ~OH_AVDemuxer() = default;
};

struct OH_AVSource : public AVObjectMagic {
    explicit OH_AVSource(enum AVMagic m) : AVObjectMagic(m) {}
    virtual ~OH_AVSource() = default;
};
#endif // NATIVE_AVMAGIC_H