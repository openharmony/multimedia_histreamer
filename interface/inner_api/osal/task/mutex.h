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

#ifndef HISTREAMER_FOUNDATION_OSAL_MUTEX_H
#define HISTREAMER_FOUNDATION_OSAL_MUTEX_H

#ifdef MEDIA_FOUNDATION_FFRT
    #include "ffrt_inner.h"
#else
    #include <pthread.h>
#endif

namespace OHOS {
namespace Media {

#ifdef MEDIA_FOUNDATION_FFRT
using Mutex = ffrt::mutex;
#else
class Mutex {
public:
    Mutex();

    virtual ~Mutex();

    Mutex(const Mutex&) = delete;

    Mutex operator=(const Mutex&) = delete;

    void lock();

    bool try_lock();

    void unlock();

private:
    pthread_mutex_t nativeHandle_;
    bool created_;
    friend class ConditionVariable;
};
#endif
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_FOUNDATION_OSAL_MUTEX_H
