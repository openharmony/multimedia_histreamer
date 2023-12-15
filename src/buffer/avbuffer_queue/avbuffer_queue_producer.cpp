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

#include "avbuffer_queue_producer_impl.h"


namespace OHOS {
namespace Media {

AVBufferQueueProducerImpl::AVBufferQueueProducerImpl(std::shared_ptr<AVBufferQueueImpl>& bufferQueue)
    : AVBufferQueueProducerStub(), bufferQueue_(bufferQueue) {  // 由内部调用保证bufferQueue_不可能为空
}

uint32_t AVBufferQueueProducerImpl::GetQueueSize()
{
    return bufferQueue_->GetQueueSize();
}

Status AVBufferQueueProducerImpl::SetQueueSize(uint32_t size)
{
    return bufferQueue_->SetQueueSize(size);
}

Status AVBufferQueueProducerImpl::RequestBuffer(std::shared_ptr<AVBuffer>& buffer,
                                                 const AVBufferConfig& config, int32_t timeoutMs)
{
    return bufferQueue_->RequestBuffer(buffer, config, timeoutMs);
}

Status AVBufferQueueProducerImpl::PushBuffer(uint64_t uniqueId, bool available)
{
    return bufferQueue_->PushBuffer(uniqueId, available);
}

Status AVBufferQueueProducerImpl::PushBuffer(const std::shared_ptr<AVBuffer>& buffer, bool available)
{
    return bufferQueue_->PushBuffer(buffer, available);
}

Status AVBufferQueueProducerImpl::ReturnBuffer(uint64_t uniqueId, bool available)
{
    return bufferQueue_->ReturnBuffer(uniqueId, available);
}

Status AVBufferQueueProducerImpl::ReturnBuffer(const std::shared_ptr<AVBuffer>& buffer, bool available)
{
    return bufferQueue_->ReturnBuffer(buffer, available);
}

Status AVBufferQueueProducerImpl::AttachBuffer(std::shared_ptr<AVBuffer>& buffer, bool isFilled)
{
    return bufferQueue_->AttachBuffer(buffer, isFilled);
}

Status AVBufferQueueProducerImpl::DetachBuffer(uint64_t uniqueId)
{
    return bufferQueue_->DetachBuffer(uniqueId);
}

Status AVBufferQueueProducerImpl::DetachBuffer(const std::shared_ptr<AVBuffer>& buffer)
{
    return bufferQueue_->DetachBuffer(buffer);
}

Status AVBufferQueueProducerImpl::SetBufferFilledListener(sptr<IBrokerListener>& listener)
{
    return bufferQueue_->SetBrokerListener(listener);
}

Status AVBufferQueueProducerImpl::SetBufferAvailableListener(sptr<IProducerListener>& listener)
{
    return bufferQueue_->SetProducerListener(listener);
}

} // namespace Media
} // namespace OHOS