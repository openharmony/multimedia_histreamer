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

#define HST_LOG_TAG "ConditionVariable"

#include "foundation/osal/thread/condition_variable.h"
#include <ratio>
#include "foundation/log.h"

namespace {
inline long long TmToNs(struct timespec tm)
{
    return tm.tv_sec * std::giga::num + tm.tv_nsec;
}

inline struct timespec NsToTm(long long ns)
{
    return {ns / std::giga::num, static_cast<long>(ns % std::giga::num)};
}
}

namespace OHOS {
namespace Media {
namespace OSAL {
ConditionVariable::ConditionVariable() noexcept : condInited_(true)
{
    pthread_condattr_t attr;
    pthread_condattr_init(&attr);
#ifndef __LITEOS_M__
#ifdef USING_CLOCK_REALTIME
    pthread_condattr_setclock(&attr, CLOCK_REALTIME);
#else
    pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
#endif
#endif
    int rtv = pthread_cond_init(&cond_, &attr);
    if (rtv != 0) {
        condInited_ = false;
        MEDIA_LOG_E("failed to init pthread cond variable");
    }
    pthread_condattr_destroy(&attr);
}

ConditionVariable::~ConditionVariable() noexcept
{
    if (condInited_) {
        pthread_cond_destroy(&cond_);
    }
}

void ConditionVariable::NotifyOne() noexcept
{
    int ret = pthread_cond_signal(&cond_);
    if (ret != 0) {
        MEDIA_LOG_E("NotifyOne failed with errno = " PUBLIC_LOG_D32, ret);
    }
}

void ConditionVariable::NotifyAll() noexcept
{
    int ret = pthread_cond_broadcast(&cond_);
    if (ret != 0) {
        MEDIA_LOG_E("NotifyAll failed with errno = " PUBLIC_LOG_D32, ret);
    }
}

void ConditionVariable::Wait(ScopedLock& lock) noexcept
{
    pthread_cond_wait(&cond_, &(lock.mutex_->nativeHandle_));
}

bool ConditionVariable::WaitFor(ScopedLock& lock, int timeoutMs)
{
    if (timeoutMs < 0) {
        MEDIA_LOG_E("ConditionVariable WaitUntil invalid timeoutMs: " PUBLIC_LOG_D32, timeoutMs);
        return false;
    }
    struct timespec timeout = {0, 0};
#ifdef USING_CLOCK_REALTIME
    clock_gettime(CLOCK_REALTIME, &timeout);
#else
    clock_gettime(CLOCK_MONOTONIC, &timeout);
#endif
    timeout = NsToTm(TmToNs(timeout) + timeoutMs * std::mega::num);
    return pthread_cond_timedwait(&cond_, &(lock.mutex_->nativeHandle_),
        &timeout) == 0;
}
} // namespace OSAL
} // namespace Media
} // namespace OHOS
