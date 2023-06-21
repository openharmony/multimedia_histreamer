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

#define HST_LOG_TAG "SurfaceAllocator"

#include "plugin/common/surface_allocator.h"
#include "display_type.h"
#include "foundation/log.h"
#include "sync_fence.h"

namespace OHOS {
namespace Media {
namespace Plugin {
const std::unordered_map<VideoScaleType, ScalingMode> scaleTypeMap = {
    { VideoScaleType::VIDEO_SCALE_TYPE_FIT, ScalingMode::SCALING_MODE_SCALE_TO_WINDOW },
    { VideoScaleType::VIDEO_SCALE_TYPE_FIT_CROP, ScalingMode::SCALING_MODE_SCALE_CROP}
};

OHOS::ScalingMode GetScaleType(VideoScaleType scaleType)
{
    if (!scaleTypeMap.count(scaleType)) {
        return OHOS::SCALING_MODE_SCALE_TO_WINDOW;
    }
    return scaleTypeMap.at(scaleType);
}

constexpr int32_t DEFAULT_SURFACE_WIDTH = 640;
constexpr int32_t DEFAULT_SURFACE_HEIGHT = 480;
constexpr int32_t DEFAULT_SURFACE_STRIDE_ALIGN = 8;

SurfaceAllocator::SurfaceAllocator(sptr<Surface> surface)
    : Allocator(MemoryType::SURFACE_BUFFER),
      surface_(surface)
{
    requestConfig_ = {
        DEFAULT_SURFACE_WIDTH, DEFAULT_SURFACE_HEIGHT, DEFAULT_SURFACE_STRIDE_ALIGN,
        PixelFormat::PIXEL_FMT_RGBA_8888, BUFFER_USAGE_CPU_READ | BUFFER_USAGE_CPU_WRITE | BUFFER_USAGE_MEM_DMA, 0};
}

sptr<SurfaceBuffer> SurfaceAllocator::AllocSurfaceBuffer()
{
    if (!surface_) {
        MEDIA_LOG_E("surface is nullptr");
        return nullptr;
    }
    MEDIA_LOG_DD("width: " PUBLIC_LOG_D32 ", height :" PUBLIC_LOG_D32 ", align: " PUBLIC_LOG_D32
                 ", format: " PUBLIC_LOG_D32 ", usage: " PUBLIC_LOG_U64 ", timeout: " PUBLIC_LOG_D32,
                 requestConfig_.width, requestConfig_.height, requestConfig_.strideAlignment, requestConfig_.format,
                 requestConfig_.usage, requestConfig_.timeout);
    OHOS::sptr<OHOS::SurfaceBuffer> surfaceBuffer = nullptr;
    int32_t releaseFence = -1;
    auto ret = surface_->RequestBuffer(surfaceBuffer, releaseFence, requestConfig_);
    if (ret != OHOS::SurfaceError::SURFACE_ERROR_OK || surfaceBuffer == nullptr) {
        if (ret == OHOS::SurfaceError::SURFACE_ERROR_NO_BUFFER) {
            MEDIA_LOG_DD("buffer queue is no more buffers");
        } else {
            MEDIA_LOG_E("surface RequestBuffer fail, ret: " PUBLIC_LOG_U64, static_cast<uint64_t>(ret));
        }
        return nullptr;
    }
    if (surfaceBuffer->Map() != OHOS::SurfaceError::SURFACE_ERROR_OK) {
        MEDIA_LOG_E("surface buffer Map failed");
        surface_->CancelBuffer(surfaceBuffer);
        return nullptr;
    }
    sptr<SyncFence> autoFence = new(std::nothrow) SyncFence(releaseFence);
    if (autoFence != nullptr) {
        autoFence->Wait(100); // 100ms
    }
    surface_->SetScalingMode(surfaceBuffer->GetSeqNum(), scalingMode_);
    if (ret != OHOS::SurfaceError::SURFACE_ERROR_OK) {
        MEDIA_LOG_E("surface buffer set scaling mode failed");
        surface_->CancelBuffer(surfaceBuffer);
        return nullptr;
    }
    MEDIA_LOG_DD("request surface buffer success, releaseFence: " PUBLIC_LOG_D32, releaseFence);
    return surfaceBuffer;
}

void SurfaceAllocator::ReleaseSurfaceBuffer(sptr<SurfaceBuffer>& surfaceBuffer, bool needRender)
{
    if (!needRender) {
        auto ret = surface_->CancelBuffer(surfaceBuffer);
        if (ret != OHOS::SurfaceError::SURFACE_ERROR_OK) {
            MEDIA_LOG_E("surface CancelBuffer fail, ret: " PUBLIC_LOG_U64, static_cast<uint64_t>(ret));
        }
    }
    surfaceBuffer = nullptr;
}

void* SurfaceAllocator::Alloc(size_t size)
{
    return nullptr;
}

void SurfaceAllocator::Free(void* ptr) // NOLINT: void*
{
    (void)ptr;
}

void SurfaceAllocator::Config(int32_t width, int32_t height, uint64_t usage, int32_t format, int32_t strideAlign,
                              int32_t timeout)
{
    requestConfig_ = {
        width, height, strideAlign, format, usage, timeout
    };
}

void SurfaceAllocator::SetScaleType(VideoScaleType videoScaleType)
{
    scalingMode_ = GetScaleType(videoScaleType);
}

void SurfaceAllocator::UpdateSurfaceBufferScaleMode(sptr<SurfaceBuffer>& surfaceBuffer)
{
    auto ret = surface_->SetScalingMode(surfaceBuffer->GetSeqNum(), scalingMode_);
    if (ret != OHOS::SurfaceError::SURFACE_ERROR_OK) {
        MEDIA_LOG_E("update surface buffer scaling mode fail, ret: " PUBLIC_LOG_U64, static_cast<uint64_t>(ret));
    }
}
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif