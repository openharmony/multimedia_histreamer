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

#define HST_LOG_TAG "State"

#include "state.h"
#include "recorder/standard/media_utils.h"
namespace OHOS {
namespace Media {
namespace Record {
State::State(StateId stateId, std::string name, RecorderExecutor& executor)
    : stateId_(stateId), name_(std::move(name)), executor_(executor)
{
}

std::tuple<ErrorCode, Action> State::Enter(Intent intent)
{
    (void)intent;
    MEDIA_LOG_D("Enter state: %s", name_.c_str());
    return {ErrorCode::SUCCESS, Action::ACTION_BUTT};
}

void State::Exit()
{
    MEDIA_LOG_D("Exit state: %s", name_.c_str());
}

std::tuple<ErrorCode, Action> State::Execute(Intent intent, const Plugin::Any& param)
{
    return DispatchIntent(intent, param);
}

const std::string& State::GetName()
{
    return name_;
}

StateId State::GetStateId()
{
    return stateId_;
}

std::tuple<ErrorCode, Action> State::SetVideoSource(const Plugin::Any& param)
{
    (void)param;
    return {ErrorCode::ERROR_INVALID_OPERATION, Action::ACTION_BUTT};
}

std::tuple<ErrorCode, Action> State::SetAudioSource(const Plugin::Any& param)
{
    (void)param;
    return {ErrorCode::ERROR_INVALID_OPERATION, Action::ACTION_BUTT};
}

std::tuple<ErrorCode, Action> State::SetOutputFormat()
{
    return {ErrorCode::ERROR_INVALID_OPERATION, Action::ACTION_BUTT};
}

std::tuple<ErrorCode, Action> State::SetObs()
{
    return {ErrorCode::ERROR_INVALID_OPERATION, Action::ACTION_BUTT};
}

std::tuple<ErrorCode, Action> State::GetSurface()
{
    return {ErrorCode::ERROR_INVALID_OPERATION, Action::ACTION_BUTT};
}

std::tuple<ErrorCode, Action> State::Prepare()
{
    return {ErrorCode::ERROR_INVALID_OPERATION, Action::ACTION_BUTT};
}

std::tuple<ErrorCode, Action> State::Start()
{
    return {ErrorCode::ERROR_INVALID_OPERATION, Action::ACTION_BUTT};
}

std::tuple<ErrorCode, Action> State::Stop()
{
    return {ErrorCode::ERROR_INVALID_OPERATION, Action::ACTION_BUTT};
}

std::tuple<ErrorCode, Action> State::Pause()
{
    return {ErrorCode::ERROR_INVALID_OPERATION, Action::ACTION_BUTT};
}

std::tuple<ErrorCode, Action> State::Resume()
{
    return {ErrorCode::ERROR_INVALID_OPERATION, Action::ACTION_BUTT};
}

std::tuple<ErrorCode, Action> State::Reset()
{
    return {ErrorCode::ERROR_INVALID_OPERATION, Action::ACTION_BUTT};
}

std::tuple<ErrorCode, Action> State::SetParameter(const Plugin::Any &param)
{
    return {ErrorCode::ERROR_INVALID_OPERATION, Action::ACTION_BUTT};
}

std::tuple<ErrorCode, Action> State::OnError(const Plugin::Any& param)
{
    ErrorCode errorCode = ErrorCode::ERROR_UNKNOWN;
    if (param.Type() == typeid(ErrorCode)) {
        errorCode = Plugin::AnyCast<ErrorCode>(param);
    }
    executor_.DoOnError(errorCode);
    return {ErrorCode::SUCCESS, Action::TRANS_TO_ERROR};
}

std::tuple<ErrorCode, Action> State::DispatchIntent(Intent intent, const Plugin::Any& param)
{
    ErrorCode rtv = ErrorCode::SUCCESS;
    Action nextAction = Action::ACTION_BUTT;
    int32_t sourceId =-1;
    switch (intent) {
        case Intent::SET_OBS:
            std::tie(rtv, nextAction) = SetObs();
            break;
        case Intent::SET_VIDEO_SOURCE:
            std::tie(rtv, nextAction) = SetVideoSource(param);
            break;
        case Intent::SET_AUDIO_SOURCE:
            std::tie(rtv, nextAction) = SetAudioSource(param);
            break;
        case Intent::SET_PARAMETER:
            std::tie(rtv, nextAction) = SetParameter(param);
            break;
        case Intent::PREPARE:
            std::tie(rtv, nextAction) = Prepare();
            break;
        case Intent::START:
            std::tie(rtv, nextAction) = Start();
            break;
        case Intent::PAUSE:
            std::tie(rtv, nextAction) = Pause();
            break;
        case Intent::RESUME:
            std::tie(rtv, nextAction) = Resume();
            break;
        case Intent::STOP:
            std::tie(rtv, nextAction) = Stop();
            break;
        case Intent::NOTIFY_READY:
            // do nothing for ready
            break;
        case Intent::NOTIFY_ERROR:
            std::tie(rtv, nextAction) = OnError(param);
            break;
        case Intent::INTENT_BUTT:
            break;
        default:
            break;
    }
    MEDIA_LOG_D("DispatchIntent %s, curState: %s, nextState: %s", intentDesc_.at(intent).c_str(), name_.c_str(),
                actionDesc_.at(nextAction).c_str());
    return {rtv, nextAction};
}
} // namespace Record
} // namespace Media
} // namespace OHOS