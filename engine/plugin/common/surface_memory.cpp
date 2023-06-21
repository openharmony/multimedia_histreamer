/*
 * Copyright (c) 2021-2021 Huawei Device Co., Ltd.
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

#if !defined(OHOS_LITE) && defined(VIDEO_SUPPORT)
#include "plugin/common/surface_memory.h"
#include <utility>
#include "foundation/log.h"
#include "plugin/common/surface_allocator.h"

namespace OHOS {
namespace Media {
namespace Plugin {
SurfaceMemory::SurfaceMemory(size_t capacity, std::shared_ptr<Allocator> allocator, size_t align)
    : Memory(capacity, std::move(allocator), align, MemoryType::SURFACE_BUFFER, false),
      fence_(-1),
      stride_(0)
{
    MEDIA_LOG_DD("SurfaceMemory ctor.");
    if (this->allocator != nullptr && this->allocator->GetMemoryType() == MemoryType::SURFACE_BUFFER) {
        surfaceAllocator_ = ReinterpretPointerCast<SurfaceAllocator>(this->allocator);
        AllocSurfaceBuffer();
    }
}

SurfaceMemory::SurfaceMemory(sptr<SurfaceBuffer> surfaceBuffer, int32_t surfaceCapacity)
    : Memory(surfaceCapacity, nullptr, 1, MemoryType::SURFACE_BUFFER, false), // align 1
      surfaceBuffer_(surfaceBuffer), fence_(-1), stride_(0)
{
}

SurfaceMemory::~SurfaceMemory()
{
    MEDIA_LOG_DD("SurfaceMemory dtor.");
    ReleaseSurfaceBuffer();
}

void SurfaceMemory::AllocSurfaceBuffer()
{
    if (surfaceAllocator_ == nullptr || surfaceBuffer_ != nullptr) {
        MEDIA_LOG_E("No need to allocate surface buffer.");
        return;
    }
    surfaceBuffer_ = surfaceAllocator_->AllocSurfaceBuffer();
    if (surfaceBuffer_ != nullptr) {
        auto bufferHandle = surfaceBuffer_->GetBufferHandle();
        if (bufferHandle != nullptr) {
            stride_ = bufferHandle->stride;
        }
        fence_ = -1;
    } else {
        // Surface often obtain buffer failed, but doesn't cause any problem.
        MEDIA_LOG_DD("AllocSurfaceBuffer failed.");
    }
}

sptr<SurfaceBuffer> SurfaceMemory::GetSurfaceBuffer()
{
    OSAL::ScopedLock l(memMutex_);
    if (!surfaceBuffer_ || needRender_) {
        // request surface buffer again when old buffer flush to nullptr
        surfaceBuffer_ = nullptr;
        AllocSurfaceBuffer();
        needRender_ = false;
    }
    return surfaceBuffer_;
}

void SurfaceMemory::ReleaseSurfaceBuffer()
{
    OSAL::ScopedLock l(memMutex_);
    if (surfaceBuffer_ != nullptr && surfaceAllocator_) {
        surfaceAllocator_->ReleaseSurfaceBuffer(surfaceBuffer_, needRender_);
    }
}

int32_t SurfaceMemory::GetFlushFence()
{
    OSAL::ScopedLock l(memMutex_);
    return fence_;
}

BufferHandle *SurfaceMemory::GetBufferHandle()
{
    OSAL::ScopedLock l(memMutex_);
    if (surfaceBuffer_) {
        return surfaceBuffer_->GetBufferHandle();
    }
    return nullptr;
}

void SurfaceMemory::SetNeedRender(bool needRender)
{
    OSAL::ScopedLock l(memMutex_);
    needRender_ = needRender;
}

uint32_t SurfaceMemory::GetSurfaceBufferStride()
{
    OSAL::ScopedLock l(memMutex_);
    return stride_;
}

uint8_t* SurfaceMemory::GetRealAddr() const
{
    OSAL::ScopedLock l(memMutex_);
    if (surfaceBuffer_) {
        return static_cast<uint8_t *>(surfaceBuffer_->GetVirAddr());
    }
    return nullptr;
}
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif