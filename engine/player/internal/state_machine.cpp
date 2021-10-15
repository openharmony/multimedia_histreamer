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

#define LOG_TAG "StateMachine"

#include "state_machine.h"

namespace OHOS {
namespace Media {
StateMachine::StateMachine(PlayExecutor& executor)
    : Task("StateMachine"),
      intentSync_("fsmSync"),
      curState_(std::make_shared<InitState>(StateId::INIT, executor)),
      jobs_("StateMachineJobQue")
{
    AddState(curState_);
    AddState(std::make_shared<PreparingState>(StateId::PREPARING, executor));
    AddState(std::make_shared<ReadyState>(StateId::READY, executor));
    AddState(std::make_shared<PlayingState>(StateId::PLAYING, executor));
    AddState(std::make_shared<PauseState>(StateId::PAUSE, executor));
}

void StateMachine::Stop()
{
    MEDIA_LOG_D("Stop called.");
    jobs_.SetActive(false);
    Task::Stop();
}

void StateMachine::SetStateCallback(StateChangeCallback* callback)
{
    callback_ = callback;
}

const std::string& StateMachine::GetCurrentState() const
{
    return curState_->GetName();
}

StateId StateMachine::GetCurrentStateId() const
{
    return curState_->GetStateId();
}

ErrorCode StateMachine::SendEvent(Intent intent, const Plugin::Any& param) const
{
    return const_cast<StateMachine*>(this)->SendEvent(intent, param);
}

ErrorCode StateMachine::SendEvent(Intent intent, const Plugin::Any& param)
{
    SendEventAsync(intent, param);
    constexpr int timeoutMs = 5000;
    ErrorCode errorCode = ERROR_TIMEOUT;
    if (!intentSync_.WaitFor(intent, timeoutMs, errorCode)) {
        MEDIA_LOG_E("SendEvent timeout, intent: %d", static_cast<int>(intent));
    }
    return errorCode;
}

ErrorCode StateMachine::SendEventAsync(Intent intent, const Plugin::Any& param) const
{
    return const_cast<StateMachine*>(this)->SendEventAsync(intent, param);
}

ErrorCode StateMachine::SendEventAsync(Intent intent, const Plugin::Any& param)
{
    MEDIA_LOG_D("SendEventAsync, intent: %d", static_cast<int>(intent));
    jobs_.Push([this, intent, param]() -> Action { return ProcessIntent(intent, param); });
    return SUCCESS;
}

Action StateMachine::ProcessIntent(Intent intent, const Plugin::Any& param)
{
    MEDIA_LOG_D("ProcessIntent, curState: %s, intent: %d.", curState_->GetName().c_str(), intent);
    OSAL::ScopedLock lock(mutex_);
    lastIntent = intent;
    ErrorCode rtv = SUCCESS;
    Action nextAction = Action::ACTION_BUTT;
    std::tie(rtv, nextAction) = curState_->Execute(intent, param);
    if (rtv == SUCCESS) {
        rtv = ProcAction(nextAction);
    }
    OnIntentExecuted(intent, nextAction, rtv);
    return (rtv == SUCCESS) ? nextAction : Action::ACTION_BUTT;
}

void StateMachine::DoTask()
{
    constexpr int timeoutMs = 100;
    auto job = jobs_.Pop(timeoutMs);
    if (!job) {
        return;
    }
    auto action = job();
    switch (action) {
        case Action::ACTION_PENDING:
            pendingJobs_.push(job);
            break;
        case Action::TRANS_TO_INIT:
        case Action::TRANS_TO_READY:
        case Action::TRANS_TO_PREPARING:
        case Action::TRANS_TO_PLAYING:
        case Action::TRANS_TO_PAUSE: {
            if (!pendingJobs_.empty()) {
                job = pendingJobs_.front();
                pendingJobs_.pop();
                action = job();
                if (action == Action::ACTION_PENDING) {
                    pendingJobs_.push(job);
                }
            }
            break;
        }
        case Action::ACTION_BUTT:
            // fall through
        default:
            break;
    }
}

void StateMachine::AddState(const std::shared_ptr<State>& state)
{
    states_[state->GetStateId()] = state;
}

ErrorCode StateMachine::ProcAction(Action nextAction)
{
    std::shared_ptr<State> nextState = nullptr;
    switch (nextAction) {
        case Action::TRANS_TO_INIT:
            nextState = states_[StateId::INIT];
            break;
        case Action::TRANS_TO_PREPARING:
            nextState = states_[StateId::PREPARING];
            break;
        case Action::TRANS_TO_READY:
            nextState = states_[StateId::READY];
            break;
        case Action::TRANS_TO_PLAYING:
            nextState = states_[StateId::PLAYING];
            break;
        case Action::TRANS_TO_PAUSE:
            nextState = states_[StateId::PAUSE];
            break;
        default:
            break;
    }
    ErrorCode ret = SUCCESS;
    if (nextState) {
        ret = TransitionTo(nextState);
    }
    return ret;
}

ErrorCode StateMachine::TransitionTo(const std::shared_ptr<State>& state)
{
    if (state == nullptr) {
        MEDIA_LOG_E("TransitionTo, nullptr for state");
        return NULL_POINTER_ERROR;
    }
    ErrorCode rtv = SUCCESS;
    if (state != curState_) {
        curState_->Exit();
        curState_ = state;
        Action nextAction;
        std::tie(rtv, nextAction) = curState_->Enter(lastIntent);
        if (rtv == SUCCESS) {
            rtv = ProcAction(nextAction);
        }
        if (callback_) {
            callback_->OnStateChanged(curState_->GetStateId());
        }
    }
    return rtv;
}

void StateMachine::OnIntentExecuted(Intent intent, Action action, ErrorCode result)
{
    MEDIA_LOG_D("OnIntentExecuted, curState: %s, intent: %d, action: %d, result: %d", curState_->GetName().c_str(),
                static_cast<int>(intent), static_cast<int>(action), static_cast<int>(result));
    if (action == Action::ACTION_PENDING) {
        return;
    }
    if (intent == Intent::PLAY) {
        if (action == Action::TRANS_TO_PLAYING) {
            intentSync_.Notify(Intent::PLAY, result);
        }
    } else {
        if (intent == Intent::NOTIFY_READY && action == Action::TRANS_TO_PLAYING) {
            intentSync_.Notify(Intent::PLAY, result);
        } else {
            intentSync_.Notify(intent, result);
        }
    }
}
} // namespace Media
} // namespace OHOS