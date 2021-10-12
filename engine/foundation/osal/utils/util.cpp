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

#include "util.h"

#if !defined(_MSC_VER) && !defined(__clang__)
#include <unistd.h>
#endif

#ifdef _MSC_VER
#include <windows.h>

void usleep(__int64 usec)
{
    HANDLE timer = CreateWaitableTimer(nullptr, TRUE, nullptr);
    // set interval to 100 nanosecond, using relative time
    LARGE_INTEGER dueTime;
    dueTime.QuadPart = -(10 * usec); // 10
    SetWaitableTimer(timer, &dueTime, 0, nullptr, nullptr, 0);
    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);
}
#else
extern "C" int usleep(unsigned);
#endif

namespace OHOS {
namespace Media {
namespace OSAL {
void SleepFor(unsigned ms)
{
    constexpr int factor = 1000;
    usleep(ms * factor);
}
} // namespace OSAL
} // namespace Media
} // namespace OHOS