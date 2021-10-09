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

#ifndef HISTREAMER_HIPLAYER_STATE_H
#define HISTREAMER_HIPLAYER_STATE_H

#include <map>
#include <memory>
#include <string>
#include <tuple>

#include "common/any.h"
#include "foundation/error_code.h"
#include "foundation/log.h"
#include "play_executor.h"

namespace OHOS {
namespace Media {
enum class StateId {
    INIT,
    PREPARING,
    READY,
    PAUSE,
    PLAYING,
    BUTT,
};

enum class Intent {
    SET_SOURCE,
    SEEK,
    PLAY,
    PAUSE,
    RESUME,
    STOP,
    SET_ATTRIBUTE,
    NOTIFY_READY,
    NOTIFY_COMPLETE,
    NOTIFY_ERROR,
    INTENT_BUTT
};

enum class Action {
    TRANS_TO_INIT,
    TRANS_TO_PREPARING,
    TRANS_TO_READY,
    TRANS_TO_PLAYING,
    TRANS_TO_PAUSE,
    ACTION_PENDING,
    ACTION_BUTT
};

class State {
public:
    State(StateId stateId, std::string name, PlayExecutor& executor);
    virtual ~State() = default;
    virtual std::tuple<ErrorCode, Action> Enter(Intent intent);
    virtual void Exit();
    std::tuple<ErrorCode, Action> Execute(Intent intent, const Plugin::Any& param);
    const std::string& GetName();
    StateId GetStateId();
    virtual std::tuple<ErrorCode, Action> SetSource(const Plugin::Any& param);
    virtual std::tuple<ErrorCode, Action> Play();
    virtual std::tuple<ErrorCode, Action> Stop();
    virtual std::tuple<ErrorCode, Action> Pause();
    virtual std::tuple<ErrorCode, Action> Resume();
    virtual std::tuple<ErrorCode, Action> Seek(const Plugin::Any& param);
    virtual std::tuple<ErrorCode, Action> SetAttribute();
    virtual std::tuple<ErrorCode, Action> OnReady();
    virtual std::tuple<ErrorCode, Action> OnError(const Plugin::Any& param) final;
    virtual std::tuple<ErrorCode, Action> OnComplete();

protected:
    std::tuple<ErrorCode, Action> DispatchIntent(Intent intent, const Plugin::Any& param);

    const StateId stateId_;
    const std::string name_;
    PlayExecutor& executor_;
    const std::map<Intent, std::string> intentDesc_ = {{Intent::SET_SOURCE, "SET_SOURCE"},
                                                       {Intent::SEEK, "SEEK"},
                                                       {Intent::PLAY, "PLAY"},
                                                       {Intent::PAUSE, "PAUSE"},
                                                       {Intent::RESUME, "RESUME"},
                                                       {Intent::STOP, "STOP"},
                                                       {Intent::SET_ATTRIBUTE, "SET_ATTRIBUTE"},
                                                       {Intent::NOTIFY_READY, "NOTIFY_READY"},
                                                       {Intent::NOTIFY_COMPLETE, "NOTIFY_COMPLETE"},
                                                       {Intent::NOTIFY_ERROR, "NOTIFY_ERROR"},
                                                       {Intent::INTENT_BUTT, "INTENT_BUTT"}};
    const std::map<Action, std::string> actionDesc_ = {
        {Action::TRANS_TO_INIT, "TRANS_TO_INIT"},   {Action::TRANS_TO_PREPARING, "TRANS_TO_PREPARING"},
        {Action::TRANS_TO_READY, "TRANS_TO_READY"}, {Action::TRANS_TO_PLAYING, "TRANS_TO_PLAYING"},
        {Action::TRANS_TO_PAUSE, "TRANS_TO_PAUSE"}, {Action::ACTION_PENDING, "ACTION_PENDING"},
        {Action::ACTION_BUTT, "ACTION_BUTT"}};
};
} // namespace Media
} // namespace OHOS
#endif
