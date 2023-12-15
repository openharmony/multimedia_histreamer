/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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

#ifndef HISTREAMER_HITRACE_UTILS_H
#define HISTREAMER_HITRACE_UTILS_H

#if defined(OHOS_LITE) || defined(_WIN32)
#define SYNC_TRACER()
#define SYNC_TRACE_START(title)
#define SYNC_TRACE_END()
#define ASYNC_TRACER()
#define ASYNC_TRACE_START(title, taskId)
#define ASYNC_TRACE_END(title, taskId)
#define COUNT_TRACE(title, count)
#else
#include "hitrace_meter.h"
#define DEFAULT_HITRACE_LIMIT (-1)
#define DEFAULT_HITRACE_TAG HITRACE_TAG_ZMEDIA
#define DEFAULT_HITRACE_TASK_ID 1
#define DEFAULT_HITRACE_PREFIX "HiStreamer::"
#define BUILD_TRACE_TITLE() ((DEFAULT_HITRACE_PREFIX) + (std::string(__FUNCTION__)))
#define WRAP_TITLE_PREFIX(title) ((DEFAULT_HITRACE_PREFIX) + (std::string(__FUNCTION__)) + " : " + (title))
#define SYNC_TRACER() SyncTracker __syncTracker(BUILD_TRACE_TITLE())
#define SYNC_TRACE_START(title) StartTrace(DEFAULT_HITRACE_TAG, WRAP_TITLE_PREFIX(title), DEFAULT_HITRACE_LIMIT)
#define SYNC_TRACE_END() FinishTrace(DEFAULT_HITRACE_TAG)
#define ASYNC_TRACER() AsyncTracker __asyncTracker(BUILD_TRACE_TITLE(), DEFAULT_HITRACE_TASK_ID)
#define ASYNC_TRACE_START(title, taskId) \
StartAsyncTrace(DEFAULT_HITRACE_TAG, WRAP_TITLE_PREFIX(title), taskId, DEFAULT_HITRACE_LIMIT)
#define ASYNC_TRACE_END(title, taskId) FinishAsyncTrace(WRAP_TITLE_PREFIX(title), taskId, DEFAULT_HITRACE_TAG)
#define COUNT_TRACE(title, count) CountTrace(DEFAULT_HITRACE_TAG, WRAP_TITLE_PREFIX(title), count)

namespace OHOS {
namespace Media {
    class SyncTracker {
    public:
        explicit SyncTracker(const std::string &title);
        ~SyncTracker();
    };

    class AsyncTracker {
    public:
        AsyncTracker(const std::string &title, int32_t taskId);
        ~AsyncTracker();
    private:
        const std::string title_;
        int32_t taskId_;
    };
}
}
#endif
#endif // HISTREAMER_HITRACE_UTILS_H
