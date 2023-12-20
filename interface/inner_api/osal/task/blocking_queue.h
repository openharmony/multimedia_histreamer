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
#include <utility>
#include "common/log.h"
#include "osal/task/condition_variable.h"
#include "osal/task/mutex.h"
#include "osal/task/autolock.h"

namespace OHOS {
namespace Media {
template <typename T>
class BlockingQueue {
public:
    explicit BlockingQueue(std::string name, size_t capacity = 10) // 10 means default queue size
        : name_(std::move(name)), capacity_(capacity), isActive(true)
    {
    }
    ~BlockingQueue() = default;
    size_t Size()
    {
        AutoLock lock(mutex_);
        return que_.size();
    }
    size_t Capacity()
    {
        return capacity_;
    }
    size_t Empty()
    {
        AutoLock lock(mutex_);
        return que_.empty();
    }
    bool Push(const T& value)
    {
        AutoLock lock(mutex_);
        if (!isActive) {
            MEDIA_LOG_DD("blocking queue " PUBLIC_LOG_S " is inactive for Push.", name_.c_str());
            return false;
        }
        if (que_.size() >= capacity_) {
            MEDIA_LOG_DD("blocking queue " PUBLIC_LOG_S " is full, waiting for pop.", name_.c_str());
            cvFull_.Wait(lock, [this] { return !isActive || que_.size() < capacity_; });
        }
        if (!isActive) {
            MEDIA_LOG_D("blocking queue " PUBLIC_LOG_S ": inactive: " PUBLIC_LOG_D32 ", isFull: " PUBLIC_LOG
                        "d", name_.c_str(), isActive.load(), que_.size() < capacity_);
            return false;
        }
        que_.push(value);
        cvEmpty_.NotifyAll();
        MEDIA_LOG_DD("blocking queue " PUBLIC_LOG_S " Push succeed.", name_.c_str());
        return true;
    }
    bool Push(const T& value, int timeoutMs)
    {
        AutoLock lock(mutex_);
        if (!isActive) {
            MEDIA_LOG_D("blocking queue " PUBLIC_LOG_S " is inactive for Push.", name_.c_str());
            return false;
        }
        if (que_.size() >= capacity_) {
            MEDIA_LOG_D("blocking queue is full, waiting for pop...");
            cvFull_.WaitFor(lock, timeoutMs, [this] { return !isActive || que_.size() < capacity_; });
        }
        if (!isActive || (que_.size() == capacity_)) {
            MEDIA_LOG_D("blocking queue: inactive: " PUBLIC_LOG_D32 ", isFull: " PUBLIC_LOG_D32,
                        isActive.load(), que_.size() < capacity_);
            return false;
        }
        que_.push(value);
        cvEmpty_.NotifyAll();
        return true;
    }
    T Pop()
    {
        MEDIA_LOG_DD("blocking queue " PUBLIC_LOG_S " Pop enter.", name_.c_str());
        AutoLock lock(mutex_);
        if (!isActive) {
            MEDIA_LOG_D("blocking queue " PUBLIC_LOG_S " is inactive.", name_.c_str());
            return {};
        }
        if (que_.empty()) {
            MEDIA_LOG_DD("blocking queue " PUBLIC_LOG_S " is empty, waiting for push", name_.c_str());
            cvEmpty_.Wait(lock, [this] { return !isActive || !que_.empty(); });
        }
        if (!isActive) {
            return {};
        }
        T el = que_.front();
        que_.pop();
        cvFull_.NotifyOne();
        MEDIA_LOG_DD("blocking queue " PUBLIC_LOG_S " Pop succeed.", name_.c_str());
        return el;
    }
    T Pop(int timeoutMs)
    {
        AutoLock lock(mutex_);
        if (!isActive) {
            MEDIA_LOG_D("blocking queue " PUBLIC_LOG_S " is inactive.", name_.c_str());
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
        AutoLock lock(mutex_);
        ClearUnprotected();
    }
    void SetActive(bool active, bool cleanData = true)
    {
        AutoLock lock(mutex_);
        MEDIA_LOG_D("SetActive for " PUBLIC_LOG_S ": " PUBLIC_LOG_D32 ".", name_.c_str(), active);
        isActive = active;
        if (!active) {
            if (cleanData) {
                ClearUnprotected();
            }
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

    Mutex mutex_;
    ConditionVariable cvFull_;
    ConditionVariable cvEmpty_;

    std::string name_;
    std::queue<T> que_;
    const size_t capacity_;
    std::atomic<bool> isActive;
};
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_FOUNDATION_BLOCKING_QUEUE_H
