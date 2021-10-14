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

#ifndef HISTREAMER_RING_BUFFER_H
#define HISTREAMER_RING_BUFFER_H

#include <atomic>
#include <condition_variable>
#include <memory>
#include "utils/memory_helper.h"

namespace OHOS {
namespace Media {
namespace Plugin {
class RingBuffer {
public:
    explicit RingBuffer(size_t bufferSize) : bufferSize_(bufferSize)
    {
    }

    ~RingBuffer() = default;

    bool Init()
    {
        buffer_ = MemoryHelper::make_unique<uint8_t[]>(bufferSize_);
        return buffer_ != nullptr;
    }

    size_t ReadBuffer(void* ptr, size_t readSize)
    {
        std::unique_lock<std::mutex> lck(writeMutex_);
        if (!isActive_) {
            return 0;
        }
        auto available = tail_ - head_;
        available = (available > readSize) ? readSize : available;
        size_t index = head_ % bufferSize_;
        if (index + available < bufferSize_) {
            (void)memcpy_s(ptr, available, buffer_.get() + index, available);
        } else {
            (void)memcpy_s(ptr, bufferSize_ - index, buffer_.get() + index, bufferSize_ - index);
            (void)memcpy_s(((uint8_t*)ptr) + (bufferSize_ - index), available - (bufferSize_ - index), buffer_.get(),
                           available - (bufferSize_ - index));
        }
        head_ += available;
        writeCondition_.notify_one();
        return available;
    }

    void WriteBuffer(void* ptr, size_t writeSize)
    {
        std::unique_lock<std::mutex> lck(writeMutex_);
        if (!isActive_) {
            return;
        }
        while (writeSize + tail_ > head_ + bufferSize_) {
            writeCondition_.wait(lck);
            if (!isActive_) {
                return;
            }
        }
        size_t index = tail_ % bufferSize_;
        if (index + writeSize < bufferSize_) {
            (void)memcpy_s(buffer_.get() + index, writeSize, ptr, writeSize);
        } else {
            (void)memcpy_s(buffer_.get() + index, bufferSize_ - index, ptr, bufferSize_ - index);
            (void)memcpy_s(buffer_.get(), writeSize - (bufferSize_ - index), ((uint8_t*)ptr) + bufferSize_ - index,
                           writeSize - (bufferSize_ - index));
        }
        tail_ += writeSize;
    }

    void SetActive(bool active)
    {
        std::unique_lock<std::mutex> lck(writeMutex_);
        isActive_ = active;
        if (!active) {
            head_ = 0;
            tail_ = 0;
            writeCondition_.notify_one();
        }
    }

private:
    const size_t bufferSize_;
    std::unique_ptr<uint8_t[]> buffer_;
    size_t head_ {0}; // head
    size_t tail_ {0}; // tail
    std::mutex writeMutex_ {};
    std::condition_variable writeCondition_ {};
    bool isActive_ {true};
};
} // namespace Plugin
} // namespace Media
} // namespace OHOS

#endif // MEDIA_PIPELINE_RING_BUFFER_H
