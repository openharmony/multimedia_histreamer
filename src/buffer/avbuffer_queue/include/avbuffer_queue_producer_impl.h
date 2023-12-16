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

#ifndef HISTREAMER_FOUNDATION_AVBUFFER_QUEUE_PRODUCER_IMPL_H
#define HISTREAMER_FOUNDATION_AVBUFFER_QUEUE_PRODUCER_IMPL_H

#include "avbuffer_queue_impl.h"

namespace OHOS {
namespace Media {

class AVBufferQueueProducerStub : public IRemoteStub<AVBufferQueueProducer> {
public:
    AVBufferQueueProducerStub();
    ~AVBufferQueueProducerStub() override = default;

    int OnRemoteRequest(uint32_t code, MessageParcel &arguments, MessageParcel &reply, MessageOption &option) override;

    uint32_t GetQueueSize() override = 0;
    Status SetQueueSize(uint32_t size) override = 0;

    Status RequestBuffer(std::shared_ptr<AVBuffer>& outBuffer,
                         const AVBufferConfig& config, int32_t timeoutMs) override = 0;
    Status PushBuffer(const std::shared_ptr<AVBuffer>& inBuffer, bool available) override = 0;
    Status ReturnBuffer(const std::shared_ptr<AVBuffer>& inBuffer, bool available) override = 0;

    Status AttachBuffer(std::shared_ptr<AVBuffer>& inBuffer, bool isFilled) override = 0;
    Status DetachBuffer(const std::shared_ptr<AVBuffer>& outBuffer) override = 0;

    Status SetBufferFilledListener(sptr<IBrokerListener>& listener) override = 0;
    Status SetBufferAvailableListener(sptr<IProducerListener>& listener) override = 0;

    virtual Status PushBuffer(uint64_t uniqueId, bool available) = 0;
    virtual Status ReturnBuffer(uint64_t uniqueId, bool available) = 0;
    virtual Status DetachBuffer(uint64_t uniqueId) = 0;

private:
    using StubFunc = int32_t (AVBufferQueueProducerStub::*)(MessageParcel&, MessageParcel&, MessageOption&);

    std::map<uint32_t, StubFunc>  stubFuncMap_;

    int32_t OnGetQueueSize(MessageParcel& arguments, MessageParcel& reply, MessageOption& option);
    int32_t OnSetQueueSize(MessageParcel& arguments, MessageParcel& reply, MessageOption& option);

    int32_t OnRequestBuffer(MessageParcel& arguments, MessageParcel& reply, MessageOption& option);
    int32_t OnPushBuffer(MessageParcel& arguments, MessageParcel& reply, MessageOption& option);
    int32_t OnReturnBuffer(MessageParcel& arguments, MessageParcel& reply, MessageOption& option);

    int32_t OnAttachBuffer(MessageParcel& arguments, MessageParcel& reply, MessageOption& option);
    int32_t OnDetachBuffer(MessageParcel& arguments, MessageParcel& reply, MessageOption& option);

    int32_t OnSetBufferFilledListener(MessageParcel& arguments, MessageParcel& reply, MessageOption& option);
    int32_t OnSetBufferAvailableListener(MessageParcel& arguments, MessageParcel& reply, MessageOption& option);
};

class AVBufferQueueProducerImpl: public AVBufferQueueProducerStub {
public:
    explicit AVBufferQueueProducerImpl(std::shared_ptr<AVBufferQueueImpl>& bufferQueue);
    ~AVBufferQueueProducerImpl() override = default;
    AVBufferQueueProducerImpl(const AVBufferQueueProducerImpl&) = delete;
    AVBufferQueueProducerImpl operator= (const AVBufferQueueProducerImpl&) = delete;

    uint32_t GetQueueSize() override;
    Status SetQueueSize(uint32_t size) override;

    Status RequestBuffer(std::shared_ptr<AVBuffer>& buffer,
                          const AVBufferConfig& config, int32_t timeoutMs) override;
    Status PushBuffer(const std::shared_ptr<AVBuffer>& buffer, bool available) override;
    Status ReturnBuffer(const std::shared_ptr<AVBuffer>& buffer, bool available) override;

    Status AttachBuffer(std::shared_ptr<AVBuffer>& buffer, bool isFilled) override;
    Status DetachBuffer(const std::shared_ptr<AVBuffer>& buffer) override;

    Status SetBufferFilledListener(sptr<IBrokerListener>& listener) override;
    Status SetBufferAvailableListener(sptr<IProducerListener>& listener) override;

protected:
    std::shared_ptr<AVBufferQueueImpl> bufferQueue_;

    Status PushBuffer(uint64_t uniqueId, bool available) override;
    Status ReturnBuffer(uint64_t uniqueId, bool available) override;
    Status DetachBuffer(uint64_t uniqueId) override;
};

} // namespace Media
} // namespace OHOS

#endif // HISTREAMER_FOUNDATION_AVBUFFER_QUEUE_PRODUCER_IMPL_H
