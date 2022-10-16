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
#include "hitrace_utils.h"

namespace OHOS {
namespace Media {
    SyncTracker::SyncTracker(const std::string &value)
    {
        StartTrace(DEFAULT_HITRACE_TAG, value, DEFAULT_HITRACE_LIMIT);
    }

    SyncTracker::~SyncTracker()
    {
        FinishTrace(DEFAULT_HITRACE_TAG);
    }

    AsyncTracker::AsyncTracker(const std::string &title, int32_t taskId)
        :title_(title), taskId_(taskId)
    {
        StartAsyncTrace(DEFAULT_HITRACE_TAG, title, taskId, DEFAULT_HITRACE_LIMIT);
    }

    AsyncTracker::~AsyncTracker()
    {
        FinishAsyncTrace(DEFAULT_HITRACE_TAG, title_, taskId_);
    }
}
}