/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#define HST_LOG_TAG "AutoLock"
#include <memory>
#include "osal/task/autolock.h"
#include <utility>
namespace OHOS {
namespace Media {
AutoLock::AutoLock(Mutex& mutex) : mutex_(&mutex)
{
    mutex_->lock();
}

AutoLock::~AutoLock()
{
    if (mutex_ != nullptr) {
        mutex_->unlock();
        mutex_ = nullptr;
    }
}

AutoLock::AutoLock(AutoLock&& other) noexcept
{
    mutex_ = nullptr;
    *this = std::move(other);
}


AutoLock& AutoLock::operator=(AutoLock&& other) noexcept
{
    if (this != &other) {
        if (mutex_) {
            mutex_->unlock();
        }
        mutex_ = other.mutex_;
        other.mutex_ = nullptr;
    }
    return *this;
}
} // namespace Media
} // namespace OHOS