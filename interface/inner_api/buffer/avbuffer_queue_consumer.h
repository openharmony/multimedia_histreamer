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

#ifndef HISTREAMER_FOUNDATION_AVBUFFER_QUEUE_CONSUMER_H
#define HISTREAMER_FOUNDATION_AVBUFFER_QUEUE_CONSUMER_H

#include "buffer/avbuffer_queue_define.h"

namespace OHOS {
namespace Media {

class AVBufferQueueConsumer : public RefBase {
public:
    ~AVBufferQueueConsumer() override = default;
    AVBufferQueueConsumer(const AVBufferQueueConsumer&) = delete;

    virtual uint32_t GetQueueSize() = 0;
    virtual Status SetQueueSize(uint32_t size) = 0;
    virtual bool IsBufferInQueue(const std::shared_ptr<AVBuffer>& buffer) = 0;

    virtual Status AcquireBuffer(std::shared_ptr<AVBuffer>& outBuffer) = 0;
    virtual Status ReleaseBuffer(const std::shared_ptr<AVBuffer>& inBuffer) = 0;

    virtual Status AttachBuffer(std::shared_ptr<AVBuffer>& inBuffer, bool isFilled) = 0;
    virtual Status DetachBuffer(const std::shared_ptr<AVBuffer>& outBuffer) = 0;

    virtual Status SetBufferAvailableListener(sptr<IConsumerListener>& listener) = 0;

protected:
    AVBufferQueueConsumer() = default;
};

} // namespace Media
} // namespace OHOS

#endif // HISTREAMER_FOUNDATION_AVBUFFER_QUEUE_CONSUMER_H
