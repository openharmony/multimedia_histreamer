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

#define HST_LOG_TAG "JobUtils"

#include "osal/task/jobutils.h"
#include "ffrt_inner.h"

namespace OHOS {
namespace Media {
void SleepInJob(unsigned ms)
{
    constexpr int factor = 1000; // to us
    ffrt_usleep(ms * factor);
}

void WaitForFinish(JobHandle handle)
{
    if (handle) {
        ffrt::wait({handle});
        handle = nullptr;
    }
}

void SubmitJobOnce(std::function<void()> job)
{
    JobHandle handle = ffrt::submit_h(job);
    ffrt::wait({handle});
}


JobHandle SubmitJobOnceAsync(std::function<void()> job)
{
    JobHandle handle = ffrt::submit_h(job);
    return handle;
}
} // namespace Media
} // namespace OHOS
