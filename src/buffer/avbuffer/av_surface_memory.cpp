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

#include "av_surface_memory.h"
#include <unistd.h>
#include "av_surface_allocator.h"
#include "common/log.h"
#include "common/status.h"
#include "message_parcel.h"
#include "scope_guard.h"
#include "surface_buffer.h"
#include "surface_type.h"

namespace OHOS {
namespace Media {
std::shared_ptr<AVAllocator> AVAllocatorFactory::CreateSurfaceAllocator(const BufferRequestConfig &config)
{
    auto allocator = std::shared_ptr<AVSurfaceAllocator>(new AVSurfaceAllocator());
    FALSE_RETURN_V_MSG_E(allocator != nullptr, nullptr, "Create AVSurfaceAllocator failed, no memory");
    allocator->config_ = config;
    return allocator;
}

AVSurfaceAllocator::AVSurfaceAllocator() {}

void *AVSurfaceAllocator::Alloc(int32_t capacity)
{
    (void)capacity;
    sptr<SurfaceBuffer> surfaceBuffer = SurfaceBuffer::Create();
    FALSE_RETURN_V_MSG_E(surfaceBuffer != nullptr, nullptr, "No memory for new SurfaceBuffer!");
    GSError ret = surfaceBuffer->Alloc(config_);
    FALSE_RETURN_V_MSG_E(ret == GSERROR_OK, nullptr, "Surface Buffer Alloc failed, %{public}s",
                         GSErrorStr(ret).c_str());
    surfaceBuffer->IncStrongRef(surfaceBuffer.GetRefPtr());
    return static_cast<void *>(surfaceBuffer.GetRefPtr());
}

bool AVSurfaceAllocator::Free(void *ptr)
{
    FALSE_RETURN_V_MSG_E(ptr != nullptr, false, "ptr is nullptr");

    sptr<SurfaceBuffer> surfaceBuffer = sptr<SurfaceBuffer>(static_cast<SurfaceBuffer *>(ptr));
    surfaceBuffer->DecStrongRef(surfaceBuffer.GetRefPtr());

    MEDIA_LOG_D("GetSptrRefCount " PUBLIC_LOG_D32, surfaceBuffer->GetSptrRefCount());
    surfaceBuffer = nullptr;
    return true;
}

MemoryType AVSurfaceAllocator::GetMemoryType()
{
    return MemoryType::SURFACE_MEMORY;
}

AVSurfaceMemory::AVSurfaceMemory() : isFirstFlag_(true) {}

AVSurfaceMemory::~AVSurfaceMemory()
{
    MEDIA_LOG_DD("enter dtor, instance: 0x%{public}06" PRIXPTR ", name = %{public}s", FAKE_POINTER(this),
                 name_.c_str());
    if (base_ != nullptr) {
        surfaceBuffer_->Unmap();
        base_ = nullptr;
    }
    if (allocator_ != nullptr) {
        allocator_->Free(surfaceBuffer_.GetRefPtr());
    }
    surfaceBuffer_ = nullptr;
}

int32_t AVSurfaceMemory::Init()
{
    surfaceBuffer_ = sptr<SurfaceBuffer>(static_cast<SurfaceBuffer *>(allocator_->Alloc(0)));
    FALSE_RETURN_V_MSG_E(surfaceBuffer_ != nullptr, static_cast<int32_t>(Status::ERROR_NO_MEMORY),
                         "surfaceBuffer_ alloc failed");
    capacity_ = surfaceBuffer_->GetSize();

    MEDIA_LOG_DD("enter init, instance: 0x%{public}06" PRIXPTR ", name = %{public}s", FAKE_POINTER(this),
                 name_.c_str());
    return static_cast<int32_t>(Status::OK);
}

int32_t AVSurfaceMemory::Init(MessageParcel &parcel)
{
    (void)parcel.ReadUint64();
    return InitSurfaceBuffer(parcel);
}

int32_t AVSurfaceMemory::InitSurfaceBuffer(MessageParcel &parcel)
{
    surfaceBuffer_ = SurfaceBuffer::Create();
    FALSE_RETURN_V_MSG_E(surfaceBuffer_ != nullptr, static_cast<int32_t>(Status::ERROR_NO_MEMORY),
                         "No memory for new SurfaceBuffer!");

    GSError ret = surfaceBuffer_->ReadFromMessageParcel(parcel);
    FALSE_RETURN_V_MSG_E(ret == GSERROR_OK, static_cast<int32_t>(Status::ERROR_INVALID_OPERATION),
                         "Read surface message parcel failed!, %{public}s", GSErrorStr(ret).c_str());
    capacity_ = surfaceBuffer_->GetSize();

    MEDIA_LOG_DD("enter init, instance: 0x%{public}06" PRIXPTR ", name = %{public}s", FAKE_POINTER(this),
                 name_.c_str());
    return static_cast<int32_t>(Status::OK);
}

bool AVSurfaceMemory::WriteToMessageParcel(MessageParcel &parcel)
{
#ifdef MEDIA_OHOS
    MessageParcel bufferParcel;
    GSError gsRet = surfaceBuffer_->WriteToMessageParcel(bufferParcel);
    FALSE_RETURN_V_MSG_E(gsRet == GSERROR_OK, false, "Write message parcel failed!, %{public}s",
                         GSErrorStr(gsRet).c_str());
    size_t size = bufferParcel.GetDataSize();
    return parcel.WriteUint64(static_cast<uint64_t>(size)) && parcel.Append(bufferParcel);
#else
    return true;
#endif
}

bool AVSurfaceMemory::ReadFromMessageParcel(MessageParcel &parcel)
{
#ifdef MEDIA_OHOS
    uint64_t size = 0;
    bool ret = parcel.ReadUint64(size);
    parcel.SkipBytes(static_cast<size_t>(size));
    return ret;
#else
    return true;
#endif
}

uint8_t *AVSurfaceMemory::GetAddr()
{
    if (isFirstFlag_) {
        int32_t ret = MapMemoryAddr();
        FALSE_RETURN_V_MSG_E(ret == static_cast<int32_t>(Status::OK), nullptr, "MapMemory failed");
        isFirstFlag_ = false;
    }
    return base_;
}

MemoryType AVSurfaceMemory::GetMemoryType()
{
    return MemoryType::SURFACE_MEMORY;
}

int32_t AVSurfaceMemory::GetFileDescriptor()
{
    return surfaceBuffer_->GetFileDescriptor();
}

sptr<SurfaceBuffer> AVSurfaceMemory::GetSurfaceBuffer()
{
    return surfaceBuffer_;
}

void AVSurfaceMemory::Close()
{
    if (base_ != nullptr) {
        surfaceBuffer_->Unmap();
        base_ = nullptr;
    }
}

int32_t AVSurfaceMemory::MapMemoryAddr()
{
    ON_SCOPE_EXIT(0)
    {
        MEDIA_LOG_E("create avsurfacememory failed, name = %{public}s, size = " PUBLIC_LOG_D32, name_.c_str(),
                    capacity_);
        Close();
        return static_cast<int32_t>(Status::ERROR_NO_MEMORY);
    };
    GSError ret = surfaceBuffer_->Map();
    FALSE_RETURN_V_MSG_E(ret == GSERROR_OK, static_cast<int32_t>(Status::ERROR_INVALID_OPERATION),
                         "mmap failed, please check params, %{public}s", GSErrorStr(ret).c_str());
    base_ = reinterpret_cast<uint8_t *>(surfaceBuffer_->GetVirAddr());
    CANCEL_SCOPE_EXIT_GUARD(0);
    return static_cast<int32_t>(Status::OK);
}
} // namespace Media
} // namespace OHOS
