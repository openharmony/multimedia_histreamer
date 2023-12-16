/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "osal/task/task.h"
#include "inner_api/common/log.h"

namespace OHOS {
namespace Media {
// ffrt::qos priority sorting:qos_utility < qos_default < qos_user_initiated <
// qos_deadline_request < qos_user_interactive
ffrt::qos ConvertPriorityType(TaskPriority priority) {
    switch (priority) {
        case TaskPriority::LOW:
            return ffrt::qos_utility;
        case TaskPriority::NORMAL:
            return ffrt::qos_default;
        case TaskPriority::MIDDLE:
            return ffrt::qos_user_initiated;
        case TaskPriority::HIGHEST:
            return ffrt::qos_user_interactive;
        default:
            return ffrt::qos_deadline_request;
    }
}

Task::Task(std::string name, TaskPriority priority)
    : name_(std::move(name)), priority_(priority), runningState_(RunningState::STOPPED)
{
    MEDIA_LOG_D("task " PUBLIC_LOG_S " ctor called", name_.c_str());
}

Task::Task(std::string name, std::function<void()> job, TaskPriority priority)
    : Task(std::move(name), priority)
{
    MEDIA_LOG_D("task " PUBLIC_LOG_S " ctor called", name_.c_str());
    job_ = std::move(job);
}

Task::~Task()
{
    MEDIA_LOG_I("task " PUBLIC_LOG_S " dtor called", name_.c_str());
    runningState_ = RunningState::STOPPED;
    syncCond_.NotifyAll();
}

void Task::Start()
{
    MEDIA_LOG_I("task " PUBLIC_LOG_S " start called", name_.c_str());
    AutoLock lock(stateMutex_);
    runningState_ = RunningState::STARTED;
    if (!loop_) {
        loop_ = std::make_unique<ffrt::thread>(name_.c_str(), ConvertPriorityType(priority_), [this] { Run(); });
    }
    if (!loop_) {
        MEDIA_LOG_E("task " PUBLIC_LOG_S " create failed", name_.c_str());
    } else {
        syncCond_.NotifyAll();
    }
}

void Task::Stop()
{
    MEDIA_LOG_W("task " PUBLIC_LOG_S " stop entered, current state: " PUBLIC_LOG_D32,
                name_.c_str(), runningState_.load());
    AutoLock lock(stateMutex_);
    if (runningState_.load() != RunningState::STOPPED) {
        runningState_ = RunningState::STOPPING;
        syncCond_.NotifyAll();
        syncCond_.Wait(lock, [this] { return runningState_.load() == RunningState::STOPPED; });
        if (loop_) {
            if (loop_->joinable()) {
                loop_->join();
            }
            loop_ = nullptr;
        }
    }
    MEDIA_LOG_W("task " PUBLIC_LOG_S " stop exited", name_.c_str());
}

void Task::StopAsync()
{
    MEDIA_LOG_D("task " PUBLIC_LOG_S " StopAsync called", name_.c_str());
    AutoLock lock(stateMutex_);
    if (runningState_.load() != RunningState::STOPPED) {
        runningState_ = RunningState::STOPPING;
    }
}

void Task::Pause()
{
    AutoLock lock(stateMutex_);
    RunningState state = runningState_.load();
    MEDIA_LOG_I("task " PUBLIC_LOG_S " Pause called, running state = " PUBLIC_LOG_D32, name_.c_str(), state);
    switch (state) {
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
    MEDIA_LOG_I("task " PUBLIC_LOG_S " Pause done.", name_.c_str());
}

void Task::PauseAsync()
{
    MEDIA_LOG_I("task " PUBLIC_LOG_S " PauseAsync called", name_.c_str());
    AutoLock lock(stateMutex_);
    if (runningState_.load() == RunningState::STARTED) {
        runningState_ = RunningState::PAUSING;
    }
}

void Task::RegisterJob(std::function<void()> job)
{
    MEDIA_LOG_D("task " PUBLIC_LOG_S " RegisterJob called", name_.c_str());
    job_ = std::move(job);
}

void Task::DoTask()
{
    MEDIA_LOG_D("task " PUBLIC_LOG_S " not override DoTask...", name_.c_str());
}

void Task::Run()
{
    for (;;) {
        MEDIA_LOG_DD("task " PUBLIC_LOG_S " is running on state : " PUBLIC_LOG_D32,
                     name_.c_str(), runningState_.load());
        if (runningState_.load() == RunningState::STARTED) {
            job_();
        }
        AutoLock lock(stateMutex_);
        if (runningState_.load() == RunningState::PAUSING || runningState_.load() == RunningState::PAUSED) {
            runningState_ = RunningState::PAUSED;
            syncCond_.NotifyAll();
            constexpr int timeoutMs = 500;
            syncCond_.WaitFor(lock, timeoutMs, [this] { return runningState_.load() != RunningState::PAUSED; });
        }
        if (runningState_.load() == RunningState::STOPPING || runningState_.load() == RunningState::STOPPED) {
            MEDIA_LOG_I("task " PUBLIC_LOG_S " is stopped", name_.c_str());
            runningState_ = RunningState::STOPPED;
            syncCond_.NotifyAll();
            break;
        }
    }
}
} // namespace Media
} // namespace OHOS
