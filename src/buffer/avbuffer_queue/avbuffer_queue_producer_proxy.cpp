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

#include "buffer/avbuffer_queue_producer_proxy.h"
#include "avbuffer_utils.h"
#include "common/log.h"

namespace OHOS {
namespace Media {

#define ABQ_IPC_DEFINE_VARIABLES                                                  \
    MessageParcel arguments;                                                      \
    MessageParcel reply;                                                          \
    MessageOption option;                                                         \
    FALSE_RETURN_V(arguments.WriteInterfaceToken(GetDescriptor()),                \
        Status::ERROR_IPC_WRITE_INTERFACE_TOKEN)

#define ABQ_IPC_SEND_REQUEST(command)                                             \
    do {                                                                          \
        NZERO_RETURN_V(Remote()->SendRequest(command, arguments, reply, option),  \
            Status::ERROR_IPC_SEND_REQUEST);                                      \
        NOK_RETURN(static_cast<Status>(reply.ReadInt32()));                       \
    } while (0)


class AVBufferQueueProducerProxyImpl : public AVBufferQueueProducerProxy {
public:
    explicit AVBufferQueueProducerProxyImpl(const sptr<IRemoteObject>& object)
        : AVBufferQueueProducerProxy(object) { }
    ~AVBufferQueueProducerProxyImpl() override = default;
    AVBufferQueueProducerProxyImpl(const AVBufferQueueProducerProxyImpl&) = delete;
    AVBufferQueueProducerProxyImpl operator=(const AVBufferQueueProducerProxyImpl&) = delete;

    uint32_t GetQueueSize() override;
    Status SetQueueSize(uint32_t size) override;

    Status RequestBuffer(std::shared_ptr<AVBuffer>& outBuffer,
        const AVBufferConfig& config, int32_t timeoutMs) override;
    Status PushBuffer(const std::shared_ptr<AVBuffer>& inBuffer, bool available) override;
    Status ReturnBuffer(const std::shared_ptr<AVBuffer>& inBuffer, bool available) override;

    Status AttachBuffer(std::shared_ptr<AVBuffer>& inBuffer, bool isFilled) override;
    Status DetachBuffer(const std::shared_ptr<AVBuffer>& outBuffer) override;

    Status SetBufferFilledListener(sptr<IBrokerListener>& listener) override;
    Status SetBufferAvailableListener(sptr<IProducerListener>& listener) override;

private:
    static inline BrokerDelegator<AVBufferQueueProducerProxyImpl> delegator_;
};

std::shared_ptr<AVBufferQueueProducerProxy> AVBufferQueueProducerProxy::Create(const sptr<IRemoteObject>& object)
{
    FALSE_RETURN_V(object != nullptr, nullptr);
    return std::make_shared<AVBufferQueueProducerProxyImpl>(object);
}

AVBufferQueueProducerProxy::AVBufferQueueProducerProxy(const sptr<IRemoteObject>& object)
    : IRemoteProxy<AVBufferQueueProducer>(object) { }

uint32_t AVBufferQueueProducerProxyImpl::GetQueueSize()
{
    MessageParcel arguments;
    MessageParcel reply;
    MessageOption option;
    FALSE_RETURN_V(arguments.WriteInterfaceToken(GetDescriptor()), 0);
    NZERO_RETURN_V(Remote()->SendRequest(PRODUCER_GET_QUEUE_SIZE, arguments, reply, option), 0);

    return reply.ReadUint32();
}

Status AVBufferQueueProducerProxyImpl::SetQueueSize(uint32_t size)
{
    ABQ_IPC_DEFINE_VARIABLES;

    arguments.WriteUint32(size);

    ABQ_IPC_SEND_REQUEST(PRODUCER_SET_QUEUE_SIZE);

    return Status::OK;
}

Status AVBufferQueueProducerProxyImpl::RequestBuffer(std::shared_ptr<AVBuffer>& outBuffer,
                                                     const AVBufferConfig& config, int32_t timeoutMs)
{
    ABQ_IPC_DEFINE_VARIABLES;

    MarshallingConfig(arguments, config);

    ABQ_IPC_SEND_REQUEST(PRODUCER_REQUEST_BUFFER);

    outBuffer = AVBuffer::CreateAVBuffer();
    FALSE_RETURN_V(outBuffer != nullptr, Status::ERROR_CREATE_BUFFER);
    outBuffer->ReadFromMessageParcel(reply);

    return Status::OK;
}

Status AVBufferQueueProducerProxyImpl::PushBuffer(const std::shared_ptr<AVBuffer>& inBuffer, bool available)
{
    FALSE_RETURN_V(inBuffer != nullptr, Status::ERROR_NULL_POINT_BUFFER);

    ABQ_IPC_DEFINE_VARIABLES;

    arguments.WriteUint64(inBuffer->GetUniqueId());
    arguments.WriteBool(available);

    ABQ_IPC_SEND_REQUEST(PRODUCER_PUSH_BUFFER);

    return Status::OK;
}

Status AVBufferQueueProducerProxyImpl::ReturnBuffer(const std::shared_ptr<AVBuffer>& inBuffer, bool available)
{
    FALSE_RETURN_V(inBuffer != nullptr, Status::ERROR_NULL_POINT_BUFFER);

    ABQ_IPC_DEFINE_VARIABLES;

    arguments.WriteUint64(inBuffer->GetUniqueId());
    arguments.WriteBool(available);

    ABQ_IPC_SEND_REQUEST(PRODUCER_RETURN_BUFFER);

    return Status::OK;
}

Status AVBufferQueueProducerProxyImpl::AttachBuffer(std::shared_ptr<AVBuffer>& inBuffer, bool isFilled)
{
    FALSE_RETURN_V(inBuffer != nullptr, Status::ERROR_NULL_POINT_BUFFER);

    ABQ_IPC_DEFINE_VARIABLES;

    inBuffer->WriteToMessageParcel(arguments);
    arguments.WriteBool(isFilled);

    ABQ_IPC_SEND_REQUEST(PRODUCER_ATTACH_BUFFER);

    return Status::OK;
}

Status AVBufferQueueProducerProxyImpl::DetachBuffer(const std::shared_ptr<AVBuffer>& outBuffer)
{
    FALSE_RETURN_V(outBuffer != nullptr, Status::ERROR_NULL_POINT_BUFFER);

    ABQ_IPC_DEFINE_VARIABLES;

    arguments.WriteUint64(outBuffer->GetUniqueId());

    ABQ_IPC_SEND_REQUEST(PRODUCER_DETACH_BUFFER);

    return Status::OK;
}

Status AVBufferQueueProducerProxyImpl::SetBufferFilledListener(sptr<IBrokerListener>& listener)
{
    ABQ_IPC_DEFINE_VARIABLES;

    arguments.WriteRemoteObject(listener->AsObject());

    ABQ_IPC_SEND_REQUEST(PRODUCER_SET_FILLED_LISTENER);

    return Status::OK;
}

Status AVBufferQueueProducerProxyImpl::SetBufferAvailableListener(sptr<IProducerListener>& listener)
{
    ABQ_IPC_DEFINE_VARIABLES;

    arguments.WriteRemoteObject(listener->AsObject());

    ABQ_IPC_SEND_REQUEST(PRODUCER_SET_AVAILABLE_LISTENER);

    return Status::OK;
}

} // namespace Media
} // namespace OHOS