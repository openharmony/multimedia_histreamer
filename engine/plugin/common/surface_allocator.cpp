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
#ifndef OHOS_LITE
#include "surface_allocator.h"
#include "foundation/log.h"
#include "display_type.h"

namespace OHOS {
namespace Media {
namespace Plugin {
SurfaceAllocator::SurfaceAllocator(sptr<Surface> surface) : Allocator(MemoryType::SURFACE_BUFFER)
{
    surface_ = surface;
}

sptr<SurfaceBuffer> SurfaceAllocator::AllocSurfaceBuffer(size_t size)
{
    OHOS::sptr<OHOS::SurfaceBuffer> surfaceBuffer = nullptr;
    int32_t fence = -1;
    auto ret = surface_->RequestBuffer(surfaceBuffer, fence, requestConfig_);
    if (ret != OHOS::SurfaceError::SURFACE_ERROR_OK) {
        if (ret == OHOS::SurfaceError::SURFACE_ERROR_NO_BUFFER) {
            MEDIA_LOG_E("buffer queue is no more buffers");
        } else {
            MEDIA_LOG_E("surface RequestBuffer fail");
        }
        return nullptr;
    }
    return surfaceBuffer;
}

void SurfaceAllocator::FreeSurfaceBuffer(sptr<SurfaceBuffer> buffer)
{
    auto ret = surface_->CancelBuffer(buffer);
    if (ret != OHOS::SurfaceError::SURFACE_ERROR_OK) {
        MEDIA_LOG_E("surface CancelBuffer fail");
    }
}

void* SurfaceAllocator::Alloc(size_t size)
{
    return nullptr;
}

void SurfaceAllocator::Free(void* ptr) // NOLINT: void*
{
    (void)ptr;
}

void SurfaceAllocator::Config(int32_t width, int32_t height, int32_t usage, int32_t format, int32_t strideAlign)
{
    requestConfig_ = {
            width, height, strideAlign, format,
            usage | HBM_USE_CPU_READ | HBM_USE_CPU_WRITE | HBM_USE_MEM_DMA, 0
    };
    bufferCnt_ = 0;
}
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif