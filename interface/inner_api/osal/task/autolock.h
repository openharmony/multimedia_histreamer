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

#ifndef HISTREAMER_FOUNDATION_OSAL_AUTO_LOCK_H
#define HISTREAMER_FOUNDATION_OSAL_AUTO_LOCK_H

#include "osal/task/mutex.h"

namespace OHOS {
namespace Media {

#ifdef MEDIA_FOUNDATION_FFRT
    using AutoLock = std::unique_lock<Mutex>;
#else
    class AutoLock {
    public:
        AutoLock() = delete;

        explicit AutoLock(Mutex& mutex);

        AutoLock(AutoLock&& other) noexcept;

        AutoLock& operator=(AutoLock&& other) noexcept;

        virtual ~AutoLock();

    private:
        Mutex* mutex_;
        friend class ConditionVariable;
    };
#endif
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_FOUNDATION_OSAL_AUTO_LOCK_H
