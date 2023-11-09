/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "av_virtual_memory.h"
#include "av_virtual_allocator.h"
#include "avbuffer_utils.h"
#include "buffer/avallocator.h"
#include "common/log.h"
#include "common/status.h"
#include "message_parcel.h"

namespace OHOS {
namespace Media {
std::shared_ptr<AVAllocator> AVAllocatorFactory::CreateVirtualAllocator()
{
    auto allocator = std::shared_ptr<AVVirtualAllocator>(new AVVirtualAllocator());
    FALSE_RETURN_V_MSG_E(allocator != nullptr, nullptr, "Create AVVirtualAllocator failed, no memory");
    return allocator;
}

AVVirtualAllocator::AVVirtualAllocator(){};

void *AVVirtualAllocator::Alloc(int32_t capacity)
{
    uint8_t *ptr = new uint8_t[capacity];
    FALSE_RETURN_V_MSG_E(ptr != nullptr, nullptr, "Alloc memory failed, no memory");

    return static_cast<void *>(ptr);
}

bool AVVirtualAllocator::Free(void *ptr)
{
    uint8_t *dataPtr = static_cast<uint8_t *>(ptr);
    FALSE_RETURN_V_MSG_E(dataPtr != nullptr, false, "Invalid ptr, ptr = 0x%{public}06" PRIXPTR, FAKE_POINTER(dataPtr));
    delete dataPtr;
    return true;
}

MemoryType AVVirtualAllocator::GetMemoryType()
{
    return MemoryType::VIRTUAL_MEMORY;
}

AVVirtualMemory::AVVirtualMemory() {}

AVVirtualMemory::~AVVirtualMemory()
{
    MEDIA_LOG_DD("enter dtor, instance: 0x%{public}06" PRIXPTR, FAKE_POINTER(this));
    if (allocator_ == nullptr) {
        return;
    }
    bool ret = allocator_->Free(static_cast<void *>(base_));
    FALSE_RETURN_MSG(ret, "Free memory failed, instance: 0x%{public}06" PRIXPTR, FAKE_POINTER(this));
    base_ = nullptr;
}

int32_t AVVirtualMemory::Init()
{
    int32_t allocSize = align_ ? (capacity_ + align_ - 1) : capacity_;
    base_ = static_cast<uint8_t *>(allocator_->Alloc(allocSize));
    FALSE_RETURN_V_MSG_E(base_ != nullptr, static_cast<int32_t>(Status::ERROR_NO_MEMORY),
                         "Alloc AVVirtualMemory failed");

    uintptr_t addrBase = reinterpret_cast<uintptr_t>(base_);
    offset_ = static_cast<size_t>(AlignUp(addrBase, static_cast<uintptr_t>(offset_)) - addrBase);

    MEDIA_LOG_DD("enter init, instance: 0x%{public}06" PRIXPTR, FAKE_POINTER(this));
    return static_cast<int32_t>(Status::OK);
}

MemoryType AVVirtualMemory::GetMemoryType()
{
    return MemoryType::VIRTUAL_MEMORY;
}
} // namespace Media
} // namespace OHOS