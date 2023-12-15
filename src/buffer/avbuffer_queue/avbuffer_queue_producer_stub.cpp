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

#include "avbuffer_utils.h"
#include "avbuffer_queue_producer_impl.h"
#include "common/log.h"


namespace OHOS {
namespace Media {

AVBufferQueueProducerStub::AVBufferQueueProducerStub()
{
    stubFuncMap_[PRODUCER_GET_QUEUE_SIZE] = &AVBufferQueueProducerStub::OnGetQueueSize;
    stubFuncMap_[PRODUCER_SET_QUEUE_SIZE] = &AVBufferQueueProducerStub::OnSetQueueSize;
    stubFuncMap_[PRODUCER_REQUEST_BUFFER] = &AVBufferQueueProducerStub::OnRequestBuffer;
    stubFuncMap_[PRODUCER_PUSH_BUFFER] = &AVBufferQueueProducerStub::OnPushBuffer;
    stubFuncMap_[PRODUCER_RETURN_BUFFER] = &AVBufferQueueProducerStub::OnReturnBuffer;
    stubFuncMap_[PRODUCER_ATTACH_BUFFER] = &AVBufferQueueProducerStub::OnAttachBuffer;
    stubFuncMap_[PRODUCER_DETACH_BUFFER] = &AVBufferQueueProducerStub::OnDetachBuffer;
    stubFuncMap_[PRODUCER_SET_FILLED_LISTENER] = &AVBufferQueueProducerStub::OnSetBufferFilledListener;
    stubFuncMap_[PRODUCER_SET_AVAILABLE_LISTENER] = &AVBufferQueueProducerStub::OnSetBufferAvailableListener;
}

int AVBufferQueueProducerStub::OnRemoteRequest(
        uint32_t code, MessageParcel& arguments, MessageParcel& reply, MessageOption& option)
{
    auto it = stubFuncMap_.find(code);
    FALSE_RETURN_V(it != stubFuncMap_.end(), IPC_STUB_INVALID_DATA_ERR);
    FALSE_RETURN_V(it->second != nullptr, IPC_STUB_ERR);
    FALSE_RETURN_V(GetDescriptor() == arguments.ReadInterfaceToken(), ERR_INVALID_STATE);

    return (this->*(it->second))(arguments, reply, option);
}

int32_t AVBufferQueueProducerStub::OnGetQueueSize(
        MessageParcel& arguments, MessageParcel& reply, MessageOption& option)
{
    auto size = GetQueueSize();

    reply.WriteInt32(0);
    reply.WriteUint32(size);

    return 0;
}

int32_t AVBufferQueueProducerStub::OnSetQueueSize(
        MessageParcel& arguments, MessageParcel& reply, MessageOption& option)
{
    auto size = arguments.ReadUint32();
    auto ret = SetQueueSize(size);

    reply.WriteInt32(static_cast<int32_t>(ret));

    return 0;
}

int32_t AVBufferQueueProducerStub::OnRequestBuffer(
        MessageParcel& arguments, MessageParcel& reply, MessageOption& option)
{
    std::shared_ptr<AVBuffer> buffer = nullptr;
    AVBufferConfig config;
    UnmarshallingConfig(arguments, config);
    auto timeoutMs = arguments.ReadInt32();

    auto ret = RequestBuffer(buffer, config, timeoutMs);

    reply.WriteInt32(static_cast<int32_t>(ret));
    if (ret == Status::OK) {
        buffer->WriteToMessageParcel(reply);
    }

    return 0;
}

int32_t AVBufferQueueProducerStub::OnPushBuffer(
        MessageParcel& arguments, MessageParcel& reply, MessageOption& option)
{
    auto uniqueId = arguments.ReadUint64();
    auto available = arguments.ReadBool();

    auto ret = PushBuffer(uniqueId, available);
    reply.WriteInt32(static_cast<int32_t>(ret));

    return 0;
}

int32_t AVBufferQueueProducerStub::OnReturnBuffer(
        MessageParcel& arguments, MessageParcel& reply, MessageOption& option)
{
    auto uniqueId = arguments.ReadUint64();
    auto available = arguments.ReadBool();

    auto ret = ReturnBuffer(uniqueId, available);
    reply.WriteInt32(static_cast<int32_t>(ret));

    return 0;
}

int32_t AVBufferQueueProducerStub::OnAttachBuffer(
        MessageParcel& arguments, MessageParcel& reply, MessageOption& option)
{
    auto buffer = AVBuffer::CreateAVBuffer();
    buffer->ReadFromMessageParcel(arguments);
    auto isFilled = arguments.ReadBool();

    auto ret = AttachBuffer(buffer, isFilled);
    reply.WriteInt32(static_cast<int32_t>(ret));

    return 0;
}

int32_t AVBufferQueueProducerStub::OnDetachBuffer(
        MessageParcel& arguments, MessageParcel& reply, MessageOption& option)
{
    auto uniqueId = arguments.ReadUint64();

    auto ret = DetachBuffer(uniqueId);
    reply.WriteInt32(static_cast<int32_t>(ret));

    return 0;
}

int32_t AVBufferQueueProducerStub::OnSetBufferFilledListener(
        MessageParcel& arguments, MessageParcel& reply, MessageOption& option)
{
    auto listenerObject = arguments.ReadRemoteObject();
    sptr<IBrokerListener> listener = iface_cast<IBrokerListener>(listenerObject);

    auto ret = SetBufferFilledListener(listener);
    reply.WriteInt32(static_cast<int32_t>(ret));

    return 0;
}

int32_t AVBufferQueueProducerStub::OnSetBufferAvailableListener(
        MessageParcel& arguments, MessageParcel& reply, MessageOption& option)
{
    auto listenerObject = arguments.ReadRemoteObject();
    sptr<IProducerListener> listener = iface_cast<IProducerListener>(listenerObject);
    auto ret = SetBufferAvailableListener(listener);
    reply.WriteInt32(static_cast<int32_t>(ret));

    return 0;
}

} // namespace Media
} // namespace OHOS
