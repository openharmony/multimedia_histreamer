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
#include "av_shared_memory_ext.h"
#include "av_surface_memory.h"
#include "av_virtual_memory.h"
#include "buffer/avallocator.h"
#include "buffer/avbuffer.h"
#include "common/log.h"
#include "common/status.h"
#include "message_parcel.h"
#include "securec.h"
#include "surface_buffer.h"

namespace {
const std::string INVALID_MEMORY_NAME = "";
} // namespace

namespace OHOS {
namespace Media {
std::shared_ptr<AVMemory> AVMemory::CreateAVMemory(const std::string &name, std::shared_ptr<AVAllocator> allocator,
                                                   int32_t capacity, int32_t align)
{
    MemoryType type = allocator->GetMemoryType();
    std::shared_ptr<AVMemory> mem = nullptr;
    switch (type) {
        case MemoryType::VIRTUAL_MEMORY: {
            mem = std::shared_ptr<AVMemory>(new AVVirtualMemory());
            break;
        }
        case MemoryType::SURFACE_MEMORY: {
            mem = std::shared_ptr<AVMemory>(new AVSurfaceMemory());
            break;
        }
        case MemoryType::SHARED_MEMORY: {
            mem = std::shared_ptr<AVMemory>(new AVSharedMemoryExt());
            break;
        }
        case MemoryType::HARDWARE_MEMORY: {
            mem = std::shared_ptr<AVMemory>(new AVHardwareMemory());
            break;
        }
        default:
            break;
    }
    FALSE_RETURN_V_MSG_E(mem != nullptr, nullptr, "Create AVMemory failed, no memory, name = %{public}s", name.c_str());

    mem->name_ = name;
    mem->allocator_ = allocator;
    mem->capacity_ = capacity;
    mem->align_ = align;
    Status ret = mem->Init();
    FALSE_RETURN_V_MSG_E(ret == Status::OK, nullptr, "Init AVMemory failed, name = %{public}s", mem->name_.c_str());
    return mem;
}

std::shared_ptr<AVMemory> AVMemory::CreateAVMemory(uint8_t *ptr, int32_t capacity, int32_t size)
{
    std::shared_ptr<AVMemory> mem = std::shared_ptr<AVMemory>(new AVVirtualMemory());
    FALSE_RETURN_V_MSG_E(mem != nullptr, nullptr, "Create AVVirtualMemory failed, no memory");
    mem->name_ = "virtualMemory";
    mem->allocator_ = nullptr;
    mem->capacity_ = capacity;
    mem->size_ = size;
    mem->base_ = ptr;
    return mem;
}

std::shared_ptr<AVMemory> AVMemory::CreateAVMemory(MessageParcel &parcel, bool isSurfaceBuffer)
{
#ifdef MEDIA_OHOS
    if (isSurfaceBuffer) {
        auto mem = std::shared_ptr<AVMemory>(new AVSurfaceMemory());
        Status ret = mem->InitSurfaceBuffer(parcel);
        FALSE_RETURN_V_MSG_E(ret == Status::OK, nullptr, "Init AVSurfaceMemory failed");
        return mem;
    }
    MemoryType type = static_cast<MemoryType>(parcel.ReadUint8());
    std::shared_ptr<AVMemory> mem = nullptr;
    switch (type) {
        case MemoryType::VIRTUAL_MEMORY: {
            return nullptr;
        }
        case MemoryType::SURFACE_MEMORY: {
            mem = std::shared_ptr<AVMemory>(new AVSurfaceMemory());
            break;
        }
        case MemoryType::SHARED_MEMORY: {
            mem = std::shared_ptr<AVMemory>(new AVSharedMemoryExt());
            break;
        }
        case MemoryType::HARDWARE_MEMORY: {
            mem = std::shared_ptr<AVMemory>(new AVHardwareMemory());
            break;
        }
        default:
            break;
    }

    FALSE_RETURN_V_MSG_E(mem != nullptr, nullptr, "Create AVMemory failed, no memory");
    bool isReadParcel = mem->ReadCommonFromMessageParcel(parcel);
    FALSE_RETURN_V_MSG_E(isReadParcel == true, nullptr, "Read common memory info from parcel failed");

    Status ret = mem->Init(parcel);
    FALSE_RETURN_V_MSG_E(ret == Status::OK, nullptr, "Init AVMemory failed, name = %{public}s", mem->name_.c_str());
    return mem;
#else
    return nullptr;
#endif
}

AVMemory::AVMemory() : name_("mem_null"), align_(0), offset_(0), size_(0), base_(nullptr), allocator_(nullptr) {}

AVMemory::~AVMemory() {}

Status AVMemory::Init()
{
    return Status::ERROR_UNIMPLEMENTED;
}

Status AVMemory::Init(MessageParcel &parcel)
{
    (void)parcel;
    return Status::ERROR_UNIMPLEMENTED;
}

Status AVMemory::InitSurfaceBuffer(MessageParcel &parcel)
{
    (void)parcel;
    return Status::ERROR_UNIMPLEMENTED;
}

bool AVMemory::ReadFromMessageParcel(MessageParcel &parcel)
{
    (void)parcel;
    return false;
}

bool AVMemory::WriteToMessageParcel(MessageParcel &parcel)
{
    (void)parcel;
    return false;
}

bool AVMemory::ReadCommonFromMessageParcel(MessageParcel &parcel)
{
#ifdef MEDIA_OHOS
    (void)parcel.ReadUint64();
    bool ret = parcel.ReadString(name_) && parcel.ReadInt32(capacity_);
    FALSE_RETURN_V_MSG_E(capacity_ >= 0, false, "capacity is invalid");

    ret &= parcel.ReadInt32(align_);
    FALSE_RETURN_V_MSG_E(align_ >= 0, false, "align is invalid");

    ret &= parcel.ReadInt32(offset_);
    FALSE_RETURN_V_MSG_E(offset_ >= 0, false, "offset is invalid");

    ret &= parcel.ReadInt32(size_);
    FALSE_RETURN_V_MSG_E((size_ >= 0) || (capacity_ < size_), false, "size is invalid");
    return ret;
#else
    return true;
#endif
}

bool AVMemory::SkipCommonFromMessageParcel(MessageParcel &parcel)
{
#ifdef MEDIA_OHOS
    uint64_t size = 0;
    bool ret = parcel.ReadUint64(size);
    parcel.SkipBytes(static_cast<size_t>(size) - 8); // 8: the size of size_ and offset_

    ret &= parcel.ReadInt32(offset_);
    FALSE_RETURN_V_MSG_E(offset_ >= 0, false, "offset is invalid");

    ret &= parcel.ReadInt32(size_);
    FALSE_RETURN_V_MSG_E((size_ >= 0) || (capacity_ < size_), false, "size is invalid");
    return ret;
#else
    return true;
#endif
}

bool AVMemory::WriteCommonToMessageParcel(MessageParcel &parcel)
{
    bool ret = true;
#ifdef MEDIA_OHOS
    MessageParcel bufferParcel;
    ret = bufferParcel.WriteString(name_) && bufferParcel.WriteInt32(capacity_) && bufferParcel.WriteInt32(align_) &&
          bufferParcel.WriteInt32(offset_) && bufferParcel.WriteInt32(size_);

    size_t size = bufferParcel.GetDataSize();
    return ret && parcel.WriteUint64(static_cast<uint64_t>(size)) && parcel.Append(bufferParcel);
#endif
    return ret;
}

MemoryType AVMemory::GetMemoryType()
{
    return MemoryType::VIRTUAL_MEMORY;
}

MemoryFlag AVMemory::GetMemoryFlag()
{
    return MemoryFlag::MEMORY_READ_WRITE;
}

int32_t AVMemory::GetCapacity()
{
    return capacity_;
}

int32_t AVMemory::GetSize()
{
    return size_;
}

Status AVMemory::SetSize(int32_t size)
{
    size_ = std::max(0, size);
    size_ = std::min(capacity_, size_);
    return Status::OK;
}

int32_t AVMemory::GetOffset()
{
    return offset_;
}

Status AVMemory::SetOffset(int32_t offset)
{
    offset_ = std::max(0, offset);
    offset_ = std::min(capacity_, offset_);
    return Status::OK;
}

uint8_t *AVMemory::GetAddr()
{
    return base_;
}

int32_t AVMemory::GetFileDescriptor()
{
    return -1;
}

int32_t AVMemory::Write(const uint8_t *in, int32_t writeSize, int32_t position)
{
    FALSE_RETURN_V_MSG_E(in != nullptr, 0, "Input buffer is nullptr");
    FALSE_RETURN_V_MSG_E(writeSize > 0, 0, "Input writeSize:" PUBLIC_LOG_D32 " is invalid", writeSize);
    uint8_t *addr = GetAddr();
    FALSE_RETURN_V_MSG_E(addr != nullptr, 0, "Base buffer is nullptr");
    int32_t start = 0;
    if (position <= INVALID_POSITION) {
        start = size_;
    } else {
        start = std::min(position, capacity_);
    }
    int32_t unusedSize = capacity_ - start;
    int32_t length = std::min(writeSize, unusedSize);
    FALSE_RETURN_V_MSG_E((length + start) <= capacity_, 0,
                         "Write out of bounds, length:" PUBLIC_LOG_D32 ", start:" PUBLIC_LOG_D32
                         ", capacity:" PUBLIC_LOG_D32,
                         length, start, capacity_);
    uint8_t *dstPtr = addr + start;
    FALSE_RETURN_V_MSG_E(dstPtr != nullptr, 0, "Inner dstPtr is nullptr");
    auto error = memcpy_s(dstPtr, length, in, length);
    FALSE_RETURN_V_MSG_E(error == EOK, 0, "Inner memcpy_s failed, name:%{public}s, %{public}s", name_.c_str(),
                         strerror(error));
    size_ = start + length;
    return length;
}

int32_t AVMemory::Read(uint8_t *out, int32_t readSize, int32_t position)
{
    FALSE_RETURN_V_MSG_E(out != nullptr, 0, "Output buffer is nullptr");
    FALSE_RETURN_V_MSG_E(readSize > 0, 0, "Output readSize:" PUBLIC_LOG_D32 " is invalid", readSize);
    uint8_t *addr = GetAddr();
    FALSE_RETURN_V_MSG_E(addr != nullptr, 0, "Base buffer is nullptr");
    int32_t start = 0;
    size_t maxLength = size_;
    if (position > INVALID_POSITION) {
        start = std::min(position, size_);
        maxLength = size_ - start;
    }
    int32_t length = std::min(static_cast<size_t>(readSize), maxLength);
    FALSE_RETURN_V_MSG_E((length + start) <= capacity_, 0,
                         "Read out of bounds, length:" PUBLIC_LOG_D32 ", start:" PUBLIC_LOG_D32
                         ", capacity:" PUBLIC_LOG_D32,
                         length, start, capacity_);
    uint8_t *srcPtr = addr + start;
    FALSE_RETURN_V_MSG_E(srcPtr != nullptr, 0, "Inner srcPtr is nullptr");
    auto error = memcpy_s(out, length, srcPtr, length);
    FALSE_RETURN_V_MSG_E(error == EOK, 0, "Inner memcpy_s failed, name:%{public}s, %{public}s", name_.c_str(),
                         strerror(error));
    return length;
}

void AVMemory::Reset()
{
    size_ = 0;
}

sptr<SurfaceBuffer> AVMemory::GetSurfaceBuffer()
{
    return nullptr;
}
} // namespace Media
} // namespace OHOS