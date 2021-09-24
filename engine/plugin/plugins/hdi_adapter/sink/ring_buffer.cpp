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

#define LOG_TAG "HdiRingBuffer"

#include "ring_buffer.h"

#include "foundation/log.h"
#include "foundation/memory_helper.h"
#include "plugin/common/plugin_buffer.h"

namespace OHOS {
namespace Media {
namespace HosLitePlugin {
bool RingBuffer::Init()
{
    buffer_ = OHOS::Media::MemoryHelper::make_unique<uint8_t[]>(bufferSize_);
    return buffer_ != nullptr;
}
std::shared_ptr<uint8_t> RingBuffer::ReadBufferWithoutAdvance(size_t readSize, size_t& outSize)
{
    OHOS::Media::OSAL::ScopedLock lck(writeMutex_);
    std::shared_ptr<uint8_t> ptr;
    if (!isActive_) {
        ptr = nullptr;
        outSize = 0;
        return ptr;
    }
    auto available = tail_ - head_;
    available = (available > readSize) ? readSize : available;
    size_t index = head_ % bufferSize_;
    if (index + available < bufferSize_) {
        ptr = std::shared_ptr<uint8_t>(buffer_.get() + index, [](uint8_t* ptr) {}); // do not delete memory
        outSize = available;
        return ptr;
    }

    ptr = std::shared_ptr<uint8_t>(new (std::nothrow) uint8_t[available], std::default_delete<uint8_t[]>());
    if (ptr == nullptr) {
        outSize = 0;
    } else {
        outSize = available;
        if (memcpy_s(ptr.get(), bufferSize_ - index, buffer_.get() + index, bufferSize_ - index) != EOK) {
            MEDIA_LOG_E("memcpy_s failed when read buffer");
            outSize = 0;
        }
        if (available - (bufferSize_ - index) > 0 && memcpy_s(ptr.get() + (bufferSize_ - index),
            available - (bufferSize_ - index), buffer_.get(), available - (bufferSize_ - index)) != EOK) {
            MEDIA_LOG_E("memcpy_s failed when read buffer");
            outSize = 0;
        }
    }
    return ptr;
}
void RingBuffer::Advance(size_t size)
{
    OHOS::Media::OSAL::ScopedLock lck(writeMutex_);
    if (!isActive_) {
        return;
    }
    head_ += size;
    writeCondition_.NotifyAll();
}
void RingBuffer::WriteBuffer(const std::shared_ptr<OHOS::Media::Plugin::Buffer>& inputInfo)
{
    auto mem = inputInfo->GetMemory();
    size_t writeSize = mem->GetSize();
    auto ptr = mem->GetReadOnlyData();
    OHOS::Media::OSAL::ScopedLock lck(writeMutex_);
    if (!isActive_) {
        return;
    }
    while (writeSize + tail_ > head_ + bufferSize_) {
        writeCondition_.Wait(lck);
        if (!isActive_) {
            return;
        }
    }
    size_t index = tail_ % bufferSize_;
    if (index + writeSize < bufferSize_) {
        if (memcpy_s(buffer_.get() + index, writeSize, ptr, writeSize) != EOK) {
            MEDIA_LOG_E("memcpy_s failed when write buffer");
            writeSize = 0;
        }
        tail_ += writeSize;
        return;
    }
    if (memcpy_s(buffer_.get() + index, bufferSize_ - index, ptr, bufferSize_ - index) != EOK) {
        MEDIA_LOG_E("memcpy_s failed when write buffer");
        return;
    }
    if (writeSize - (bufferSize_ - index) > 0 && memcpy_s(buffer_.get(), writeSize - (bufferSize_ - index),
        ((uint8_t*)ptr) + bufferSize_ - index, writeSize - (bufferSize_ - index)) != EOK) {
        MEDIA_LOG_E("memcpy_s failed when write buffer");
        return;
    }
    tail_ += writeSize;
}
void RingBuffer::SetActive(bool active)
{
    OHOS::Media::OSAL::ScopedLock lck(writeMutex_);
    isActive_ = active;
    if (!active) {
        head_ = 0;
        tail_ = 0;
        writeCondition_.NotifyOne();
    }
}
void RingBuffer::Clear()
{
    OHOS::Media::OSAL::ScopedLock lck(writeMutex_);
    head_ = 0;
    tail_ = 0;
    writeCondition_.NotifyOne();
}
}
}
}