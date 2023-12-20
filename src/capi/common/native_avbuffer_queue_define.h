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

#ifndef NATIVE_AVBUFFER_QUEUE_DEFINE_H
#define NATIVE_AVBUFFER_QUEUE_DEFINE_H

#include <refbase.h>
#include <list>
#include <vector>
#include <mutex>
#include "buffer/avbuffer_queue.h"

#define AV_BUFFER_QUEUE_MAGIC(a, b, c, d) (((a) << 24) + ((b) << 16) + ((c) << 8) + ((d) << 0))

using NativeAVBufferQueueObjectMagic = enum NativeAVBufferQueueObjectMagic: uint32_t {
    NATIVE_ABQ_OBJECT_MAGIC_QUEUE = AV_BUFFER_QUEUE_MAGIC('Q', 'U', 'E', 'E'),
    NATIVE_ABQ_OBJECT_MAGIC_PRODUCER = AV_BUFFER_QUEUE_MAGIC('Q', 'P', 'D', 'C'),
    NATIVE_ABQ_OBJECT_MAGIC_CONSUMER = AV_BUFFER_QUEUE_MAGIC('Q', 'C', 'S', 'M'),
    NATIVE_ABQ_OBJECT_MAGIC_INVALID = AV_BUFFER_QUEUE_MAGIC('0', '0', '0', '0'),
};

std::list<NativeAVBufferQueueObjectMagic> NativeAVBufferQueueObjectMagicList = {
    NATIVE_ABQ_OBJECT_MAGIC_QUEUE,
    NATIVE_ABQ_OBJECT_MAGIC_PRODUCER,
    NATIVE_ABQ_OBJECT_MAGIC_CONSUMER,
};

struct NativeAVBufferQueueMagic : public OHOS::RefBase {
    explicit NativeAVBufferQueueMagic(NativeAVBufferQueueObjectMagic m) : magic_(m) {}
    ~NativeAVBufferQueueMagic() override { magic_ = NATIVE_ABQ_OBJECT_MAGIC_INVALID; }
    NativeAVBufferQueueObjectMagic magic_;
};

struct OH_AVBufferQueue: public NativeAVBufferQueueMagic {
    explicit OH_AVBufferQueue(
            std::shared_ptr<OHOS::Media::AVBufferQueue>& bufferQueue, uint32_t maxBufferCount);
    ~OH_AVBufferQueue() override = default;

    std::shared_ptr<OHOS::Media::AVBufferQueue> bufferQueue_;
    std::vector<std::shared_ptr<OH_AVBuffer>> nativeBufferVector_;
    std::vector<bool> nativeBufferFree_;

    std::mutex lock_;

    OH_AVBuffer* FindAndBindNativeBuffer(std::shared_ptr<OHOS::Media::AVBuffer>& buffer);
    OH_AVErrCode UnbindNativeBuffer(const OH_AVBuffer *nativeBuffer);
};

struct OH_AVBufferQueueConsumer : public NativeAVBufferQueueMagic {
    explicit OH_AVBufferQueueConsumer(
        OHOS::sptr<OH_AVBufferQueue>& nativeBufferQueue,
        OHOS::sptr<OHOS::Media::AVBufferQueueConsumer>& consumer)
        : NativeAVBufferQueueMagic(NATIVE_ABQ_OBJECT_MAGIC_CONSUMER),
          nativeBufferQueue_(nativeBufferQueue),
          consumer_(consumer) { }

    ~OH_AVBufferQueueConsumer() override { nativeBufferQueue_ = nullptr; }

    OHOS::sptr<OH_AVBufferQueue> nativeBufferQueue_;
    OHOS::sptr<OHOS::Media::AVBufferQueueConsumer> consumer_;
};

struct OH_AVBufferQueueProducer : public NativeAVBufferQueueMagic {
    explicit OH_AVBufferQueueProducer(
        OHOS::sptr<OH_AVBufferQueue>& nativeBufferQueue,
        OHOS::sptr<OHOS::Media::AVBufferQueueProducer>& producer)
        : NativeAVBufferQueueMagic(NATIVE_ABQ_OBJECT_MAGIC_PRODUCER),
          nativeBufferQueue_(nativeBufferQueue),
          producer_(producer) { }

    ~OH_AVBufferQueueProducer() override { nativeBufferQueue_ = nullptr; }

    OHOS::sptr<OH_AVBufferQueue> nativeBufferQueue_;
    OHOS::sptr<OHOS::Media::AVBufferQueueProducer> producer_;
};

#endif // NATIVE_AVBUFFER_QUEUE_DEFINE_H