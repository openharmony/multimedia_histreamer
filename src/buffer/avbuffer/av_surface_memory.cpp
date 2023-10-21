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
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "message_parcel.h"
#include "scope_guard.h"
#include "surface_buffer.h"
#include "surface_type.h"
#include "sys/mman.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVSurfaceMemory"};
constexpr uint8_t LOGD_FREQUENCY = 5;
} // namespace

namespace OHOS {
namespace MediaAVCodec {
std::shared_ptr<AVAllocator> AVAllocatorFactory::CreateSurfaceAllocator(const BufferRequestConfig &config)
{
    auto allocator = std::shared_ptr<AVSurfaceAllocator>(new AVSurfaceAllocator());
    CHECK_AND_RETURN_RET_LOG(allocator != nullptr, nullptr, "Create AVSurfaceAllocator failed, no memory");
    allocator->config_ = config;
    return allocator;
}

AVSurfaceAllocator::AVSurfaceAllocator() {}

void *AVSurfaceAllocator::Alloc(int32_t capacity)
{
    (void)capacity;
    sptr<SurfaceBuffer> surfaceBuffer = SurfaceBuffer::Create();
    CHECK_AND_RETURN_RET_LOG(surfaceBuffer != nullptr, nullptr, "No memory for new SurfaceBuffer!");
    GSError ret = surfaceBuffer->Alloc(config_);
    CHECK_AND_RETURN_RET_LOG(ret == GSERROR_OK, nullptr, "Surface Buffer Alloc failed, %{public}s",
                             GSErrorStr(ret).c_str());
    surfaceBuffer->IncStrongRef(surfaceBuffer.GetRefPtr());
    return static_cast<void *>(surfaceBuffer.GetRefPtr());
}

bool AVSurfaceAllocator::Free(void *ptr)
{
    CHECK_AND_RETURN_RET_LOG(ptr != nullptr, false, "ptr is nullptr");

    sptr<SurfaceBuffer> surfaceBuffer = sptr<SurfaceBuffer>(static_cast<SurfaceBuffer *>(ptr));
    surfaceBuffer->DecStrongRef(surfaceBuffer.GetRefPtr());

    AVCODEC_LOGD("GetSptrRefCount %{public}d", surfaceBuffer->GetSptrRefCount());
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
    AVCODEC_LOGD_LIMIT(LOGD_FREQUENCY, "enter dtor, instance: 0x%{public}06" PRIXPTR ", name = %{public}s",
                       FAKE_POINTER(this), name_.c_str());
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
    CHECK_AND_RETURN_RET_LOG(surfaceBuffer_ != nullptr, AVCS_ERR_NO_MEMORY, "surfaceBuffer_ alloc failed");
    capacity_ = surfaceBuffer_->GetSize();

    AVCODEC_LOGD_LIMIT(LOGD_FREQUENCY, "enter init, instance: 0x%{public}06" PRIXPTR ", name = %{public}s",
                       FAKE_POINTER(this), name_.c_str());
    return AVCS_ERR_OK;
}

int32_t AVSurfaceMemory::Init(MessageParcel &parcel)
{
    surfaceBuffer_ = SurfaceBuffer::Create();
    CHECK_AND_RETURN_RET_LOG(surfaceBuffer_ != nullptr, AVCS_ERR_NO_MEMORY, "No memory for new SurfaceBuffer!");

    GSError ret = surfaceBuffer_->ReadFromMessageParcel(parcel);
    CHECK_AND_RETURN_RET_LOG(ret == GSERROR_OK, AVCS_ERR_INVALID_OPERATION,
                             "Read surface message parcel failed!, %{public}s", GSErrorStr(ret).c_str());
    capacity_ = surfaceBuffer_->GetSize();

    AVCODEC_LOGD_LIMIT(LOGD_FREQUENCY, "enter init, instance: 0x%{public}06" PRIXPTR ", name = %{public}s",
                       FAKE_POINTER(this), name_.c_str());
    return AVCS_ERR_OK;
}

bool AVSurfaceMemory::WriteToMessageParcel(MessageParcel &parcel)
{
    MessageParcel bufferParcel;
    GSError gsRet = surfaceBuffer_->WriteToMessageParcel(bufferParcel);
    CHECK_AND_RETURN_RET_LOG(gsRet == GSERROR_OK, false, "Write message parcel failed!, %{public}s",
                             GSErrorStr(gsRet).c_str());
    parcel.Append(bufferParcel);
    return true;
}

uint8_t *AVSurfaceMemory::GetAddr()
{
    if (isFirstFlag_) {
        int32_t ret = MapMemoryAddr();
        CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "MapMemory failed");
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
        AVCODEC_LOGE("create avsurfacememory failed, name = %{public}s, size = %{public}d", name_.c_str(), capacity_);
        Close();
        return AVCS_ERR_NO_MEMORY;
    };
    GSError ret = surfaceBuffer_->Map();
    CHECK_AND_RETURN_RET_LOG(ret == GSERROR_OK, AVCS_ERR_INVALID_OPERATION,
                             "mmap failed, please check params, %{public}s", GSErrorStr(ret).c_str());
    base_ = reinterpret_cast<uint8_t *>(surfaceBuffer_->GetVirAddr());
    CANCEL_SCOPE_EXIT_GUARD(0);
    return AVCS_ERR_OK;
}
} // namespace MediaAVCodec
} // namespace OHOS
