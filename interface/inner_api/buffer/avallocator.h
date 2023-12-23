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

#ifndef AV_ALLOCATOR_H
#define AV_ALLOCATOR_H

#ifndef MEDIA_NO_OHOS
#ifndef MEDIA_OHOS
#define MEDIA_OHOS
#endif
#else
#ifdef MEDIA_OHOS
#undef MEDIA_OHOS
#endif
#endif

#include <memory>
#include <string>
#include "refbase.h"
#include "buffer/avbuffer_common.h"

namespace OHOS {
struct BufferRequestConfig;
} // namespace OHOS
namespace OHOS {
namespace Media {
/**
 * @brief AVBuffer's allocator.
 */
class __attribute__((visibility("default"))) AVAllocator {
public:
    virtual ~AVAllocator() = default;
    /**
     * @brief Get the memory's type set by the creator, refer to {@link MemoryType}
     * @return the memory's type.
     * @since 4.1
     */
    virtual MemoryType GetMemoryType() = 0;

    /**
     * @brief Allocate a memory.
     * @param capacity The capacity of the memory to be allocated.
     * @return The pointer of the allocated buffer. When memory's type is {@link SHARED_MEMORY} returns the file
     * descriptor of allocated memory.
     * @since 4.1
     */
    virtual void *Alloc(int32_t capacity) = 0;

    /**
     * @brief Free a memory.
     * @param ptr The pointer of the allocated buffer. When memory's type is {@link SHARED_MEMORY} the parameter is the
     * file descriptor of allocated memory.
     * @return Whether the free was successful.
     * @since 4.1
     */
    virtual bool Free(void *ptr) = 0;

protected:
    explicit AVAllocator(){};
};

class __attribute__((visibility("default"))) AVAllocatorFactory {
public:
    /**
     * @brief Create the allocator of CPU buffer.
     * @return The allocator that allocate CPU buffer.
     * @since 4.1
     */
    static std::shared_ptr<AVAllocator> CreateVirtualAllocator();

    /**
     * @brief Create the allocator of shared memory.
     * @param memFlag Set the memory's flags, refer to {@link MemoryFlag}.
     * @return The allocator that allocate shared memory.
     * @since 4.1
     */
    static std::shared_ptr<AVAllocator> CreateSharedAllocator(MemoryFlag memFlag);

    /**
     * @brief Create the allocator of surface buffer, refer to {@link SurfaceBuffer}.
     * @param config Set the config of the surface buffer, refer to {@link BufferRequestConfig}.
     * @return The allocator that allocate surface buffer.
     * @since 4.1
     */
    static std::shared_ptr<AVAllocator> CreateSurfaceAllocator(const struct BufferRequestConfig &configs);

    /**
     * @brief Create the allocator of DMA buffer.
     * @param fd The file descriptor obtained from allocated DMA buffer.
     * @param capacity The capacity obtained from allocated DMA buffer.
     * @param memFlag Set the memory's flags, refer to {@link MemoryFlag}.
     * @return The allocator that allocate DMA buffer.
     * @since 4.1
     */
    static std::shared_ptr<AVAllocator> CreateHardwareAllocator(int32_t fd, int32_t capacity, MemoryFlag memFlag);

private:
    AVAllocatorFactory() = default;
    ~AVAllocatorFactory() = default;
};
} // namespace Media
} // namespace OHOS
#endif // AV_ALLOCATOR_H