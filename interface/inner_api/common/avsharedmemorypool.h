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

#ifndef AVSHAREDMEMORYPOOL_H
#define AVSHAREDMEMORYPOOL_H

#include <condition_variable>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include "nocopyable.h"
#include "buffer/avsharedmemorybase.h"

namespace OHOS {
namespace Media {
/**
 * @brief A simple pool implementation for shared memory.
 *
 * This pool support multi configuration:
 * @preAllocMemCnt: The number of memory blocks allocated when the pool is initialized.
 * @memSize: the size of the preallocated memory blocks.
 * @maxMemCnt: the total number of memory blocks in the pool.
 * @flags: the shared memory access property, refer to {@AVSharedMemory::Flags}.
 * @enableFixedSize: if true, the pool will allocate all memory block using the memSize. If the acquired
 *                  size is larger than the memSize, the acquire will failed. If false, the pool will
 *                  free the smallest idle memory block when there is no idle memory block that can
 *                  satisfy the acqiured size and reallocate a new memory block with the acquired size.
 * @notifier: the callback will be called to notify there are any available memory. It will be useful for
 *            non-blocking memory acquisition.
 */
class AVSharedMemoryPool : public std::enable_shared_from_this<AVSharedMemoryPool>, public NoCopyable {
public:
    explicit AVSharedMemoryPool(const std::string &name);
    ~AVSharedMemoryPool();

    using MemoryAvailableNotifier = std::function<void(void)>;

    struct InitializeOption {
        uint32_t preAllocMemCnt = 0;
        int32_t memSize = 0;
        uint32_t maxMemCnt = 0;
        uint32_t flags = AVSharedMemory::Flags::FLAGS_READ_WRITE;
        bool enableFixedSize = true;
        MemoryAvailableNotifier notifier;
    };

    /**
     * @brief Initialize the pool and preallocate some memory blocks.
     */
    int32_t Init(const InitializeOption &option);

    /**
     * @brief Get a memory from the pool and optional to wait for a memory to be release when there
     * are no memories available.
     *
     * @param size the expected memory size. if the enableFixedSize is configured, this param can be empty.
     * @return valid memory block if success, or nullptr.
     */
    std::shared_ptr<AVSharedMemory> AcquireMemory(int32_t size = -1, bool blocking = true);

    /**
     * @brief Set or Unset the pool to be non-blocking memory pool. If enable, the AcquireMemory will always
     * be non-blocking and the waiters will be returned with null memory.
     */
    void SetNonBlocking(bool enable);

    /**
     * @brief Free all memory blocks and reset the pool configuration. After reseted, all waiters of
     * the pool will be awaken up and returned with a nullptr.
     */
    void Reset();

    std::string GetName()
    {
        return name_;
    }

private:
    bool DoAcquireMemory(int32_t size, AVSharedMemory **outMemory);
    AVSharedMemory *AllocMemory(int32_t size);
    void ReleaseMemory(AVSharedMemory *memory);
    bool CheckSize(int32_t size);

    InitializeOption option_ {};
    std::list<AVSharedMemory *> idleList_;
    std::list<AVSharedMemory *> busyList_;
    std::mutex mutex_;
    std::condition_variable cond_;
    bool inited_ = false;
    std::string name_;
    MemoryAvailableNotifier notifier_;
    bool forceNonBlocking_ = false;
};
} // namespace Media
} // namespace OHOS

#endif