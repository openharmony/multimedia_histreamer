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

#ifndef HISTREAMER_FOUNDATION_OSAL_BASE_SYNCHRONIZER_H
#define HISTREAMER_FOUNDATION_OSAL_BASE_SYNCHRONIZER_H

#include <map>
#include <set>
#include <string>

#include "foundation/log.h"
#include "thread/condition_variable.h"
#include "thread/mutex.h"

namespace OHOS {
namespace Media {
namespace OSAL {
template<typename SyncIdType, typename ResultType = void>
class Synchronizer {
public:
    explicit Synchronizer(std::string name) : name_(std::move(name))
    {
    }

    Synchronizer(const Synchronizer<SyncIdType, ResultType>&) = delete;

    Synchronizer<SyncIdType, ResultType>& operator=(const Synchronizer<SyncIdType, ResultType>&) = delete;

    virtual ~Synchronizer() = default;

    void Wait(SyncIdType syncId)
    {
        MEDIA_LOG_I("Synchronizer %s Wait for %d", name_.c_str(), static_cast<int>(syncId));
        OSAL::ScopedLock lock(mutex_);
        waitSet_.insert(syncId);
        cv_.Wait(lock, [this, syncId] { return syncMap_.find(syncId) != syncMap_.end(); });
        syncMap_.erase(syncId);
    }

    bool WaitFor(SyncIdType syncId, int timeoutMs)
    {
        MEDIA_LOG_I("Synchronizer %s Wait for %d, timeout: %d", name_.c_str(), static_cast<int>(syncId), timeoutMs);
        OSAL::ScopedLock lock(mutex_);
        waitSet_.insert(syncId);
        auto rtv = cv_.WaitFor(lock, timeoutMs, [this, syncId] { return syncMap_.find(syncId) != syncMap_.end(); });
        if (rtv) {
            syncMap_.erase(syncId);
        } else {
            waitSet_.erase(syncId);
        }
        return rtv;
    }

    void Wait(SyncIdType syncId, ResultType& result)
    {
        MEDIA_LOG_I("Synchronizer %s Wait for %d", name_.c_str(), static_cast<int>(syncId));
        OSAL::ScopedLock lock(mutex_);
        waitSet_.insert(syncId);
        cv_.Wait(lock, [this, syncId] { return syncMap_.find(syncId) != syncMap_.end(); });
        result = syncMap_[syncId];
        syncMap_.erase(syncId);
    }

    bool WaitFor(SyncIdType syncId, int timeoutMs, ResultType& result)
    {
        MEDIA_LOG_I("Synchronizer %s Wait for %d, timeout: %d", name_.c_str(), static_cast<int>(syncId), timeoutMs);
        OSAL::ScopedLock lock(mutex_);
        waitSet_.insert(syncId);
        auto rtv = cv_.WaitFor(lock, timeoutMs, [this, syncId] { return syncMap_.find(syncId) != syncMap_.end(); });
        if (rtv) {
            result = syncMap_[syncId];
            syncMap_.erase(syncId);
        } else {
            waitSet_.erase(syncId);
        }
        return rtv;
    }

    void Notify(SyncIdType syncId, ResultType result = ResultType())
    {
        MEDIA_LOG_I("Synchronizer %s Notify: %d", name_.c_str(), static_cast<int>(syncId));
        OSAL::ScopedLock lock(mutex_);
        if (waitSet_.find(syncId) != waitSet_.end()) {
            waitSet_.erase(syncId);
            syncMap_.insert({syncId, result});
            cv_.NotifyAll();
        }
    }

private:
    Mutex mutex_;
    ConditionVariable cv_;
    std::string name_;
    std::map<SyncIdType, ResultType> syncMap_;
    std::set<SyncIdType> waitSet_;
};
} // namespace OSAL
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_FOUNDATION_OSAL_BASE_SYNCHRONIZER_H
