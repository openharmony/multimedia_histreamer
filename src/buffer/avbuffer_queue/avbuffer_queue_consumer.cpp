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

#include "avbuffer_queue_consumer_impl.h"

namespace OHOS {
namespace Media {

AVBufferQueueConsumerImpl::AVBufferQueueConsumerImpl(std::shared_ptr<AVBufferQueueImpl>& bufferQueue)
    : AVBufferQueueConsumer(), bufferQueue_(bufferQueue) {
}

uint32_t AVBufferQueueConsumerImpl::GetQueueSize()
{
    return bufferQueue_->GetQueueSize();
}

Status AVBufferQueueConsumerImpl::SetQueueSize(uint32_t size)
{
    return bufferQueue_->SetQueueSize(size);
}

bool AVBufferQueueConsumerImpl::IsBufferInQueue(const std::shared_ptr<AVBuffer>& buffer)
{
    return bufferQueue_->IsBufferInQueue(buffer);
}

Status AVBufferQueueConsumerImpl::AcquireBuffer(std::shared_ptr<AVBuffer>& buffer)
{
    return bufferQueue_->AcquireBuffer(buffer);
}

Status AVBufferQueueConsumerImpl::ReleaseBuffer(const std::shared_ptr<AVBuffer>& buffer)
{
    return bufferQueue_->ReleaseBuffer(buffer);
}

Status AVBufferQueueConsumerImpl::AttachBuffer(std::shared_ptr<AVBuffer>& buffer, bool isFilled)
{
    return bufferQueue_->AttachBuffer(buffer, isFilled);
}

Status AVBufferQueueConsumerImpl::DetachBuffer(const std::shared_ptr<AVBuffer>& buffer)
{
    return bufferQueue_->DetachBuffer(buffer);
}

Status AVBufferQueueConsumerImpl::SetBufferAvailableListener(sptr<IConsumerListener>& listener)
{
    return bufferQueue_->SetConsumerListener(listener);
}

} // namespace Media
} // namespace OHOS
