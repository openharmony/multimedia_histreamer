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

#include "buffer/avbuffer.h"
#include <atomic>
#include <iomanip>
#include <sstream>
#include "avbuffer_utils.h"
#include "common/log.h"
#include "common/status.h"
#include "meta/meta.h"
#include "surface_buffer.h"
#include "surface_type.h"
#include "unistd.h"

namespace OHOS {
namespace Media {
AVBuffer::AVBuffer() : uid_(0) {}
std::shared_ptr<AVBuffer> AVBuffer::CreateAVBuffer(const AVBufferConfig &config)
{
    std::shared_ptr<AVAllocator> allocator = nullptr;
    int32_t capacity = std::max(config.size, config.capacity);
    MemoryFlag memflag = MemoryFlag::MEMORY_READ_WRITE;
    switch (config.memoryType) {
        case MemoryType::VIRTUAL_MEMORY: {
            allocator = AVAllocatorFactory::CreateVirtualAllocator();
            break;
        }
        case MemoryType::SHARED_MEMORY: {
            memflag = config.memoryFlag;
            allocator = AVAllocatorFactory::CreateSharedAllocator(config.memoryFlag);
            break;
        }
        case MemoryType::SURFACE_MEMORY: {
            allocator = AVAllocatorFactory::CreateSurfaceAllocator(*(config.surfaceBufferConfig));
            break;
        }
        case MemoryType::HARDWARE_MEMORY: {
            memflag = config.memoryFlag;
            allocator = AVAllocatorFactory::CreateHardwareAllocator(config.dmaFd, capacity, config.memoryFlag);
            break;
        }
        default:
            return nullptr;
    }
    auto buffer = CreateAVBuffer(allocator, capacity, config.align);
    if (buffer != nullptr) {
        buffer->config_ = config;
        buffer->config_.capacity = capacity;
        buffer->config_.memoryFlag = memflag;
    }
    return buffer;
}

const AVBufferConfig &AVBuffer::GetConfig()
{
    if (memory_ == nullptr) {
        return config_;
    }
    config_.size = memory_->GetSize();
    if (config_.memoryType == MemoryType::UNKNOWN_MEMORY) {
        config_.memoryType = memory_->GetMemoryType();
        config_.capacity = memory_->GetCapacity();
        config_.align = memory_->align_;
        config_.memoryFlag = memory_->GetMemoryFlag();
        switch (config_.memoryType) {
            case MemoryType::VIRTUAL_MEMORY: {
                break;
            }
            case MemoryType::SHARED_MEMORY: {
                break;
            }
            case MemoryType::HARDWARE_MEMORY: {
                config_.dmaFd = memory_->GetFileDescriptor();
                break;
            }
            case MemoryType::SURFACE_MEMORY: {
                auto surfaceBuffer = memory_->GetSurfaceBuffer();
                config_.surfaceBufferConfig->width = surfaceBuffer->GetWidth();
                config_.surfaceBufferConfig->height = surfaceBuffer->GetHeight();
                config_.surfaceBufferConfig->strideAlignment = surfaceBuffer->GetStride();
                config_.surfaceBufferConfig->format = surfaceBuffer->GetFormat();
                config_.surfaceBufferConfig->usage = surfaceBuffer->GetUsage();
                config_.surfaceBufferConfig->colorGamut = surfaceBuffer->GetSurfaceBufferColorGamut();
                config_.surfaceBufferConfig->transform = surfaceBuffer->GetSurfaceBufferTransform();
                break;
            }
            default:
                break;
        }
    }
    return config_;
}

std::shared_ptr<AVBuffer> AVBuffer::CreateAVBuffer(std::shared_ptr<AVAllocator> allocator, int32_t capacity,
                                                   int32_t align)
{
    FALSE_RETURN_V_MSG_E(allocator != nullptr, nullptr, "allocator is nullptr");
    FALSE_RETURN_V_MSG_E(capacity >= 0, nullptr, "capacity is invalid");
    FALSE_RETURN_V_MSG_E(align >= 0, nullptr, "align is invalid");

    auto buffer = std::shared_ptr<AVBuffer>(new AVBuffer());
    FALSE_RETURN_V_MSG_E(buffer != nullptr, nullptr, "Create AVBuffer failed, no memory");

    int32_t ret = buffer->Init(allocator, capacity, align);
    FALSE_RETURN_V_MSG_E(ret == static_cast<int32_t>(Status::OK), nullptr, "Init AVBuffer failed");

    buffer->meta_ = std::make_shared<Meta>();
    FALSE_RETURN_V_MSG_E(buffer->meta_ != nullptr, nullptr, "Create meta_ failed, no memory");
    return buffer;
}

std::shared_ptr<AVBuffer> AVBuffer::CreateAVBuffer(uint8_t *ptr, int32_t capacity, int32_t size)
{
    FALSE_RETURN_V_MSG_E(ptr != nullptr, nullptr, "ptr is nullptr");
    FALSE_RETURN_V_MSG_E(capacity >= 0, nullptr, "capacity is invalid");
    FALSE_RETURN_V_MSG_E((0 <= size) && (size <= capacity), nullptr, "size is invalid");

    auto buffer = std::shared_ptr<AVBuffer>(new AVBuffer());
    FALSE_RETURN_V_MSG_E(buffer != nullptr, nullptr, "Create AVBuffer failed, no memory");

    buffer->meta_ = std::make_shared<Meta>();
    FALSE_RETURN_V_MSG_E(buffer->meta_ != nullptr, nullptr, "Create meta_ failed, no memory");

    int32_t ret = buffer->Init(ptr, capacity, size);
    FALSE_RETURN_V_MSG_E(ret == static_cast<int32_t>(Status::OK), nullptr, "Init AVBuffer failed");
    return buffer;
}

std::shared_ptr<AVBuffer> AVBuffer::CreateAVBuffer()
{
    auto buffer = std::shared_ptr<AVBuffer>(new AVBuffer());
    FALSE_RETURN_V_MSG_E(buffer != nullptr, nullptr, "Create AVBuffer failed, no memory");

    buffer->meta_ = std::make_shared<Meta>();
    FALSE_RETURN_V_MSG_E(buffer->meta_ != nullptr, nullptr, "Create meta_ failed, no memory");
    return buffer;
}

int32_t AVBuffer::Init(std::shared_ptr<AVAllocator> allocator, int32_t capacity, int32_t align)
{
    std::string uidName = std::to_string(GetUniqueId());
    memory_ = AVMemory::CreateAVMemory(uidName, allocator, capacity, align);
    FALSE_RETURN_V_MSG_E(memory_ != nullptr, static_cast<int32_t>(Status::ERROR_UNKNOWN), "Create memory failed");
    return static_cast<int32_t>(Status::OK);
}

int32_t AVBuffer::Init(uint8_t *ptr, int32_t capacity, int32_t size)
{
    memory_ = AVMemory::CreateAVMemory(ptr, capacity, size);
    FALSE_RETURN_V_MSG_E(memory_ != nullptr, static_cast<int32_t>(Status::ERROR_UNKNOWN), "Create memory failed");
    return static_cast<int32_t>(Status::OK);
}

uint64_t AVBuffer::GetUniqueId()
{
#ifdef MEDIA_OHOS
    using namespace std::chrono;
    static const uint64_t startTime = time_point_cast<seconds>(system_clock::now()).time_since_epoch().count();
    static const uint16_t processId = static_cast<uint16_t>(getpid());
#else
    static const uint64_t startTime = 0;
    static const uint16_t processId = 0;
#endif
    static std::atomic<uint32_t> bufferId = 0;
    if (memory_ == nullptr) {
        uid_ = 0;
        return 0;
    }
    if (uid_ == 0) {
        if (bufferId == UINT32_MAX) {
            bufferId = 0;
        }
        union UniqueId {
            uint64_t startTime;    //  1--16, 16: time
            uint16_t processId[4]; // 17--32, 16: process id
            uint32_t bufferId[2];  // 33--64, 32: atomic val
        } uid = {.startTime = startTime};
        ++bufferId;
        uid.processId[1] = processId;
        uid.bufferId[1] = bufferId;
        uid_ = uid.startTime;
    }
    return uid_;
}

bool AVBuffer::WriteToMessageParcel(MessageParcel &parcel)
{
#ifdef MEDIA_OHOS
    MessageParcel bufferParcel;
    bool ret = bufferParcel.WriteUint64(GetUniqueId()) && bufferParcel.WriteInt64(pts_) &&
               bufferParcel.WriteInt64(dts_) && bufferParcel.WriteInt64(duration_) && bufferParcel.WriteUint32(flag_) &&
               meta_->ToParcel(bufferParcel);

    if (memory_ != nullptr) {
        MemoryType type = memory_->GetMemoryType();
        FALSE_RETURN_V_MSG_E(type != MemoryType::VIRTUAL_MEMORY, false, "Virtual memory not support");

        ret = ret && bufferParcel.WriteUint8(static_cast<uint8_t>(type)) &&
              memory_->WriteCommonToMessageParcel(bufferParcel) && memory_->WriteToMessageParcel(bufferParcel);
    }
    if (ret) {
        parcel.Append(bufferParcel);
    }
    return ret;
#else
    return false;
#endif
}

bool AVBuffer::ReadFromMessageParcel(MessageParcel &parcel, bool isSurfaceBuffer)
{
#ifdef MEDIA_OHOS
    if (isSurfaceBuffer) {
        if (memory_ != nullptr) {
            return false;
        }
        memory_ = AVMemory::CreateAVMemory(parcel, true);
        FALSE_RETURN_V_MSG_E(memory_ != nullptr, false, "Create memory failed");

        memory_->name_ = std::to_string(GetUniqueId());
        return true;
    }
    // 1. 不同buffer更新attr：  memroy != nullptr，uid != fromParcel， 返回错误
    // 2. 相同buffer更新attr：  memroy != nullptr，uid == fromParcel，不创建memory，更新attr + memory的attr
    // 3. 初始化buffer：        memroy == nullptr，fromParcel != 0，创建memory
    // 4. 只传buffer的attr：    memroy == nullptr，fromParcel == 0，更新attr
    uint64_t uid = 0;
    int64_t pts = 0;
    int64_t dts = 0;
    int64_t duration = 0;
    uint32_t flag = 0;
    auto meta = std::make_shared<Meta>();
    bool ret = parcel.ReadUint64(uid) && parcel.ReadInt64(pts) && parcel.ReadInt64(dts) && parcel.ReadInt64(duration) &&
               parcel.ReadUint32(flag) && meta->FromParcel(parcel);
    FALSE_RETURN_V_MSG_E(ret, false, "Unmarshalling buffer info failed");

    if (memory_ != nullptr) {
        FALSE_RETURN_V_MSG_E(GetUniqueId() == uid, false, "Can't read message parcel from other AVBuffer object!");
        (void)parcel.ReadUint8();
        ret = memory_->SkipCommonFromMessageParcel(parcel) && memory_->ReadFromMessageParcel(parcel);
        FALSE_RETURN_V_MSG_E(ret, false, "Update memory info failed");
    } else if (uid != 0) {
        uid_ = uid;
        memory_ = AVMemory::CreateAVMemory(parcel, false);
        FALSE_RETURN_V_MSG_E(memory_ != nullptr, false, "Create memory failed");
    }
    pts_ = pts;
    dts_ = dts;
    duration_ = duration;
    flag_ = flag;
    meta_ = meta;
#endif
    return true;
}
} // namespace Media
} // namespace OHOS