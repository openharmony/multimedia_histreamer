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

#ifndef HISTREAMER_HIPLAYER_PAUSE_STATE_H
#define HISTREAMER_HIPLAYER_PAUSE_STATE_H

#include <memory>

#include "foundation/error_code.h"
#include "foundation/log.h"
#include "play_executor.h"

#include "state.h"

namespace OHOS {
namespace Media {
class PauseState : public State {
public:
    explicit PauseState(PlayExecutor& executor, const std::string& name = "PauseState") : State(executor, name)
    {
    }

    ~PauseState() override = default;

    std::tuple<ErrorCode, Action> Enter(Intent) override
    {
        MEDIA_LOG_D("Pause state entered.");
        auto ret = executor_.DoPause();
        return {ret, ACTION_BUTT};
    }

    std::tuple<ErrorCode, Action> Play() override
    {
        MEDIA_LOG_D("Play in pause state.");
        return {SUCCESS, TRANS_TO_PLAYING};
    }

    std::tuple<ErrorCode, Action> Seek(const Plugin::Any& param) override
    {
        MEDIA_LOG_D("Seek in pause state.");
        if (param.Type() != typeid(int64_t)) {
            return {INVALID_PARAM_VALUE, ACTION_BUTT};
        }
        auto timeMs = Plugin::AnyCast<int64_t>(param);
        auto ret = executor_.DoSeek(timeMs);
        return {ret, ACTION_BUTT};
    }

    std::tuple<ErrorCode, Action> Resume() override
    {
        MEDIA_LOG_D("Resume in pause state.");
        return {SUCCESS, TRANS_TO_PLAYING};
    }

    std::tuple<ErrorCode, Action> Stop() override
    {
        MEDIA_LOG_D("Stop called in pause state.");
        return {SUCCESS, TRANS_TO_INIT};
    }
};
} // namespace Media
} // namespace OHOS
#endif
