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

#ifndef HISTREAMER_FOUNDATION_AVBUFFER_QUEUE_PRODUCER_H
#define HISTREAMER_FOUNDATION_AVBUFFER_QUEUE_PRODUCER_H

#include "buffer/avbuffer_queue_define.h"
#include "iremote_stub.h"
#include "surface.h"

namespace OHOS {
namespace Media {

class AVBufferQueueProducer: public virtual IRemoteBroker {
public:
    virtual uint32_t GetQueueSize() = 0;
    virtual Status SetQueueSize(uint32_t size) = 0;

    virtual Status RequestBuffer(std::shared_ptr<AVBuffer>& outBuffer,
                                 const AVBufferConfig& config, int32_t timeoutMs) = 0;
    virtual Status PushBuffer(const std::shared_ptr<AVBuffer>& inBuffer, bool available) = 0;
    virtual Status ReturnBuffer(const std::shared_ptr<AVBuffer>& inBuffer, bool available) = 0;

    virtual Status AttachBuffer(std::shared_ptr<AVBuffer>& inBuffer, bool isFilled) = 0;
    virtual Status DetachBuffer(const std::shared_ptr<AVBuffer>& outBuffer) = 0;

    virtual Status SetBufferFilledListener(sptr<IBrokerListener>& listener) = 0;
    virtual Status SetBufferAvailableListener(sptr<IProducerListener>& listener) = 0;

    DECLARE_INTERFACE_DESCRIPTOR(u"Media.AVBufferQueueProducer");

protected:
    enum: uint32_t {
        PRODUCER_GET_QUEUE_SIZE = 0,
        PRODUCER_SET_QUEUE_SIZE = 1,
        PRODUCER_REQUEST_BUFFER = 2,
        PRODUCER_PUSH_BUFFER = 3,
        PRODUCER_RETURN_BUFFER = 4,
        PRODUCER_ATTACH_BUFFER = 5,
        PRODUCER_DETACH_BUFFER = 6,
        PRODUCER_SET_FILLED_LISTENER = 7,
        PRODUCER_SET_AVAILABLE_LISTENER = 8
    };
};

} // namespace Media
} // namespace OHOS

#endif // HISTREAMER_FOUNDATION_AVBUFFER_QUEUE_PRODUCER_H
