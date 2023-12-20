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

#include "buffer/avsharedmemorybase.h"
#include <sys/mman.h>
#include <unistd.h>
#include "ashmem.h"
#include "common/status.h"
#include "common/log.h"
#include "scope_guard.h"
#include "securec.h"

namespace OHOS {
namespace Media {
struct AVSharedMemoryBaseImpl : public AVSharedMemoryBase {
public:
    AVSharedMemoryBaseImpl(int32_t fd, int32_t size, uint32_t flags, const std::string &name)
        : AVSharedMemoryBase(fd, size, flags, name) {}
};

std::shared_ptr<AVSharedMemory> AVSharedMemoryBase::CreateFromLocal(
    int32_t size, uint32_t flags, const std::string &name)
{
    std::shared_ptr<AVSharedMemoryBase> memory = std::make_shared<AVSharedMemoryBase>(size, flags, name);
    int32_t ret = memory->Init();
    if (ret != static_cast<int32_t>(Status::OK)) {
        MEDIA_LOG_E("Create avsharedmemory failed, ret = %{public}d", ret);
        return nullptr;
    }

    return memory;
}

std::shared_ptr<AVSharedMemory> AVSharedMemoryBase::CreateFromRemote(
    int32_t fd, int32_t size, uint32_t flags, const std::string &name)
{
    std::shared_ptr<AVSharedMemoryBase> memory = std::make_shared<AVSharedMemoryBaseImpl>(fd, size, flags, name);
    int32_t ret = memory->Init();
    if (ret != static_cast<int32_t>(Status::OK)) {
        MEDIA_LOG_E("Create avsharedmemory failed, ret = %{public}d", ret);
        return nullptr;
    }

    return memory;
}

AVSharedMemoryBase::AVSharedMemoryBase(int32_t size, uint32_t flags, const std::string &name)
    : base_(nullptr), capacity_(size), flags_(flags), name_(name), fd_(-1), size_(0)
{
    MEDIA_LOG_DD(LOGD_FREQUENCY, "enter ctor, instance: 0x%{public}06" PRIXPTR ", name = %{public}s",
               FAKE_POINTER(this), name_.c_str());
}

AVSharedMemoryBase::AVSharedMemoryBase(int32_t fd, int32_t size, uint32_t flags, const std::string &name)
    : base_(nullptr), capacity_(size), flags_(flags), name_(name), fd_(dup(fd)), size_(0)
{
    MEDIA_LOG_DD(LOGD_FREQUENCY, "enter ctor, instance: 0x%{public}06" PRIXPTR ", name = %{public}s",
               FAKE_POINTER(this), name_.c_str());
}

AVSharedMemoryBase::~AVSharedMemoryBase()
{
    MEDIA_LOG_DD(LOGD_FREQUENCY, "enter dtor, instance: 0x%{public}06" PRIXPTR ", name = %{public}s",
               FAKE_POINTER(this), name_.c_str());
    Close();
}

int32_t AVSharedMemoryBase::Init(bool isMapVirAddr)
{
    ON_SCOPE_EXIT(0) {
        MEDIA_LOG_E("create avsharedmemory failed, name = %{public}s, size = %{public}d, "
                    "flags = 0x%{public}x, fd = %{public}d",
                    name_.c_str(), capacity_, flags_, fd_);
        Close();
    };

    FALSE_RETURN_V_MSG_E(capacity_ > 0, static_cast<int32_t>(Status::ERROR_INVALID_PARAMETER),
                         "size is invalid, size = %{public}d", capacity_);

    bool isRemote = false;
    if (fd_ > 0) {
        int size = AshmemGetSize(fd_);
        FALSE_RETURN_V_MSG_E(size == capacity_, static_cast<int32_t>(Status::ERROR_INVALID_PARAMETER),
            "size not equal capacity_, size = %{public}d, capacity_ = %{public}d", size, capacity_);
        isRemote = true;
    } else {
        fd_ = AshmemCreate(name_.c_str(), static_cast<size_t>(capacity_));
        FALSE_RETURN_V_MSG_E(fd_ > 0, static_cast<int32_t>(Status::ERROR_INVALID_PARAMETER),
                             "fd is invalid, fd = %{public}d", fd_);
    }
    if (isMapVirAddr) {
        int32_t ret = MapMemory(isRemote);
        FALSE_RETURN_V_MSG_E(ret == static_cast<int32_t>(Status::OK),
                             static_cast<int32_t>(Status::ERROR_INVALID_PARAMETER),
                             "MapMemory failed, ret = %{plublic}d", ret);
    }
    CANCEL_SCOPE_EXIT_GUARD(0);
    return static_cast<int32_t>(Status::OK);
}

int32_t AVSharedMemoryBase::MapMemory(bool isRemote)
{
#ifdef MEDIA_OHOS
    unsigned int prot = PROT_READ | PROT_WRITE;
    if (isRemote && (flags_ & FLAGS_READ_ONLY)) {
        prot &= ~PROT_WRITE;
    }

    int result = AshmemSetProt(fd_, static_cast<int>(prot));
    FALSE_RETURN_V_MSG_E(result >= 0, static_cast<int32_t>(Status::ERROR_INVALID_OPERATION),
        "AshmemSetProt failed, result = %{public}d", result);

    void *addr = ::mmap(nullptr, static_cast<size_t>(capacity_), static_cast<int>(prot), MAP_SHARED, fd_, 0);
    FALSE_RETURN_V_MSG_E(addr != MAP_FAILED, static_cast<int32_t>(Status::ERROR_INVALID_OPERATION),
                         "mmap failed, please check params");

    base_ = reinterpret_cast<uint8_t*>(addr);
#endif
    return static_cast<int32_t>(Status::OK);
}

void AVSharedMemoryBase::Close() noexcept
{
#ifdef MEDIA_OHOS
    if (base_ != nullptr) {
        (void)::munmap(base_, static_cast<size_t>(capacity_));
        base_ = nullptr;
        capacity_ = 0;
        flags_ = 0;
        size_ = 0;
    }
#endif
    if (fd_ > 0) {
        (void)::close(fd_);
        fd_ = -1;
    }
}

int32_t AVSharedMemoryBase::Write(const uint8_t *in, int32_t writeSize, int32_t position)
{
    FALSE_RETURN_V_MSG_E(in != nullptr, 0, "Input buffer is nullptr");
    FALSE_RETURN_V_MSG_E(writeSize > 0, 0, "Input writeSize:%{public}d is invalid", writeSize);
    int32_t start = 0;
    if (position == INVALID_POSITION) {
        start = size_;
    } else {
        start = std::min(position, capacity_);
    }
    int32_t unusedSize = capacity_ - start;
    int32_t length = std::min(writeSize, unusedSize);
    MEDIA_LOG_DD("write data,length:%{public}d, start:%{public}d, name:%{public}s", length, start, name_.c_str());
    FALSE_RETURN_V_MSG_E((length + start) <= capacity_, 0, "Write out of bounds, length:%{public}d, "
                            "start:%{public}d, capacity_:%{public}d", length, start, capacity_);
    uint8_t *dstPtr = base_ + start;
    FALSE_RETURN_V_MSG_E(dstPtr != nullptr, 0, "Inner dstPtr is nullptr");

    auto error = memcpy_s(dstPtr, length, in, length);
    FALSE_RETURN_V_MSG_E(error == EOK, 0, "Inner memcpy_s failed,name:%{public}s, %{public}s",
        name_.c_str(), strerror(error));
    size_ = start + length;
    return length;
}

int32_t AVSharedMemoryBase::Read(uint8_t *out, int32_t readSize, int32_t position)
{
    FALSE_RETURN_V_MSG_E(out != nullptr, 0, "Input buffer is nullptr");
    FALSE_RETURN_V_MSG_E(readSize > 0, 0, "Input readSize:%{public}d is invalid", readSize);
    int32_t start = 0;
    int32_t maxLength = size_;
    if (position != INVALID_POSITION) {
        start = std::min(position, size_);
        maxLength = size_ - start;
    }
    int32_t length = std::min(readSize, maxLength);
    FALSE_RETURN_V_MSG_E((length + start) <= capacity_, 0, "Read out of bounds, length:%{public}d, "
                            "start:%{public}d, capacity_:%{public}d", length, start, capacity_);
    uint8_t *srcPtr = base_ + start;
    FALSE_RETURN_V_MSG_E(srcPtr != nullptr, 0, "Inner srcPtr is nullptr");
    auto error = memcpy_s(out, length, srcPtr, length);
    FALSE_RETURN_V_MSG_E(error == EOK, 0, "Inner memcpy_s failed,name:%{public}s, %{public}s",
        name_.c_str(), strerror(error));
    return length;
}

void AVSharedMemoryBase::ClearUsedSize()
{
    size_ = 0;
}

int32_t AVSharedMemoryBase::GetUsedSize() const
{
    return size_;
}
} // namespace Media
} // namespace OHOS
