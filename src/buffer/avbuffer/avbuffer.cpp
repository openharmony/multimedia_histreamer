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

#include <atomic>
#include <iomanip>
#include <sstream>
#include "av_hardware_memory.h"
#include "av_shared_memory_ext.h"
#include "avbuffer_utils.h"
#include "buffer/avbuffer.h"
#include "common/log.h"
#include "common/status.h"
#include "meta/meta.h"
#include "surface_buffer.h"
#include "surface_type.h"
#include "unistd.h"

namespace {
    constexpr uint16_t BUFFERID_BOUNDARY = 0xffff;
} // namespace

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
        switch (config_.memoryType) {
            case MemoryType::VIRTUAL_MEMORY: {
                config_.memoryFlag = MemoryFlag::MEMORY_READ_WRITE;
                break;
            }
            case MemoryType::SHARED_MEMORY: {
                config_.memoryFlag = std::static_pointer_cast<AVSharedMemoryExt>(memory_)->GetMemoryFlag();
                break;
            }
            case MemoryType::HARDWARE_MEMORY: {
                config_.memoryFlag = std::static_pointer_cast<AVHardwareMemory>(memory_)->GetMemoryFlag();
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

std::shared_ptr<AVBuffer> AVBuffer::CreateAVBuffer(MessageParcel &parcel, bool isSurfaceBuffer)
{
    auto buffer = std::shared_ptr<AVBuffer>(new AVBuffer());
    FALSE_RETURN_V_MSG_E(buffer != nullptr, nullptr, "Create AVBuffer failed, no memory");

    buffer->meta_ = std::make_shared<Meta>();
    FALSE_RETURN_V_MSG_E(buffer->meta_ != nullptr, nullptr, "Create meta_ failed, no memory");

    int32_t ret = buffer->Init(parcel, isSurfaceBuffer);
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
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << GetUniqueId();
    std::string uidName = ss.str();
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

int32_t AVBuffer::Init(MessageParcel &parcel, bool isSurfaceBuffer)
{
    if (isSurfaceBuffer) {
        memory_ = AVMemory::CreateAVMemory(parcel, true);
        FALSE_RETURN_V_MSG_E(memory_ != nullptr, static_cast<int32_t>(Status::ERROR_UNKNOWN), "Create memory failed");
        std::stringstream ss;
        ss << std::hex << std::setw(16) << std::setfill('0') << GetUniqueId();
        memory_->name_ = ss.str();
        return static_cast<int32_t>(Status::OK);
    }
    uid_ = parcel.ReadUint64();
    pts_ = parcel.ReadInt64();
    dts_ = parcel.ReadInt64();
    duration_ = parcel.ReadInt64();
    flag_ = parcel.ReadUint32();

    bool ret = meta_->FromParcel(parcel);
    FALSE_RETURN_V_MSG_E(ret, static_cast<int32_t>(Status::ERROR_UNKNOWN), "Unmarshalling meta_ failed");

    bool isBufferAttrToParcel = parcel.ReadBool();
    if (isBufferAttrToParcel) {
        return static_cast<int32_t>(Status::OK);
    }
    memory_ = AVMemory::CreateAVMemory(parcel, false);
    FALSE_RETURN_V_MSG_E(memory_ != nullptr, static_cast<int32_t>(Status::ERROR_UNKNOWN), "Create memory failed");
    return static_cast<int32_t>(Status::OK);
}

uint64_t AVBuffer::GetUniqueId()
{
    using namespace std::chrono;
    static const uint32_t processId = static_cast<uint32_t>(getpid());
    static std::atomic<uint16_t> bufferId = 0;
    if (uid_ == 0) {
        union UniqueId {
            uint64_t nowTime;
            uint32_t processId[2];
            uint16_t bufferId[4];
        } uid{0};
        if (bufferId == BUFFERID_BOUNDARY) {
            bufferId = 0;
        }
        ++bufferId;
        uid.nowTime = time_point_cast<nanoseconds>(system_clock::now()).time_since_epoch().count();
        uid.processId[1] = processId;
        uid.bufferId[3] = bufferId;
        uid_ = uid.nowTime;
    }
    return uid_;
}

bool AVBuffer::WriteToMessageParcel(MessageParcel &parcel)
{
    MessageParcel bufferParcel;
    bool isBufferAttrToParcel = (memory_ == nullptr);
    bool ret = bufferParcel.WriteUint64(GetUniqueId()) && bufferParcel.WriteInt64(pts_) &&
               bufferParcel.WriteInt64(dts_) && bufferParcel.WriteInt64(duration_) && bufferParcel.WriteUint32(flag_) &&
               meta_->ToParcel(bufferParcel) &&
               bufferParcel.WriteBool(isBufferAttrToParcel);

    if (!isBufferAttrToParcel) {
        MemoryType type = memory_->GetMemoryType();
        FALSE_RETURN_V_MSG_E(type != MemoryType::VIRTUAL_MEMORY, false, "Virtual memory not support");

        ret = ret && bufferParcel.WriteUint8(static_cast<uint8_t>(type)) &&
              memory_->WriteCommonToMessageParcel(bufferParcel) && memory_->WriteToMessageParcel(bufferParcel);
    }
    if (ret) {
        parcel.Append(bufferParcel);
    }
    return ret;
}
} // namespace Media
} // namespace OHOS