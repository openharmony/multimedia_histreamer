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

#include "av_hardware_memory.h"
#include <unistd.h>
#include <unordered_map>
#include "ashmem.h"
#include "av_hardware_allocator.h"
#include "common/log.h"
#include "common/status.h"
#include "message_parcel.h"
#include "scope_guard.h"

#ifdef MEDIA_OHOS
#include "sys/mman.h"
#endif

namespace OHOS {
namespace Media {
std::shared_ptr<AVAllocator> AVAllocatorFactory::CreateHardwareAllocator(int32_t fd, int32_t capacity,
                                                                         MemoryFlag memFlag)
{
    FALSE_RETURN_V_MSG_E(fd > 0, nullptr, "File descriptor is invalid");
    auto allocator = std::shared_ptr<AVHardwareAllocator>(new AVHardwareAllocator());
    allocator->fd_ = dup(fd);
    allocator->capacity_ = capacity;
    allocator->memFlag_ = memFlag;
    return allocator;
}

AVHardwareAllocator::AVHardwareAllocator()
    : fd_(-1), capacity_(-1), allocBase_(nullptr), memFlag_(MemoryFlag::MEMORY_READ_ONLY){};

void *AVHardwareAllocator::Alloc(int32_t capacity)
{
    (void)capacity;
    Status ret = MapMemoryAddr();
    FALSE_RETURN_V_MSG_E(ret == Status::OK, nullptr, "Map dma buffer failed");

    return reinterpret_cast<void *>(allocBase_);
}

bool AVHardwareAllocator::Free(void *ptr)
{
#ifdef MEDIA_OHOS
    FALSE_RETURN_V_MSG_E(static_cast<uint8_t *>(ptr) == allocBase_, false, "Mapped buffer not match");
    if (allocBase_ != nullptr) {
        (void)::munmap(allocBase_, static_cast<size_t>(capacity_));
    }
    if (fd_ > 0) {
        (void)::close(fd_);
        fd_ = -1;
    }
    allocBase_ = nullptr;
    capacity_ = -1;
#endif
    return true;
}

MemoryType AVHardwareAllocator::GetMemoryType()
{
    return MemoryType::HARDWARE_MEMORY;
}

MemoryFlag AVHardwareAllocator::GetMemFlag()
{
    return memFlag_;
}

int32_t AVHardwareAllocator::GetFileDescriptor()
{
    return fd_;
}

Status AVHardwareAllocator::MapMemoryAddr()
{
#ifdef MEDIA_OHOS
    ON_SCOPE_EXIT(0)
    {
        MEDIA_LOG_E("MapMemoryAddr failed, size = " PUBLIC_LOG_D32 ", "
                    "flags = 0x%{public}x, fd = " PUBLIC_LOG_D32,
                    capacity_, memFlag_, fd_);
        if (allocBase_ != nullptr) {
            (void)::munmap(allocBase_, static_cast<size_t>(capacity_));
            allocBase_ = nullptr;
        }
        return Status::ERROR_NO_MEMORY;
    };
    FALSE_RETURN_V_MSG_E(capacity_ > 0, Status::ERROR_INVALID_DATA, "capacity is invalid, capacity = " PUBLIC_LOG_D32,
                         capacity_);
    unsigned int prot = PROT_READ | PROT_WRITE;
    FALSE_RETURN_V_MSG_E(fd_ > 0, Status::ERROR_INVALID_OPERATION, "fd is invalid, fd = " PUBLIC_LOG_D32, fd_);
    if (memFlag_ == MemoryFlag::MEMORY_READ_ONLY) {
        prot &= ~PROT_WRITE;
    } else if (memFlag_ == MemoryFlag::MEMORY_WRITE_ONLY) {
        prot &= ~PROT_READ;
    }
    void *addr = ::mmap(nullptr, static_cast<size_t>(capacity_), static_cast<int>(prot), MAP_SHARED, fd_, 0);
    FALSE_RETURN_V_MSG_E(addr != MAP_FAILED, Status::ERROR_INVALID_OPERATION, "mmap failed, please check params");
    allocBase_ = reinterpret_cast<uint8_t *>(addr);
    CANCEL_SCOPE_EXIT_GUARD(0);
#endif
    return Status::OK;
}

AVHardwareMemory::AVHardwareMemory() : isStartSync_(false), memFlag_(MemoryFlag::MEMORY_READ_ONLY) {}

AVHardwareMemory::~AVHardwareMemory()
{
#ifdef MEDIA_OHOS
    MEDIA_LOG_DD("enter dtor, instance: 0x%{public}06" PRIXPTR ", name = %{public}s", FAKE_POINTER(this),
                 name_.c_str());
    if (allocator_ == nullptr) {
        if (base_ != nullptr) {
            (void)::munmap(base_, static_cast<size_t>(capacity_));
        }
        if (fd_ > 0) {
            (void)::close(fd_);
            fd_ = -1;
        }
        return;
    }
#endif
    allocator_->Free(base_);
}

Status AVHardwareMemory::Init()
{
    fd_ = std::static_pointer_cast<AVHardwareAllocator>(allocator_)->GetFileDescriptor();
    memFlag_ = std::static_pointer_cast<AVHardwareAllocator>(allocator_)->GetMemFlag();
    base_ = static_cast<uint8_t *>(allocator_->Alloc(0));

    FALSE_RETURN_V_MSG_E(base_ != nullptr, Status::ERROR_NO_MEMORY, "dma memory alloc failed");
    MEDIA_LOG_DD("enter init, instance: 0x%{public}06" PRIXPTR ", name = %{public}s", FAKE_POINTER(this),
                 name_.c_str());
    return Status::OK;
}

Status AVHardwareMemory::Init(MessageParcel &parcel)
{
#ifdef MEDIA_OHOS
    int32_t fd = parcel.ReadFileDescriptor();
    FALSE_RETURN_V_MSG_E(fd > 0, Status::ERROR_INVALID_DATA, "File descriptor is invalid");

    memFlag_ = static_cast<MemoryFlag>(parcel.ReadUint32());

    allocator_ = AVAllocatorFactory::CreateHardwareAllocator(fd, capacity_, memFlag_);
    if (allocator_ == nullptr) {
        MEDIA_LOG_E("allocator is nullptr");
        (void)::close(fd);
        return Status::ERROR_NO_MEMORY;
    }
    fd_ = std::static_pointer_cast<AVHardwareAllocator>(allocator_)->GetFileDescriptor();
    (void)::close(fd);

    base_ = static_cast<uint8_t *>(allocator_->Alloc(0));
    allocator_ = nullptr;

    FALSE_RETURN_V_MSG_E(base_ != nullptr, Status::ERROR_NO_MEMORY, "dma memory alloc failed");
    MEDIA_LOG_DD("enter init, instance: 0x%{public}06" PRIXPTR ", name = %{public}s", FAKE_POINTER(this),
                 name_.c_str());
#endif
    return Status::OK;
}

bool AVHardwareMemory::WriteToMessageParcel(MessageParcel &parcel)
{
    bool ret = true;
#ifdef MEDIA_OHOS
    MessageParcel bufferParcel;
    ret = bufferParcel.WriteFileDescriptor(fd_) && bufferParcel.WriteUint32(static_cast<uint32_t>(memFlag_));
    if (ret) {
        parcel.Append(bufferParcel);
    }
#endif
    return ret;
}

bool AVHardwareMemory::ReadFromMessageParcel(MessageParcel &parcel)
{
#ifdef MEDIA_OHOS
    int32_t fd = parcel.ReadFileDescriptor();
    (void)parcel.ReadUint32();
    if (fd > 0) {
        (void)::close(fd);
    }
#endif
    return true;
}

MemoryType AVHardwareMemory::GetMemoryType()
{
    return MemoryType::HARDWARE_MEMORY;
}

MemoryFlag AVHardwareMemory::GetMemoryFlag()
{
    return memFlag_;
}

int32_t AVHardwareMemory::GetFileDescriptor()
{
    return fd_;
}
} // namespace Media
} // namespace OHOS
