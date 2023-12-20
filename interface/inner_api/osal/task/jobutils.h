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

#ifndef HISTREAMER_FOUNDATION_OSAL_JOB_UTILS_H
#define HISTREAMER_FOUNDATION_OSAL_JOB_UTILS_H

#include <functional>
#include "osal/task/mutex.h"

namespace OHOS {
namespace Media {
#ifdef MEDIA_FOUNDATION_FFRT
    using JobHandle = ffrt::task_handle;
#else
    using JobHandle = pthread_t;
#endif

void SleepInJob(unsigned ms);
void WaitForFinish(JobHandle handle);
void SubmitJobOnce(std::function<void()> job);
JobHandle SubmitJobOnceAsync(std::function<void()> job);

} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_FOUNDATION_OSAL_JOB_UTILS_H
