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
#include <unistd.h>
#include "common/log.h"

namespace OHOS {
namespace Media {
void SleepInJob(unsigned ms)
{
    constexpr int factor = 1000; // to us
    usleep(ms * factor);
}

void WaitForFinish(JobHandle)
{
    MEDIA_LOG_E("JobHandle WaitForFinish has not been implemented.");
}

void SubmitJobOnce(std::function<void()> job)
{
    job();
}


JobHandle SubmitJobOnceAsync(std::function<void()> job)
{
    // pthread async submit has not been implemented
    job();
    JobHandle h = 0;
    return h;
}
} // namespace Media
} // namespace OHOS