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

#ifndef HISTREAMER_HIPLAYER_INIT_STATE_H
#define HISTREAMER_HIPLAYER_INIT_STATE_H

#include <memory>

#include "foundation/error_code.h"
#include "foundation/log.h"
#include "play_executor.h"
#include "thread/mutex.h"

#include "state.h"

namespace OHOS {
namespace Media {
class InitState : public State {
public:
    explicit InitState(PlayExecutor& executor, const std::string& name = "InitState") : State(executor, name)
    {
    }

    ~InitState() override = default;

    std::tuple<ErrorCode, Action> SetSource(const Plugin::Any& param) override
    {
        OSAL::ScopedLock lock(mutex_);
        std::shared_ptr<MediaSource> source;
        if (param.Type() != typeid(std::shared_ptr<MediaSource>) ||
            !(source = Plugin::AnyCast<std::shared_ptr<MediaSource>>(param))) {
            return {INVALID_SOURCE, ACTION_BUTT};
        }
        auto ret = executor_.DoSetSource(source);
        return {ret, TRANS_TO_PREPARING};
    }

    std::tuple<ErrorCode, Action> Stop() override
    {
        return {SUCCESS, TRANS_TO_INIT};
    }

    std::tuple<ErrorCode, Action> Enter(Intent) override
    {
        OSAL::ScopedLock lock(mutex_);
        auto ret = executor_.DoStop();
        return {ret, ACTION_BUTT};
    }

private:
    OSAL::Mutex mutex_ {};
};
} // namespace Media
} // namespace OHOS
#endif
