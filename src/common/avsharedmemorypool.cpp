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

#include "buffer/avsharedmemorybase.h"
#include "common/avsharedmemorypool.h"
#include "common/log.h"
#include "scope_guard.h"

namespace {
    constexpr int32_t MAX_MEM_SIZE = 100 * 1024 * 1024;
}

namespace OHOS {
namespace Media {
AVSharedMemoryPool::AVSharedMemoryPool(const std::string &name) : name_(name)
{
    MEDIA_LOG_D("enter ctor, 0x%{public}06" PRIXPTR ", name: %{public}s", FAKE_POINTER(this), name_.c_str());
}

AVSharedMemoryPool::~AVSharedMemoryPool()
{
    MEDIA_LOG_D("enter dtor, 0x%{public}06" PRIXPTR ", name: %{public}s", FAKE_POINTER(this), name_.c_str());
    Reset();
}

int32_t AVSharedMemoryPool::Init(const InitializeOption &option)
{
    std::unique_lock<std::mutex> lock(mutex_);

    FALSE_RETURN_V_MSG_E(!inited_, -1, "memory pool has inited.");
    FALSE_RETURN_V_MSG_E(option.memSize < MAX_MEM_SIZE, -1, "memSize is larger than MAX_MEM_SIZE.");
    FALSE_RETURN_V_MSG_E(option.maxMemCnt != 0, -1, "maxMemCnt is equal to zero.");

    option_ = option;
    option_.preAllocMemCnt = std::min(option.preAllocMemCnt, option.maxMemCnt);

    MEDIA_LOG_I("name: %{public}s, init option: preAllocMemCnt = %{public}u, memSize = %{public}d, "
        "maxMemCnt = %{public}u, enableFixedSize = %{public}d",
        name_.c_str(), option_.preAllocMemCnt, option_.memSize, option_.maxMemCnt,
        option_.enableFixedSize);
    for (uint32_t i = 0; i < option_.preAllocMemCnt; ++i) {
        auto memory = AllocMemory(option_.memSize);
        FALSE_RETURN_V_MSG_E(memory != nullptr, -1, "failed to AllocMemory");
        idleList_.push_back(memory);
    }

    inited_ = true;
    notifier_ = option.notifier;
    return 0;
}

AVSharedMemory *AVSharedMemoryPool::AllocMemory(int32_t size)
{
    AVSharedMemoryBase *memory = new (std::nothrow) AVSharedMemoryBase(size, option_.flags, name_);
    FALSE_RETURN_V_MSG_E(memory != nullptr, nullptr, "create object failed");
    ON_SCOPE_EXIT(0) { delete memory; };
    memory->Init();
    int32_t ret = memory->Init();
    FALSE_RETURN_V_MSG_E(ret == 0, nullptr, "init avsharedmemorybase failed");

    CANCEL_SCOPE_EXIT_GUARD(0);
    return memory;
}

void AVSharedMemoryPool::ReleaseMemory(AVSharedMemory *memory)
{
    FALSE_LOG_MSG(memory != nullptr, "memory is nullptr");
    std::unique_lock<std::mutex> lock(mutex_);

    for (auto iter = busyList_.begin(); iter != busyList_.end(); ++iter) {
        if (*iter != memory) {
            continue;
        }

        busyList_.erase(iter);
        idleList_.push_back(memory);
        cond_.notify_all();
        MEDIA_LOG_D("0x%{public}06" PRIXPTR " released back to pool %{public}s",
                    FAKE_POINTER(memory), name_.c_str());

        lock.unlock();
        if (notifier_ != nullptr) {
            notifier_();
        }
        return;
    }

    MEDIA_LOG_E("0x%{public}06" PRIXPTR " is no longer managed by this pool", FAKE_POINTER(memory));
    delete memory;
}

bool AVSharedMemoryPool::DoAcquireMemory(int32_t size, AVSharedMemory **outMemory)
{
    MEDIA_LOG_D("busylist size " PUBLIC_LOG_ZU ", idlelist size " PUBLIC_LOG_ZU, busyList_.size(), idleList_.size());

    AVSharedMemory *result = nullptr;
    std::list<AVSharedMemory *>::iterator minSizeIdleMem = idleList_.end();
    int32_t minIdleSize = std::numeric_limits<int32_t>::max();

    for (auto iter = idleList_.begin(); iter != idleList_.end(); ++iter) {
        if ((*iter)->GetSize() >= size) {
            result = *iter;
            idleList_.erase(iter);
            break;
        }

        if ((*iter)->GetSize() < minIdleSize) {
            minIdleSize = (*iter)->GetSize();
            minSizeIdleMem = iter;
        }
    }

    if (result == nullptr) {
        auto totalCnt = busyList_.size() + idleList_.size();
        if (totalCnt < option_.maxMemCnt) {
            result = AllocMemory(size);
            FALSE_RETURN_V_MSG_E(result != nullptr, false, "result is nullptr");
            FALSE_RETURN_V(result != nullptr, false);
        }

        if (!option_.enableFixedSize && minSizeIdleMem != idleList_.end()) {
            delete *minSizeIdleMem;
            *minSizeIdleMem = nullptr;
            idleList_.erase(minSizeIdleMem);
            result = AllocMemory(size);
            FALSE_RETURN_V_MSG_E(result != nullptr, false, "result is nullptr");
            FALSE_RETURN_V(result != nullptr, false);
        }
    }

    *outMemory = result;
    return true;
}

bool AVSharedMemoryPool::CheckSize(int32_t size)
{
    if (size <= 0 && size != -1) {
        return false;
    }

    if (!option_.enableFixedSize && size == -1) {
        return false;
    }

    if (option_.enableFixedSize) {
        if (size > option_.memSize) {
            return false;
        }

        if (size <= 0 && size != -1) {
            return false;
        }
    }

    return true;
}

std::shared_ptr<AVSharedMemory> AVSharedMemoryPool::AcquireMemory(int32_t size, bool blocking)
{
    MEDIA_LOG_D("acquire memory for size: %{public}d from pool %{public}s, blocking: %{public}d",
        size, name_.c_str(), blocking);

    std::unique_lock<std::mutex> lock(mutex_);
    FALSE_RETURN_V_MSG_E(CheckSize(size), nullptr, "invalid size: %{public}d", size);

    if (option_.enableFixedSize) {
        size = option_.memSize;
    }

    AVSharedMemory *memory = nullptr;
    do {
        if (!DoAcquireMemory(size, &memory) || memory != nullptr) {
            break;
        }

        if (!blocking || forceNonBlocking_) {
            break;
        }

        cond_.wait(lock);
    } while (inited_ && !forceNonBlocking_);

    FALSE_RETURN_V_MSG_E(memory != nullptr, nullptr, "acquire memory failed for size: %{public}d", size);
    busyList_.push_back(memory);

    auto result = std::shared_ptr<AVSharedMemory>(memory, [weakPool = weak_from_this()](AVSharedMemory *memory) {
        std::shared_ptr<AVSharedMemoryPool> pool = weakPool.lock();
        if (pool != nullptr) {
            pool->ReleaseMemory(memory);
        } else {
            MEDIA_LOG_I("release memory 0x%{public}06" PRIXPTR ", but the pool is destroyed", FAKE_POINTER(memory));
            delete memory;
        }
    });

    MEDIA_LOG_D("0x%{public}06" PRIXPTR " acquired from pool", FAKE_POINTER(memory));
    return result;
}

void AVSharedMemoryPool::SetNonBlocking(bool enable)
{
    std::unique_lock<std::mutex> lock(mutex_);
    MEDIA_LOG_D("SetNonBlocking: %{public}d", enable);
    forceNonBlocking_ = enable;
    if (forceNonBlocking_) {
        cond_.notify_all();
    }
}

void AVSharedMemoryPool::Reset()
{
    MEDIA_LOG_D("Reset");

    std::unique_lock<std::mutex> lock(mutex_);
    for (auto &memory : idleList_) {
        delete memory;
        memory = nullptr;
    }
    idleList_.clear();
    inited_ = false;
    forceNonBlocking_ = false;
    notifier_ = nullptr;
    cond_.notify_all();
    // for busylist, the memory will be released when the refcount of shared_ptr is zero.
}
} // namespace Media
} // namespace OHOS