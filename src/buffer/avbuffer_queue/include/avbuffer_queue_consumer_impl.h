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

#ifndef HISTREAMER_FOUNDATION_AVBUFFER_QUEUE_CONSUMER_IMPL_H
#define HISTREAMER_FOUNDATION_AVBUFFER_QUEUE_CONSUMER_IMPL_H

#include "avbuffer_queue_impl.h"

namespace OHOS {
namespace Media {

class AVBufferQueueConsumerImpl : public AVBufferQueueConsumer {
public:
    explicit AVBufferQueueConsumerImpl(std::shared_ptr<AVBufferQueueImpl>& bufferQueue);
    ~AVBufferQueueConsumerImpl() override = default;
    AVBufferQueueConsumerImpl(const AVBufferQueueConsumerImpl&) = delete;
    AVBufferQueueConsumerImpl operator=(const AVBufferQueueConsumerImpl&) = delete;

    uint32_t GetQueueSize() override;
    Status SetQueueSize(uint32_t size) override;
    bool IsBufferInQueue(const std::shared_ptr<AVBuffer>& buffer) override;

    Status AcquireBuffer(std::shared_ptr<AVBuffer>& buffer) override;
    Status ReleaseBuffer(const std::shared_ptr<AVBuffer>& buffer) override;

    Status AttachBuffer(std::shared_ptr<AVBuffer>& buffer, bool isFilled) override;
    Status DetachBuffer(const std::shared_ptr<AVBuffer>& buffer) override;

    Status SetBufferAvailableListener(sptr<IConsumerListener>& listener) override;

private:
    std::shared_ptr<AVBufferQueueImpl> bufferQueue_;
};

}
}

#endif // HISTREAMER_FOUNDATION_AVBUFFER_QUEUE_CONSUMER_IMPL_H
