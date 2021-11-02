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

#ifndef HISTREAMER_FOUNDATION_BLOCKING_QUEUE_H
#define HISTREAMER_FOUNDATION_BLOCKING_QUEUE_H

#include <atomic>
#include <queue>
#include <string>
#include "constants.h"
#include "foundation/log.h"
#include "osal/thread/condition_variable.h"
#include "osal/thread/mutex.h"
#include "osal/thread/scoped_lock.h"

namespace OHOS {
namespace Media {
template <typename T>
class BlockingQueue {
public:
    explicit BlockingQueue(const std::string& name, size_t capacity = DEFAULT_QUEUE_SIZE)
        : name_(name), capacity_(capacity), isActive(true)
    {
    }
    ~BlockingQueue() = default;
    size_t Size()
    {
        OSAL::ScopedLock lock(mutex_);
        return que_.size();
    }
    size_t Capacity()
    {
        return capacity_;
    }
    size_t Empty()
    {
        OSAL::ScopedLock lock(mutex_);
        return que_.empty();
    }
    bool Push(const T& value)
    {
        OSAL::ScopedLock lock(mutex_);
        if (!isActive) {
            MEDIA_LOG_D("blocking queue %s is inactive for Push.", name_.c_str());
            return false;
        }
        if (que_.size() >= capacity_) {
            MEDIA_LOG_D("blocking queue %s is full, waiting for pop.", name_.c_str());
            cvFull_.Wait(lock, [this] { return !isActive || que_.size() < capacity_; });
        }
        if (!isActive) {
            MEDIA_LOG_D("blocking queue %s: inactive: %d, isFull: %d", name_.c_str(), isActive.load(),
                        que_.size() < capacity_);
            return false;
        }
        que_.push(value);
        cvEmpty_.NotifyAll();
        MEDIA_LOG_D("blocking queue %s Push succeed.", name_.c_str());
        return true;
    }
    bool Push(const T& value, int timeoutMs)
    {
        OSAL::ScopedLock lock(mutex_);
        if (!isActive) {
            MEDIA_LOG_D("blocking queue %s is inactive for Push.", name_.c_str());
            return false;
        }
        if (que_.size() >= capacity_) {
            MEDIA_LOG_D("blocking queue is full, waiting for pop...");
            cvFull_.WaitFor(lock, timeoutMs, [this] { return !isActive || que_.size() < capacity_; });
        }
        if (!isActive || (que_.size() == capacity_)) {
            MEDIA_LOG_D("blocking queue: inactive: %d, isFull: %d", isActive, que_.size() < capacity_);
            return false;
        }
        que_.push(value);
        cvEmpty_.NotifyAll();
        return true;
    }
    T Pop()
    {
        MEDIA_LOG_D("blocking queue %s Pop enter.", name_.c_str());
        OSAL::ScopedLock lock(mutex_);
        if (!isActive) {
            MEDIA_LOG_D("blocking queue %s is inactive.", name_.c_str());
            return {};
        }
        if (que_.empty()) {
            MEDIA_LOG_D("blocking queue %s is empty, waiting for push", name_.c_str());
            cvEmpty_.Wait(lock, [this] { return !isActive || !que_.empty(); });
        }
        if (!isActive) {
            return {};
        }
        T el = que_.front();
        que_.pop();
        cvFull_.NotifyOne();
        MEDIA_LOG_D("blocking queue %s Pop succeed.", name_.c_str());
        return el;
    }
    T Pop(int timeoutMs)
    {
        OSAL::ScopedLock lock(mutex_);
        if (!isActive) {
            MEDIA_LOG_D("blocking queue %s is inactive.", name_.c_str());
            return {};
        }
        if (que_.empty()) {
            cvEmpty_.WaitFor(lock, timeoutMs, [this] { return !isActive || !que_.empty(); });
        }
        if (!isActive || que_.empty()) {
            return {};
        }
        T el = que_.front();
        que_.pop();
        cvFull_.NotifyOne();
        return el;
    }
    void Clear()
    {
        OSAL::ScopedLock lock(mutex_);
        ClearUnprotected();
    }
    void SetActive(bool active)
    {
        OSAL::ScopedLock lock(mutex_);
        MEDIA_LOG_D("SetActive for %s: %d.", name_.c_str(), active);
        isActive = active;
        if (!active) {
            ClearUnprotected();
            cvEmpty_.NotifyOne();
        }
    }

private:
    void ClearUnprotected()
    {
        if (que_.empty()) {
            return;
        }
        bool needNotify = que_.size() == capacity_;
        std::queue<T>().swap(que_);
        if (needNotify) {
            cvFull_.NotifyOne();
        }
    }

    OSAL::Mutex mutex_;
    OSAL::ConditionVariable cvFull_;
    OSAL::ConditionVariable cvEmpty_;

    std::string name_;
    std::queue<T> que_;
    const size_t capacity_;
    std::atomic<bool> isActive;
};
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_FOUNDATION_BLOCKING_QUEUE_H
