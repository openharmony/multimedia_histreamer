/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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

#define HST_LOG_TAG "DataStreamImpl"

#include "data_stream_impl.h"

#include "foundation/log.h"

namespace OHOS {
namespace Media {

DataStreamImpl::DataStreamImpl(size_t size, size_t count, MemoryType type)
{
    FALSE_LOG(type == MemoryType::MEMORY_VIRTUAL);
    for (size_t i = 0; i < count; ++i) {
        emptyBuffers_.push(std::make_shared<VirtualDataBuffer>(size));
    }
}

bool DataStreamImpl::GetDataBuffer(std::shared_ptr<DataBuffer>& buffer, int timeout)
{
    OSAL::ScopedLock lock(dataMutex_);
    if (dataBuffers_.empty()) {
        dataCondition_.Wait(lock, [this] { return !dataBuffers_.empty(); });
    }
    buffer = dataBuffers_.front();
    dataBuffers_.pop();
    return true;
}
bool DataStreamImpl::QueueEmptyBuffer(const std::shared_ptr<DataBuffer>& buffer)
{
    OSAL::ScopedLock lock(emptyMutex_);
    emptyBuffers_.push(buffer);
    emptyCondition_.NotifyAll();
    return true;
}

bool DataStreamImpl::GetEmptyBuffer(std::shared_ptr<DataBuffer>& buffer, int timeout)
{
    OSAL::ScopedLock lock(emptyMutex_);
    if (emptyBuffers_.empty()) {
        emptyCondition_.Wait(lock, [this] { return !emptyBuffers_.empty(); });
    }
    buffer = emptyBuffers_.front();
    emptyBuffers_.pop();
    return true;
}
bool DataStreamImpl::QueueDataBuffer(const std::shared_ptr<DataBuffer>& buffer)
{
    OSAL::ScopedLock lock(dataMutex_);
    dataBuffers_.push(buffer);
    dataCondition_.NotifyAll();
    return true;
}

VirtualDataBuffer::VirtualDataBuffer(size_t capacity) : capacity_(capacity)
{
    address_ = new uint8_t[capacity];
}
VirtualDataBuffer::~VirtualDataBuffer()
{
    delete [] address_;
}

bool VirtualDataBuffer::IsEos()
{
    return isEos_;
}
void VirtualDataBuffer::SetEos(bool isEos)
{
    isEos_ = isEos;
}
uint8_t* VirtualDataBuffer::GetAddress()
{
    return address_;
}
size_t VirtualDataBuffer::GetCapacity()
{
    return capacity_;
}
size_t VirtualDataBuffer::GetSize()
{
    return size_;
}
void VirtualDataBuffer::SetSize(size_t size)
{
    size_ = size;
}

} // Media
} // OHOS