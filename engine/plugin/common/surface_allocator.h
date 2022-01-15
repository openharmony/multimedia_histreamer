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

#ifndef HISTREAMER_PLUGIN_COMMON_SURFACE_ALLOCATOR_H
#define HISTREAMER_PLUGIN_COMMON_SURFACE_ALLOCATOR_H

#ifndef OHOS_LITE

#include "refbase.h"
#include "surface/surface.h"
#include "plugin_buffer.h"

namespace OHOS {
namespace Media {
namespace Plugin {

class SurfaceAllocator : public Allocator {
public:
    SurfaceAllocator(sptr<Surface> surface = nullptr);
    ~SurfaceAllocator() override = default;

    sptr<SurfaceBuffer> AllocSurfaceBuffer(size_t size);

    void FreeSurfaceBuffer(sptr<SurfaceBuffer> buffer);

    void* Alloc(size_t size) override;
    void Free(void* ptr) override; // NOLINT: void*

    void Config(int32_t width, int32_t height, int32_t usage, int32_t format, int32_t strideAlign);

private:
    sptr<Surface> surface_ {nullptr};
    BufferRequestConfig requestConfig_;
    uint32_t bufferCnt_ {0};
};
} // namespace Plugin
} // namespace Media
} // namespace OHOS

#endif
#endif // HISTREAMER_PLUGIN_COMMON_SURFACE_BUFFER_H
