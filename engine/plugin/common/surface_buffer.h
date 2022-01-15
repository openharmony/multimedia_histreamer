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

#ifndef HISTREAMER_PLUGIN_COMMON_SURFACE_BUFFER_H
#define HISTREAMER_PLUGIN_COMMON_SURFACE_BUFFER_H

#ifndef OHOS_LITE

#include "refbase.h"
#include "surface/surface.h"
#include "plugin_buffer.h"

namespace OHOS {
namespace Media {
namespace Plugin {
class SurfaceMemory : public Memory {
public:
    sptr<SurfaceBuffer> GetSurfaceBuffer();

private:
    SurfaceMemory(size_t capacity, std::shared_ptr<Allocator> allocator = nullptr, size_t align = 1);

    uint8_t *GetRealAddr() const override;

private:
    sptr<SurfaceBuffer> surfaceBuffer;

    /// the fence fd for Surface
    int32_t fenceFd {-1};

    /// the buffer handle for SurfaceBuffer
    intptr_t handle {0};

    friend class Buffer;
};
} // namespace Plugin
} // namespace Media
} // namespace OHOS

#endif
#endif // HISTREAMER_PLUGIN_COMMON_SURFACE_BUFFER_H
