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

#define HST_LOG_TAG "ConditionVariable"

#include "osal/task/condition_variable.h"

namespace OHOS {
namespace Media {
ConditionVariable::ConditionVariable() noexcept : condInited_(true)
{
}

ConditionVariable::~ConditionVariable() noexcept
{
}

void ConditionVariable::NotifyOne() noexcept
{
    cond_.notify_one();
}

void ConditionVariable::NotifyAll() noexcept
{
    cond_.notify_all();
}

void ConditionVariable::Wait(AutoLock& lock) noexcept
{
    cond_.wait(lock);
}

void ConditionVariable::Wait(AutoLock& lock, std::function<bool()> pred) noexcept
{
    while (!pred()) {
        Wait(lock);
    }
}

bool ConditionVariable::WaitFor(AutoLock& lock, int timeoutMs)
{
    auto status = cond_.wait_for(lock, std::chrono::milliseconds(timeoutMs));
    if (status == ffrt::cv_status::no_timeout) {
        return true;
    } else {
        return false;
    }
}

bool ConditionVariable::WaitFor(AutoLock& lock, int timeoutMs, std::function<bool()> pred)
{
    return cond_.wait_for(lock, std::chrono::milliseconds(timeoutMs), pred);
}
} // namespace Media
} // namespace OHOS