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
#include <dirent.h>
#include <unistd.h>
#include "ashmem.h"
#include "av_hardware_allocator.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "message_parcel.h"
#include "scope_guard.h"
#include "sys/mman.h"
// #include <unordered_map>

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVHardWareMemory"};
constexpr uint8_t LOGD_FREQUENCY = 5;
// const std::unordered_map<OHOS::MediaAVCodec::MemoryFlag, DmabufHeapBufferSyncType> MEMORY_FLAG_MAP = {
//     {OHOS::MediaAVCodec::MemoryFlag::FLAGS_WRITE_ONLY, DmabufHeapBufferSyncType::DMA_BUF_HEAP_BUF_SYNC_WRITE},
//     {OHOS::MediaAVCodec::MemoryFlag::FLAGS_READ_ONLY, DmabufHeapBufferSyncType::DMA_BUF_HEAP_BUF_SYNC_READ},
//     {OHOS::MediaAVCodec::MemoryFlag::FLAGS_READ_WRITE, DmabufHeapBufferSyncType::DMA_BUF_HEAP_BUF_SYNC_RW}};
} // namespace

namespace OHOS {
namespace MediaAVCodec {
HardwareHeapFactory::HardwareHeapFactory()
{
    std::string rootDir = "/dev/dma_heap/";
    DIR *dir = opendir(rootDir.c_str());
    if (dir == nullptr) {
        return;
    }
    struct dirent *ptr;
    std::string heapName = "";
    while ((ptr = readdir(dir)) != nullptr) {
        std::string fileName = ptr->d_name;
        std::string::size_type idx = fileName.find("system");
        if (idx != std::string::npos) {
            heapName = fileName;
            break;
        }
    }
    closedir(dir);
    dmaHeapFd_ = DmabufHeapOpen(heapName.c_str());
}

HardwareHeapFactory::~HardwareHeapFactory()
{
    DmabufHeapClose(dmaHeapFd_);
}

HardwareHeapFactory &HardwareHeapFactory::GetInstance()
{
    static HardwareHeapFactory hwHeapInstance_;
    return hwHeapInstance_;
}

int32_t HardwareHeapFactory::GetHardwareHeapFd()
{
    return dmaHeapFd_;
}

std::shared_ptr<AVAllocator> AVAllocatorFactory::CreateHardwareAllocator(int32_t fd, int32_t capacity, uint32_t memFlag)
{
    DmabufHeapBufferSyncType tempFlag = static_cast<DmabufHeapBufferSyncType>(memFlag);
    bool isFlagValid = (tempFlag == DMA_BUF_HEAP_BUF_SYNC_RW) || (tempFlag == DMA_BUF_HEAP_BUF_SYNC_READ) ||
                       (tempFlag == DMA_BUF_HEAP_BUF_SYNC_WRITE);
    CHECK_AND_RETURN_RET_LOG(isFlagValid, nullptr, "memFlag is invalid, memFlag = %{public}d", memFlag);
    auto allocator = std::shared_ptr<AVHardwareAllocator>(new AVHardwareAllocator());
    allocator->fd_ = fd;
    allocator->capacity_ = capacity;
    allocator->memFlag_ = static_cast<DmabufHeapBufferSyncType>(memFlag);
    return allocator;
}

AVHardwareAllocator::AVHardwareAllocator() : fd_(-1), capacity_(-1), allocBase_(nullptr){};
// void *AVHardwareAllocator::Alloc(int32_t capacity)
// {
//     (void)capacity;
//     DmabufHeapBuffer dmaBuffer = {.fd = fd_, .size = capacity_, .heapFlags = 0};
//     isRemote_ = true;
//     if (fd_ <= 0) {
//         isRemote_ = false;
//         dmaBuffer = {.size = capacity_, .heapFlags = 0};
//         int32_t dmaHeapFd = HardwareHeapFactory::GetInstance().GetHardwareHeapFd();
//         DmabufHeapBufferAlloc(dmaHeapFd, &dmaBuffer);
//         CHECK_AND_RETURN_RET_LOG(dmaHeapFd >= 0, nullptr, "dma heap fd error, dmaHeapFd_ = %{public}d", dmaHeapFd);
//     }
//     fd_ = dmaBuffer.fd;
//     return reinterpret_cast<void *>(fd_);
// }

void *AVHardwareAllocator::Alloc(int32_t capacity)
{
    (void)capacity;
    int32_t ret = MapMemoryAddr();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "Map dma buffer failed");

    return reinterpret_cast<void *>(allocBase_);
}

bool AVHardwareAllocator::Free(void *ptr)
{
    CHECK_AND_RETURN_RET_LOG(static_cast<uint8_t *>(ptr) == allocBase_, false, "Mapped buffer not match");
    if (allocBase_ != nullptr) {
        (void)::munmap(allocBase_, static_cast<size_t>(capacity_));
    }
    allocBase_ = nullptr;
    fd_ = -1;
    capacity_ = -1;
    return true;
}

MemoryType AVHardwareAllocator::GetMemoryType()
{
    return MemoryType::HARDWARE_MEMORY;
}

DmabufHeapBufferSyncType AVHardwareAllocator::GetMemFlag()
{
    return memFlag_;
}

int32_t AVHardwareAllocator::GetFileDescriptor()
{
    return fd_;
}

int32_t AVHardwareAllocator::MapMemoryAddr()
{
    ON_SCOPE_EXIT(0)
    {
        AVCODEC_LOGE("MapMemoryAddr failed, size = %{public}d, "
                     "flags = 0x%{public}x, fd = %{public}d",
                     capacity_, static_cast<int32_t>(memFlag_), fd_);
        if (allocBase_ != nullptr) {
            (void)::munmap(allocBase_, static_cast<size_t>(capacity_));
            allocBase_ = nullptr;
        }
        return AVCS_ERR_NO_MEMORY;
    };
    CHECK_AND_RETURN_RET_LOG(capacity_ > 0, AVCS_ERR_INVALID_VAL, "capacity is invalid, capacity = %{public}d",
                             capacity_);
    unsigned int prot = PROT_READ | PROT_WRITE;
    CHECK_AND_RETURN_RET_LOG(fd_ > 0, AVCS_ERR_INVALID_OPERATION, "fd is invalid, fd = %{public}d", fd_);
    if (memFlag_ == DMA_BUF_HEAP_BUF_SYNC_READ) {
        prot &= ~PROT_WRITE;
    } else if (memFlag_ == DMA_BUF_HEAP_BUF_SYNC_WRITE) {
        prot &= ~PROT_READ;
    }
    void *addr = ::mmap(nullptr, static_cast<size_t>(capacity_), static_cast<int>(prot), MAP_SHARED, fd_, 0);
    CHECK_AND_RETURN_RET_LOG(addr != MAP_FAILED, AVCS_ERR_INVALID_OPERATION, "mmap failed, please check params");
    allocBase_ = reinterpret_cast<uint8_t *>(addr);
    CANCEL_SCOPE_EXIT_GUARD(0);
    return AVCS_ERR_OK;
}

AVHardwareMemory::AVHardwareMemory() : isStartSync_(false) {}

AVHardwareMemory::~AVHardwareMemory()
{
    AVCODEC_LOGD_LIMIT(LOGD_FREQUENCY, "enter dtor, instance: 0x%{public}06" PRIXPTR ", name = %{public}s",
                       FAKE_POINTER(this), name_.c_str());
    if (allocator_ == nullptr) {
        if (base_ != nullptr) {
            (void)::munmap(base_, static_cast<size_t>(capacity_));
        }
        return;
    }
    allocator_->Free(base_);
}

int32_t AVHardwareMemory::Init()
{
    fd_ = std::static_pointer_cast<AVHardwareAllocator>(allocator_)->GetFileDescriptor();
    memFlag_ = std::static_pointer_cast<AVHardwareAllocator>(allocator_)->GetMemFlag();
    base_ = static_cast<uint8_t *>(allocator_->Alloc(0));

    CHECK_AND_RETURN_RET_LOG(base_ != nullptr, AVCS_ERR_NO_MEMORY, "dma memory alloc failed");
    AVCODEC_LOGD_LIMIT(LOGD_FREQUENCY, "enter init, instance: 0x%{public}06" PRIXPTR ", name = %{public}s",
                       FAKE_POINTER(this), name_.c_str());
    return AVCS_ERR_OK;
}

int32_t AVHardwareMemory::Init(MessageParcel &parcel)
{
    fd_ = dup(parcel.ReadFileDescriptor());
    memFlag_ = static_cast<DmabufHeapBufferSyncType>(parcel.ReadUint32());

    allocator_ = AVAllocatorFactory::CreateHardwareAllocator(fd_, capacity_, memFlag_);
    CHECK_AND_RETURN_RET_LOG(allocator_ != nullptr, AVCS_ERR_NO_MEMORY, "allocator_ is nullptr");

    base_ = static_cast<uint8_t *>(allocator_->Alloc(0));
    allocator_ = nullptr;

    CHECK_AND_RETURN_RET_LOG(base_ != nullptr, AVCS_ERR_NO_MEMORY, "dma memory alloc failed");
    AVCODEC_LOGD_LIMIT(LOGD_FREQUENCY, "enter init, instance: 0x%{public}06" PRIXPTR ", name = %{public}s",
                       FAKE_POINTER(this), name_.c_str());
    return AVCS_ERR_OK;
}

bool AVHardwareMemory::WriteToMessageParcel(MessageParcel &parcel)
{
    MessageParcel bufferParcel;
    bool ret = bufferParcel.WriteFileDescriptor(fd_) && bufferParcel.WriteUint32(static_cast<uint32_t>(memFlag_));
    if (ret) {
        parcel.Append(bufferParcel);
    }
    return ret;
}

MemoryType AVHardwareMemory::GetMemoryType()
{
    return MemoryType::HARDWARE_MEMORY;
}

int32_t AVHardwareMemory::GetFileDescriptor()
{
    return fd_;
}

int32_t AVHardwareMemory::SyncStart()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(!isStartSync_, -1, "Not ready to start syncing yet!");
    isStartSync_ = true;
    return DmabufHeapBufferSyncStart(fd_, memFlag_);
}

int32_t AVHardwareMemory::SyncEnd()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(isStartSync_, -1, "Haven't started syncing yet!");
    isStartSync_ = false;
    return DmabufHeapBufferSyncEnd(fd_, memFlag_);
}
} // namespace MediaAVCodec
} // namespace OHOS
