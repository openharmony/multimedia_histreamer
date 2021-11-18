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

#ifndef HISTREAMER_STEADY_CLOCK_H
#define HISTREAMER_STEADY_CLOCK_H

#include <chrono>
#include "foundation/log.h"

namespace OHOS {
namespace Media {
class SteadyClock {
public:
    static int64_t GetCurrentTimeMs();
    SteadyClock();
    ~SteadyClock() = default;
    void Reset();
    int64_t ElapsedMilliseconds();
    int64_t ElapsedMicroseconds();
    int64_t ElapsedNanoseconds();
    int64_t ElapsedSeconds();

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> begin_;
};
} // namespace Media
} // namespace OHOS

#if defined(PROFILE)
#define PROFILE_BEGIN(message, ...)                                                                                    \
    MEDIA_LOG_W(message ", timestamp(ms): %" PRId64, ##__VA_ARGS__, SteadyClock::GetCurrentTimeMs());                  \
    OHOS::Media::SteadyClock _steadyClock;

#define PROFILE_RESET() _steadyClock.Reset()

#define PROFILE_END(message, ...)                                                                                      \
    MEDIA_LOG_W(message ", timestamp(ms): %" PRId64 ", duration(ms): %" PRId64, ##__VA_ARGS__,                         \
                SteadyClock::GetCurrentTimeMs(), _steadyClock.ElapsedMilliseconds())

#else
#define PROFILE_BEGIN(message, ...)
#define PROFILE_RESET()
#define PROFILE_END(message, ...)
#endif

#endif // HISTREAMER_STEADY_CLOCK_H
