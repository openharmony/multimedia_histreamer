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

#define LOG_TAG "Task"

#include "task.h"

#include "foundation/log.h"
#include "utils/util.h"

namespace OHOS {
namespace Media {
namespace OSAL {
Task::Task(std::string name, ThreadPriority priority)
    : name_(std::move(name)),
      runningState_(RunningState::PAUSED),
      loop_(priority),
      pauseDone_(false),
      workInProgress_(false)
{
    MEDIA_LOG_D("task %s ctor called", name_.c_str());
    loop_.SetName(name_);
}

Task::Task(std::string name, std::function<void()> handler, ThreadPriority priority) : Task(std::move(name), priority)
{
    MEDIA_LOG_D("task %s ctor called", name_.c_str());
    handler_ = std::move(handler);
}

Task::~Task()
{
    MEDIA_LOG_D("task %s dtor called", name_.c_str());
    runningState_ = RunningState::STOPPED;
    cv_.NotifyOne();
}

void Task::Start()
{
#ifndef START_FAKE_TASK
    OSAL::ScopedLock lock(stateMutex_);
    runningState_ = RunningState::STARTED;
    if (!loop_ && !loop_.CreateThread([this] { Run(); })) {
        MEDIA_LOG_E("task %s create failed", name_.c_str());
    } else {
        cv_.NotifyOne();
    }
    MEDIA_LOG_D("task %s start called", name_.c_str());
#endif
}

void Task::Stop()
{
    OSAL::ScopedLock lock(stateMutex_);
    runningState_ = RunningState::STOPPED;
    cv_.NotifyOne();
    while (workInProgress_.load()) {}
    MEDIA_LOG_D("task %s stop called", name_.c_str());
}

void Task::StopAsync()
{
    OSAL::ScopedLock lock(stateMutex_);
    runningState_ = RunningState::STOPPED;
    cv_.NotifyOne();
    MEDIA_LOG_D("task %s stop called", name_.c_str());
}

void Task::Pause()
{
    if (runningState_.load() != RunningState::STARTED) {
        return;
    }
    OSAL::ScopedLock lock(stateMutex_);
    MEDIA_LOG_D("task %s Pause called", name_.c_str());
    if (runningState_.load() == RunningState::STARTED) {
        pauseDone_ = false;
        runningState_ = RunningState::PAUSED;
        pauseCond_.Wait(lock, [this] { return pauseDone_.load(); });
    }
    MEDIA_LOG_D("task %s Pause done.", name_.c_str());
}

void Task::PauseAsync()
{
    if (runningState_.load() != RunningState::STARTED) {
        return;
    }
    OSAL::ScopedLock lock(stateMutex_);
    MEDIA_LOG_D("task %s Pause called", name_.c_str());
    runningState_ = RunningState::PAUSED;
}

void Task::RegisterHandler(std::function<void()> handler)
{
    MEDIA_LOG_D("task %s RegisterHandler called", name_.c_str());
    handler_ = std::move(handler);
}

void Task::DoTask()
{
    MEDIA_LOG_D("task %s not override DoTask...", name_.c_str());
}

void Task::Run()
{
    for (;;) {
        if (runningState_.load() == RunningState::STARTED) {
            workInProgress_ = true;
            handler_();
            workInProgress_ = false;
            continue;
        }
        if (runningState_.load() == RunningState::STOPPED) {
            MEDIA_LOG_D("task %s stopped, exit task", name_.c_str());
            break;
        }
        OSAL::ScopedLock lock(stateMutex_);
        if (runningState_.load() == RunningState::PAUSED) {
            pauseDone_ = true;
            pauseCond_.NotifyOne();
            constexpr int timeoutMs = 500;
            cv_.WaitFor(lock, timeoutMs, [this] { return runningState_.load() != RunningState::PAUSED; });
        }
    }
}
} // namespace OSAL
} // namespace Media
} // namespace OHOS
