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

#ifndef AVBUFFER_COMMON_H
#define AVBUFFER_COMMON_H

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
#include "meta/meta.h"

namespace OHOS {
class SurfaceBuffer;
class MessageParcel;
struct BufferRequestConfig;
} // namespace OHOS

namespace OHOS {
namespace Media {
class AVMemory;
constexpr int32_t INVALID_POSITION = -1;

/**
 * @enum MemoryType
 * @brief For platforms that support multiple processes, this flag bit indicates the types of data storage, refer to
 * {@link AVAllocator}.
 * @since 4.1
 * @version 1.0
 */
enum struct MemoryType : uint8_t {
    /**
     * If this type is not set, the allocator will be initialized by the current type by default. This type of memory is
     * created through malloc() and can only be used by the current process.
     */
    VIRTUAL_MEMORY = 0,
    /**
     * A memory type that implements a convenient memory sharing mechanism. For platforms that do not support
     * multiprocessing, it may only encapsulate ordinary memory blocks rather than truly multiprocess shared memory
     */
    SHARED_MEMORY,
    /**
     * A memory type that provides surface buffer for sharing multi process data.
     */
    SURFACE_MEMORY,
    /**
     * A memory type that provides DMA method for sharing multi process data. If the hardware does not support it, it
     * will be invalid when initializing AVAlocator.
     */
    HARDWARE_MEMORY,
    /**
     * The identifier for buffer queue, representing any type of memory can be allocated.
     */
    UNKNOWN_MEMORY
};

/**
 * @brief Enumerates the flag bits used to create a new shared memory.
 */
enum MemoryFlag : uint8_t {
    /**
     * For platforms that support multiple processes, this flag bit indicates that the remote process can only read data
     * in the shared memory. If this flag is not set, the remote process has both read and write permissions by default.
     * Adding this flag does not affect the process that creates the memory, which always has the read and write
     * permission on the shared memory. For platforms that do not support multi-processes, the memory read and write
     * permission control capability may not be available. In this case, this flag is invalid.
     */
    MEMORY_READ_ONLY = 0x1 << 0,
    /**
     * For platforms that support multiple processes, this flag bit indicates that the remote process can only write
     * data in the shared memory.
     */
    MEMORY_WRITE_ONLY = 0x1 << 1,
    /**
     * This flag bit indicates that the remote process is allowed to read and write the shared memory. If no flags are
     * specified, this is the default memory sharing policy. If the FLAGS_READ_ONLY bit is set, this flag bit is
     * ignored.
     */
    MEMORY_READ_WRITE = MEMORY_READ_ONLY | MEMORY_WRITE_ONLY,
};

/**
 * @brief Struct that encapsulates some info of media buffer.
 */
using AVBufferConfig = struct AVBufferConfig {
    int32_t size = 0;
    int32_t align = 0;
    MemoryType memoryType = MemoryType::UNKNOWN_MEMORY;
    MemoryFlag memoryFlag = MemoryFlag::MEMORY_READ_WRITE;
    std::unique_ptr<struct BufferRequestConfig> surfaceBufferConfig;
    int32_t dmaFd = -1;   // to create dma buffer
    int32_t capacity = 0; // get from buffer

    AVBufferConfig();
    AVBufferConfig(const AVBufferConfig &rhs);
    AVBufferConfig(AVBufferConfig &&rhs) noexcept;
    AVBufferConfig &operator=(const AVBufferConfig &rhs);
    AVBufferConfig &operator=(AVBufferConfig &&rhs) noexcept;
    bool operator<=(const struct AVBufferConfig &rhs) const;
};
} // namespace Media
} // namespace OHOS
#endif // AVBUFFER_COMMON_H