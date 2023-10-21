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

#include "av_shared_memory_ext.h"
#include <unistd.h>
#include "ashmem.h"
#include "av_shared_allocator.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "message_parcel.h"
#include "scope_guard.h"
#include "sys/mman.h"
#include "avbuffer_utils.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVSharedMemoryExt"};
constexpr uint8_t LOGD_FREQUENCY = 5;
} // namespace

namespace OHOS {
namespace MediaAVCodec {
std::shared_ptr<AVAllocator> AVAllocatorFactory::CreateSharedAllocator(MemoryFlag memFlag)
{
    auto allocator = std::shared_ptr<AVSharedAllocator>(new AVSharedAllocator());
    CHECK_AND_RETURN_RET_LOG(allocator != nullptr, nullptr, "Create AVSharedAllocator failed, no memory");
    allocator->memFlag_ = memFlag;
    return allocator;
}

AVSharedAllocator::AVSharedAllocator(){};

void *AVSharedAllocator::Alloc(int32_t capacity)
{
    int32_t fd = AshmemCreate(0, static_cast<size_t>(capacity)); // release by close(fd)
    CHECK_AND_RETURN_RET_LOG(fd > 0, nullptr, "fd is invalid, fd = %{public}d", fd);

    return reinterpret_cast<void *>(fd);
}

bool AVSharedAllocator::Free(void *ptr)
{
    int32_t fd = reinterpret_cast<intptr_t>(ptr);
    if (fd > 0) {
        (void)::close(fd);
        return true;
    }
    return false;
}

MemoryType AVSharedAllocator::GetMemoryType()
{
    return MemoryType::SHARED_MEMORY;
}

MemoryFlag AVSharedAllocator::GetMemFlag()
{
    return memFlag_;
}

AVSharedMemoryExt::AVSharedMemoryExt() : fd_(-1), isFirstFlag_(true), memFlag_(MemoryFlag::MEMORY_READ_ONLY) {}

AVSharedMemoryExt::~AVSharedMemoryExt()
{
    AVCODEC_LOGD_LIMIT(LOGD_FREQUENCY, "enter dtor, instance: 0x%{public}06" PRIXPTR ", name = %{public}s",
                       FAKE_POINTER(this), name_.c_str());
    Close();
    if (allocator_ == nullptr) {
        return;
    }
    bool ret = allocator_->Free(reinterpret_cast<void *>(fd_));
    CHECK_AND_RETURN_LOG(ret, "Free memory failed, instance: 0x%{public}06" PRIXPTR, FAKE_POINTER(this));
}

int32_t AVSharedMemoryExt::Init()
{
    memFlag_ = std::static_pointer_cast<AVSharedAllocator>(allocator_)->GetMemFlag();

    int32_t allocSize = align_ ? (capacity_ + align_ - 1) : capacity_;
    fd_ = reinterpret_cast<intptr_t>(allocator_->Alloc(allocSize));
    CHECK_AND_RETURN_RET_LOG(fd_ > 0, AVCS_ERR_NO_MEMORY, "Alloc AVSharedMemoryExt failed");

    uintptr_t addrBase = reinterpret_cast<uintptr_t>(base_);
    offset_ = static_cast<size_t>(AlignUp(addrBase, static_cast<uintptr_t>(offset_)) - addrBase);

    AVCODEC_LOGD_LIMIT(LOGD_FREQUENCY, "enter init, instance: 0x%{public}06" PRIXPTR ", name = %{public}s",
                       FAKE_POINTER(this), name_.c_str());
    return AVCS_ERR_OK;
}

int32_t AVSharedMemoryExt::Init(MessageParcel &parcel)
{
    fd_ = dup(parcel.ReadFileDescriptor());
    memFlag_ = static_cast<MemoryFlag>(parcel.ReadUint32());
    AVCODEC_LOGD_LIMIT(LOGD_FREQUENCY, "enter init, instance: 0x%{public}06" PRIXPTR ", name = %{public}s",
                       FAKE_POINTER(this), name_.c_str());
    return AVCS_ERR_OK;
}

bool AVSharedMemoryExt::WriteToMessageParcel(MessageParcel &parcel)
{
    MessageParcel bufferParcel;
    bool ret = bufferParcel.WriteFileDescriptor(fd_) &&
               bufferParcel.WriteUint32(static_cast<uint32_t>(memFlag_));
    if (ret) {
        parcel.Append(bufferParcel);
    }
    return ret;
}

uint8_t *AVSharedMemoryExt::GetAddr()
{
    if (isFirstFlag_) {
        int32_t ret = MapMemoryAddr();
        CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "MapMemory failed");
        isFirstFlag_ = false;
    }
    return base_;
}

MemoryType AVSharedMemoryExt::GetMemoryType()
{
    return MemoryType::SHARED_MEMORY;
}

int32_t AVSharedMemoryExt::GetFileDescriptor()
{
    return fd_;
}

MemoryFlag AVSharedMemoryExt::GetMemFlag()
{
    return memFlag_;
}

void AVSharedMemoryExt::Close() noexcept
{
    if (base_ != nullptr) {
        (void)::munmap(base_, static_cast<size_t>(capacity_));
        base_ = nullptr;
        size_ = 0;
    }
}

int32_t AVSharedMemoryExt::MapMemoryAddr()
{
    ON_SCOPE_EXIT(0)
    {
        AVCODEC_LOGE("create avsharedmemory failed, name = %{public}s, size = %{public}d, "
                     "flags = 0x%{public}x, fd = %{public}d",
                     name_.c_str(), capacity_, static_cast<int32_t>(memFlag_), fd_);
        Close();
        return AVCS_ERR_NO_MEMORY;
    };
    CHECK_AND_RETURN_RET_LOG(capacity_ > 0, AVCS_ERR_INVALID_VAL, "size is invalid, size = %{public}d", capacity_);
    unsigned int prot = PROT_READ | PROT_WRITE;
    if (memFlag_ == MemoryFlag::MEMORY_READ_ONLY) {
        prot &= ~PROT_WRITE;
    } else if (memFlag_ == MemoryFlag::MEMORY_WRITE_ONLY) {
        prot &= ~PROT_READ;
    }
    int result = AshmemSetProt(fd_, static_cast<int>(prot));
    CHECK_AND_RETURN_RET_LOG(result >= 0, AVCS_ERR_INVALID_OPERATION, "AshmemSetProt failed, result = %{public}d",
                             result);

    void *addr = ::mmap(nullptr, static_cast<size_t>(capacity_), static_cast<int>(prot), MAP_SHARED, fd_, 0);
    CHECK_AND_RETURN_RET_LOG(addr != MAP_FAILED, AVCS_ERR_INVALID_OPERATION, "mmap failed, please check params");

    base_ = static_cast<uint8_t *>(addr);
    CANCEL_SCOPE_EXIT_GUARD(0);
    return AVCS_ERR_OK;
}
} // namespace MediaAVCodec
} // namespace OHOS
