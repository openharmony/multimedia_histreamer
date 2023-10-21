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

#include "avbuffer.h"
#include <atomic>
#include <iomanip>
#include <sstream>
#include "av_common.h"
#include "av_hardware_memory.h"
#include "av_shared_memory_ext.h"
#include "avbuffer_utils.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "surface_buffer.h"
#include "surface_type.h"
#include "unistd.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVBuffer"};
const uint64_t NANOSEC_MASK = 0xFFFFFFFF;
std::atomic<uint64_t> g_bufferId = 0;
} // namespace

namespace OHOS {
namespace MediaAVCodec {
AVBuffer::AVBuffer() : uid_(0) {}
std::shared_ptr<AVBuffer> AVBuffer::CreateAVBuffer(const AVBufferConfig &config)
{
    std::shared_ptr<AVAllocator> allocator = nullptr;
    switch (config.memoryType) {
        case MemoryType::VIRTUAL_MEMORY: {
            allocator = AVAllocatorFactory::CreateVirtualAllocator();
            break;
        }
        case MemoryType::SHARED_MEMORY: {
            allocator = AVAllocatorFactory::CreateSharedAllocator(config.memoryFlag);
            break;
        }
        case MemoryType::SURFACE_MEMORY: {
            allocator = AVAllocatorFactory::CreateSurfaceAllocator(config.surfaceBufferConfig);
            break;
        }
        case MemoryType::HARDWARE_MEMORY: {
            allocator = AVAllocatorFactory::CreateHardwareAllocator(config.dmaFd, config.capacity, config.memoryFlag);
            break;
        }
        default:
            return nullptr;
    }
    auto buffer = CreateAVBuffer(allocator, config.capacity, config.align);
    if (buffer != nullptr) {
        buffer->config_ = config;
    }
    return buffer;
}

AVBufferConfig AVBuffer::GetConfig()
{
    if ((config_.memoryType == MemoryType::UNKNOWN_MEMORY) && (memory_ != nullptr)) {
        config_.memoryType = memory_->GetMemoryType();
        config_.capacity = memory_->capacity_;
        config_.align = memory_->align_;
        switch (config_.memoryType) {
            case MemoryType::SHARED_MEMORY: {
                config_.memoryFlag = std::static_pointer_cast<AVSharedMemoryExt>(memory_)->GetMemFlag();
                break;
            }
            case MemoryType::HARDWARE_MEMORY: {
                config_.memoryFlag = std::static_pointer_cast<AVHardwareMemory>(memory_)->GetMemFlag();
                config_.dmaFd = memory_->GetFileDescriptor();
                break;
            }
            case MemoryType::SURFACE_MEMORY: {
                auto surfaceBuffer = memory_->GetSurfaceBuffer();
                config_.surfaceBufferConfig.width = surfaceBuffer->GetWidth();
                config_.surfaceBufferConfig.height = surfaceBuffer->GetHeight();
                config_.surfaceBufferConfig.strideAlignment = surfaceBuffer->GetStride();
                config_.surfaceBufferConfig.format = surfaceBuffer->GetFormat();
                config_.surfaceBufferConfig.usage = surfaceBuffer->GetUsage();
                config_.surfaceBufferConfig.colorGamut = surfaceBuffer->GetSurfaceBufferColorGamut();
                config_.surfaceBufferConfig.transform = surfaceBuffer->GetSurfaceBufferTransform();
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
    CHECK_AND_RETURN_RET_LOG(allocator != nullptr, nullptr, "allocator is nullptr");
    CHECK_AND_RETURN_RET_LOG(capacity >= 0, nullptr, "capacity is invalid");
    CHECK_AND_RETURN_RET_LOG(align >= 0, nullptr, "align is invalid");

    auto buffer = std::shared_ptr<AVBuffer>(new AVBuffer());
    CHECK_AND_RETURN_RET_LOG(buffer != nullptr, nullptr, "Create AVBuffer failed, no memory");

    int32_t ret = buffer->Init(allocator, capacity, align);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "Init AVBuffer failed");

    buffer->meta_ = std::make_shared<Format>();
    CHECK_AND_RETURN_RET_LOG(buffer->meta_ != nullptr, nullptr, "Create meta_ failed, no memory");
    return buffer;
}

std::shared_ptr<AVBuffer> AVBuffer::CreateAVBuffer(uint8_t *ptr, int32_t capacity, int32_t size)
{
    CHECK_AND_RETURN_RET_LOG(ptr != nullptr, nullptr, "ptr is nullptr");
    CHECK_AND_RETURN_RET_LOG(capacity >= 0, nullptr, "capacity is invalid");
    CHECK_AND_RETURN_RET_LOG((0 <= size) && (size <= capacity), nullptr, "size is invalid");

    auto buffer = std::shared_ptr<AVBuffer>(new AVBuffer());
    CHECK_AND_RETURN_RET_LOG(buffer != nullptr, nullptr, "Create AVBuffer failed, no memory");

    buffer->meta_ = std::make_shared<Format>();
    CHECK_AND_RETURN_RET_LOG(buffer->meta_ != nullptr, nullptr, "Create meta_ failed, no memory");

    int32_t ret = buffer->Init(ptr, capacity, size);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "Init AVBuffer failed");
    return buffer;
}

std::shared_ptr<AVBuffer> AVBuffer::CreateAVBuffer(MessageParcel &parcel, bool isSurfaceBuffer)
{
    auto buffer = std::shared_ptr<AVBuffer>(new AVBuffer());
    CHECK_AND_RETURN_RET_LOG(buffer != nullptr, nullptr, "Create AVBuffer failed, no memory");

    buffer->meta_ = std::make_shared<Format>();
    CHECK_AND_RETURN_RET_LOG(buffer->meta_ != nullptr, nullptr, "Create meta_ failed, no memory");

    int32_t ret = buffer->Init(parcel, isSurfaceBuffer);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "Init AVBuffer failed");
    return buffer;
}

std::shared_ptr<AVBuffer> AVBuffer::CreateAVBuffer()
{
    auto buffer = std::shared_ptr<AVBuffer>(new AVBuffer());
    CHECK_AND_RETURN_RET_LOG(buffer != nullptr, nullptr, "Create AVBuffer failed, no memory");

    buffer->meta_ = std::make_shared<Format>();
    CHECK_AND_RETURN_RET_LOG(buffer->meta_ != nullptr, nullptr, "Create meta_ failed, no memory");
    return buffer;
}

int32_t AVBuffer::Init(std::shared_ptr<AVAllocator> allocator, int32_t capacity, int32_t align)
{
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << GetUniqueId();
    std::string uidName = ss.str();
    memory_ = AVMemory::CreateAVMemory(uidName, allocator, capacity, align);
    CHECK_AND_RETURN_RET_LOG(memory_ != nullptr, AVCS_ERR_UNKNOWN, "Create memory failed");
    return AVCS_ERR_OK;
}

int32_t AVBuffer::Init(uint8_t *ptr, int32_t capacity, int32_t size)
{
    memory_ = AVMemory::CreateAVMemory(ptr, capacity, size);
    CHECK_AND_RETURN_RET_LOG(memory_ != nullptr, AVCS_ERR_UNKNOWN, "Create memory failed");
    return AVCS_ERR_OK;
}

int32_t AVBuffer::Init(MessageParcel &parcel, bool isSurfaceBuffer)
{
    if (isSurfaceBuffer) {
        memory_ = AVMemory::CreateAVMemory(parcel, true);
        CHECK_AND_RETURN_RET_LOG(memory_ != nullptr, AVCS_ERR_UNKNOWN, "Create memory failed");
        std::stringstream ss;
        ss << std::hex << std::setw(16) << std::setfill('0') << GetUniqueId();
        memory_->name_ = ss.str();
        return AVCS_ERR_OK;
    }
    uid_ = parcel.ReadUint64();
    pts_ = parcel.ReadInt64();
    dts_ = parcel.ReadInt64();
    duration_ = parcel.ReadInt64();
    flag_ = parcel.ReadUint32();

    bool ret = Unmarshalling(parcel, *(meta_));
    CHECK_AND_RETURN_RET_LOG(ret, AVCS_ERR_UNKNOWN, "Unmarshalling meta_ failed");

    bool isBufferAttrToParcel = parcel.ReadBool();
    if (isBufferAttrToParcel) {
        return AVCS_ERR_OK;
    }
    memory_ = AVMemory::CreateAVMemory(parcel, false);
    CHECK_AND_RETURN_RET_LOG(memory_ != nullptr, AVCS_ERR_UNKNOWN, "Create memory failed");
    return AVCS_ERR_OK;
}

uint64_t AVBuffer::GetUniqueId()
{
    using namespace std::chrono;
    if (uid_ == 0) {
        uint64_t pid = static_cast<uint64_t>(getpid());
        uint64_t nowTime = time_point_cast<nanoseconds>(system_clock::now()).time_since_epoch().count();
        uint64_t bufferId = (NANOSEC_MASK & nowTime) - g_bufferId;
        ++g_bufferId;
        uid_ = (pid << 32) | bufferId;
    }
    return uid_;
}

bool AVBuffer::WriteToMessageParcel(MessageParcel &parcel)
{
    MessageParcel bufferParcel;
    bool isBufferAttrToParcel = (memory_ == nullptr);
    bool ret = bufferParcel.WriteUint64(GetUniqueId()) && bufferParcel.WriteInt64(pts_) &&
               bufferParcel.WriteInt64(dts_) && bufferParcel.WriteInt64(duration_) && bufferParcel.WriteUint32(flag_) &&
               Marshalling(bufferParcel, *(meta_)) && bufferParcel.WriteBool(isBufferAttrToParcel);

    if (!isBufferAttrToParcel) {
        MemoryType type = memory_->GetMemoryType();
        CHECK_AND_RETURN_RET_LOG(type != MemoryType::VIRTUAL_MEMORY, false, "Virtual memory not support");

        ret = ret && bufferParcel.WriteUint8(static_cast<uint8_t>(type)) &&
              memory_->WriteCommonToMessageParcel(bufferParcel) && memory_->WriteToMessageParcel(bufferParcel);
    }
    if (ret) {
        parcel.Append(bufferParcel);
    }
    return ret;
}
} // namespace MediaAVCodec
} // namespace OHOS