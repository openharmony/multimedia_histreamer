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

#ifndef OHOS_MEDIA_AV_SHARED_MEMORY_H
#define OHOS_MEDIA_AV_SHARED_MEMORY_H

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

namespace OHOS {
namespace Media {
/**
 * @brief Provides a unified interface to implement convenient memory sharing
 * mechanism. For those platforms that do not support multi-process, it may
 * simply encapsulate ordinary memory blocks, not really multi-process shareable memory.
 */
class __attribute__((visibility("default"))) AVSharedMemory {
public:
    virtual ~AVSharedMemory() = default;

    /**
     * @brief Enumerates the flag bits used to create a new shared memory.
     */
    enum Flags : uint32_t {
        /**
         * This flag bit indicates that the remote process is allowed to read and write
         * the shared memory. If no flags are specified, this is the default memory
         * sharing policy. If the FLAGS_READ_ONLY bit is set, this flag bit is ignored.
         */
        FLAGS_READ_WRITE = 0x1,
        /**
         * For platforms that support multiple processes, this flag bit indicates that the
         * remote process can only read data in the shared memory. If this flag is not set,
         * the remote process has both read and write permissions by default. Adding this
         * flag does not affect the process that creates the memory, which always has the
         * read and write permission on the shared memory. For platforms that do not support
         * multi-processes, the memory read and write permission control capability may
         * not be available. In this case, this flag is invalid.
         */
        FLAGS_READ_ONLY = 0x2,
    };

    /**
     * @brief Get the memory's virtual address
     * @return the memory's virtual address if the memory is valid, otherwise nullptr.
     */
    virtual uint8_t *GetBase() const = 0;

    /**
     * @brief Get the memory's size
     * @return the memory's size if the memory is valid, otherwise -1.
     */
    virtual int32_t GetSize() const = 0;

    /**
     * @brief Get the memory's flags set by the creator, refer to {@Flags}
     * @return the memory's flags if the memory is valid, otherwise 0.
     */
    virtual uint32_t GetFlags() const = 0;
};
} // namespace Media
} // namespace OHOS
#endif // AVSHAREDMEMORY_H