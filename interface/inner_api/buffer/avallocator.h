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

#include <memory>
#include <stdlib.h>
#include <string>
#include "refbase.h"
#include "surface_type.h" // foundation/graphic/graphic_2d/interfaces/inner_api/surface/surface_type.h

namespace OHOS {
class SurfaceBuffer;
class MessageParcel;
} // namespace OHOS
namespace OHOS {
namespace MediaAVCodec {
class AVMemory;
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
enum MemoryFlag : uint32_t {
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
 * @brief AVBuffer's allocator.
 */
class AVAllocator {
public:
    virtual ~AVAllocator() = default;
    /**
     * @brief Get the memory's type set by the creator, refer to {@link MemoryType}
     * @return the memory's type.
     * @since 4.1
     * @version 1.0
     */
    virtual MemoryType GetMemoryType() = 0;

    /**
     * @brief Allocate a memory.
     * @param capacity The capacity of the memory to be allocated.
     * @return The pointer of the allocated buffer. When memory's type is {@link SHARED_MEMORY} returns the file
     * descriptor of allocated memory.
     * @since 4.1
     * @version 1.0
     */
    virtual void *Alloc(int32_t capacity) = 0;

    /**
     * @brief Free a memory.
     * @param ptr The pointer of the allocated buffer. When memory's type is {@link SHARED_MEMORY} the parameter is the
     * file descriptor of allocated memory.
     * @return Whether the free was successful.
     * @since 4.1
     * @version 1.0
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
     * @version 1.0
     */
    static std::shared_ptr<AVAllocator> CreateVirtualAllocator();

    /**
     * @brief Create the allocator of shared memory.
     * @param memFlag Set the memory's flags, refer to {@link MemoryFlag}.
     * @return The allocator that allocate shared memory.
     * @since 4.1
     * @version 1.0
     */
    static std::shared_ptr<AVAllocator> CreateSharedAllocator(MemoryFlag memFlag);

    /**
     * @brief Create the allocator of surface buffer, refer to {@link SurfaceBuffer}.
     * @param config Set the config of the surface buffer, refer to {@link BufferRequestConfig}.
     * @return The allocator that allocate surface buffer.
     * @since 4.1
     * @version 1.0
     */
    static std::shared_ptr<AVAllocator> CreateSurfaceAllocator(const BufferRequestConfig &configs);

    /**
     * @brief Create the allocator of DMA buffer.
     * @param fd The file descriptor obtained from allocated DMA buffer.
     * @param capacity The capacity obtained from allocated DMA buffer.
     * @param memFlag Set the memory's flags, refer to {@link MemoryFlag}.
     * @return The allocator that allocate DMA buffer.
     * @since 4.1
     * @version 1.0
     */
    static std::shared_ptr<AVAllocator> CreateHardwareAllocator(int32_t fd, int32_t capacity, MemoryFlag memFlag);

private:
    AVAllocatorFactory() = default;
    ~AVAllocatorFactory() = default;
};
} // namespace MediaAVCodec
} // namespace OHOS
#endif // AV_ALLOCATOR_H