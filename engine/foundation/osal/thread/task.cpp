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

#define HST_LOG_TAG "Task"

#include "task.h"

#include "foundation/log.h"

namespace OHOS {
namespace Media {
namespace OSAL {
Task::Task(std::string name, ThreadPriority priority)
    : name_(std::move(name)), runningState_(RunningState::PAUSED), loop_(priority)
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
    syncCond_.NotifyAll();
}

void Task::Start()
{
#ifndef START_FAKE_TASK
    OSAL::ScopedLock lock(stateMutex_);
    runningState_ = RunningState::STARTED;
    if (!loop_ && !loop_.CreateThread([this] { Run(); })) {
        MEDIA_LOG_E("task %s create failed", name_.c_str());
    } else {
        syncCond_.NotifyAll();
    }
    MEDIA_LOG_D("task %s start called", name_.c_str());
#endif
}

void Task::Stop()
{
    MEDIA_LOG_W("task %s stop entered, current state: %d", name_.c_str(), runningState_.load());
    OSAL::ScopedLock lock(stateMutex_);
    if (runningState_.load() != RunningState::STOPPED) {
        runningState_ = RunningState::STOPPING;
        syncCond_.NotifyAll();
        syncCond_.Wait(lock, [this] { return runningState_.load() == RunningState::STOPPED; });
    }
    MEDIA_LOG_W("task %s stop exited", name_.c_str());
}

void Task::Pause()
{
    MEDIA_LOG_D("task %s Pause called", name_.c_str());
    OSAL::ScopedLock lock(stateMutex_);
    switch (runningState_.load()) {
        case RunningState::STARTED: {
            runningState_ = RunningState::PAUSING;
            syncCond_.Wait(lock, [this] {
                return runningState_.load() == RunningState::PAUSED || runningState_.load() == RunningState::STOPPED;
            });
            break;
        }
        case RunningState::STOPPING: {
            syncCond_.Wait(lock, [this] { return runningState_.load() == RunningState::STOPPED; });
            break;
        }
        case RunningState::PAUSING: {
            syncCond_.Wait(lock, [this] { return runningState_.load() == RunningState::PAUSED; });
            break;
        }
        default:
            break;
    }
    MEDIA_LOG_D("task %s Pause done.", name_.c_str());
}

void Task::PauseAsync()
{
    MEDIA_LOG_D("task %s PauseAsync called", name_.c_str());
    OSAL::ScopedLock lock(stateMutex_);
    if (runningState_.load() == RunningState::STARTED) {
        runningState_ = RunningState::PAUSING;
    }
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
            handler_();
        }
        OSAL::ScopedLock lock(stateMutex_);
        if (runningState_.load() == RunningState::PAUSING || runningState_.load() == RunningState::PAUSED) {
            runningState_ = RunningState::PAUSED;
            syncCond_.NotifyAll();
            constexpr int timeoutMs = 500;
            syncCond_.WaitFor(lock, timeoutMs, [this] { return runningState_.load() != RunningState::PAUSED; });
        }
        if (runningState_.load() == RunningState::STOPPING || runningState_.load() == RunningState::STOPPED) {
            runningState_ = RunningState::STOPPED;
            syncCond_.NotifyAll();
            break;
        }
    }
}
} // namespace OSAL
} // namespace Media
} // namespace OHOS
