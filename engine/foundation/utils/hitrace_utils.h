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
#include "hitrace_meter.h"

#define AUTO_SYNC_TRACE(value) SyncScopedTracer __autoSyncTrace(HITRACE_TAG_ZMEDIA, value, 0)
#define MANUAL_SYNC_TRACE(value) StartTrace(HITRACE_TAG_ZMEDIA, value, 0)
#define MANUAL_SYNC_TRACE_END() FinishTrace(HITRACE_TAG_ZMEDIA)
#define AUTO_ASYNC_TRACE(value, taskId) AsyncScopedTracer __autoAsyncTrace(HITRACE_TAG_ZMEDIA, value, taskId, 0)
#define MANUAL_ASYNC_TRACE(value, taskId) StartAsyncTrace(HITRACE_TAG_ZMEDIA, value, taskId, 0)
#define MANUAL_ASYNC_TRACE_END(value, taskId) FinishAsyncTrace(value, taskId, HITRACE_TAG_ZMEDIA)

namespace OHOS {
namespace Media {

class SyncScopedTracer {
public:
    SyncScopedTracer(uint64_t label, const std::string &value, float limit);
    ~SyncScopedTracer();
private:
    uint64_t label_;
};

class AsyncScopedTracer {
public:
    AsyncScopedTracer(uint64_t label, const std::string &value, int32_t taskId, float limit);
    ~AsyncScopedTracer();
private:
    uint64_t label_;
    const std::string value_;
    int32_t taskId_;
};
}
}
#endif // HISTREAMER_HITRACE_UTILS_H
