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

#ifndef HISTREAMER_FOUNDATION_AVBUFFER_QUEUE_PRODUCER_PROXY_H
#define HISTREAMER_FOUNDATION_AVBUFFER_QUEUE_PRODUCER_PROXY_H

#include "buffer/avbuffer_queue_producer.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace Media {

class AVBufferQueueProducerProxy : public IRemoteProxy<AVBufferQueueProducer> {
public:
    static std::shared_ptr<AVBufferQueueProducerProxy> Create(const sptr<IRemoteObject>& object);

    ~AVBufferQueueProducerProxy() override = default;
    AVBufferQueueProducerProxy(const AVBufferQueueProducerProxy&) = delete;

    Status RequestBuffer(std::shared_ptr<AVBuffer>& outBuffer,
                                  const AVBufferConfig& config, int32_t timeoutMs) override = 0;
    Status PushBuffer(const std::shared_ptr<AVBuffer>& inBuffer, bool cancel) override = 0;
    Status ReturnBuffer(const std::shared_ptr<AVBuffer>& inBuffer, bool cancel) override = 0;

    Status AttachBuffer(std::shared_ptr<AVBuffer>& inBuffer, bool isFilled) override = 0;
    Status DetachBuffer(const std::shared_ptr<AVBuffer>& outBuffer) override = 0;

    Status SetBufferFilledListener(sptr<IBrokerListener>& listener) override = 0;
    Status SetBufferAvailableListener(sptr<IProducerListener>& listener) override = 0;

protected:
    explicit AVBufferQueueProducerProxy(const sptr<IRemoteObject>& object);
};

}
}

#endif //HISTREAMER_FOUNDATION_AVBUFFER_QUEUE_PRODUCER_PROXY_H
