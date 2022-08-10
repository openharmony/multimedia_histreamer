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

#if !defined(OHOS_LITE) && defined(VIDEO_SUPPORT)

#include "share_memory.h"
#include "foundation/log.h"

namespace OHOS {
namespace Media {
namespace Plugin {
ShareMemory::ShareMemory(size_t capacity, std::shared_ptr<Allocator> allocator, size_t align)
    : Memory(capacity, std::move(allocator), align, MemoryType::SHARE_MEMORY, false)
{
    size_t allocSize = align ? (capacity + align - 1) : capacity;
    if (this->allocator != nullptr && this->allocator->GetMemoryType() == MemoryType::SHARE_MEMORY) {
        shareAllocator_ = ReinterpretPointerCast<ShareAllocator>(this->allocator);
        fd_ = (uintptr_t)shareAllocator_->Alloc(allocSize);
        sharedMem_ = std::make_shared<Ashmem>(fd_, allocSize);
        InitShareMemory(shareAllocator_->GetShareMemType());
    } else {
        MEDIA_LOG_E("create sharedMem_ failed");
    }
}

ShareMemory::~ShareMemory()
{
    if (sharedMem_) {
        sharedMem_->UnmapAshmem();
        sharedMem_->CloseAshmem();
        sharedMem_ = nullptr;
    }
}

size_t ShareMemory::Write(const uint8_t* in, size_t writeSize, size_t position)
{
    size_t start = 0;
    if (position == INVALID_POSITION) {
        start = size;
    } else {
        start = std::min(position, capacity);
    }
    size_t length = std::min(writeSize, capacity - start);
    if (!sharedMem_->WriteToAshmem(in, (int32_t)writeSize, (int32_t)start)) {
        MEDIA_LOG_E("sharedMem_ WriteToAshmem failed");
        return 0;
    }
    size = start + length;
    return length;
}

size_t ShareMemory::Read(uint8_t* out, size_t readSize, size_t position)
{
    size_t start = 0;
    size_t maxLength = size;
    if (position != INVALID_POSITION) {
        start = std::min(position, size);
        maxLength = size - start;
    }
    size_t length = std::min(readSize, maxLength);
    if (memcpy_s(out, length, sharedMem_->ReadFromAshmem((int32_t)readSize, (int32_t)start), length) != EOK) {
        return 0;
    }
    return length;
}

int ShareMemory::GetShareMemoryFd()
{
    return fd_;
}

void ShareMemory::InitShareMemory(ShareMemType type)
{
    switch (type) {
        case ShareMemType::READ_ONLY_TYPE :
            if (!sharedMem_->MapReadOnlyAshmem()) {
                MEDIA_LOG_E("failed to exec MapReadOnlyAshmem");
            }
            break;
        case ShareMemType::READ_WRITE_TYPE :
            if (!sharedMem_->MapReadAndWriteAshmem()) {
                MEDIA_LOG_E("failed to exec MapReadAndWriteAshmem");
            }
            break;
        default:
            MEDIA_LOG_E("set share memory type failed, not find this type: " PUBLIC_LOG_D32,
                static_cast<int32_t>(type));
            break;
    }
}
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif

