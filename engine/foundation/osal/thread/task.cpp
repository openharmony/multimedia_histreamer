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
Task::Task(std::string name)
    : name_(std::move(name)), loop_([this] { Run(); }), runningState_(PAUSED), pauseDone_(false), workInProgress_(false)
{
    MEDIA_LOG_D("task %s ctor called", name_.c_str());
    loop_.SetName(name_);
}

Task::Task(std::string name, std::function<void()> handler) : Task(std::move(name))
{
    MEDIA_LOG_D("task %s ctor called", name_.c_str());
    handler_ = std::move(handler);
}

Task::~Task()
{
    MEDIA_LOG_D("task %s dtor called", name_.c_str());
    runningState_ = STOPPED;
    cv_.NotifyOne();
}

void Task::Start()
{
#ifndef START_FAKE_TASK
    OSAL::ScopedLock lock(stateMutex_);
    runningState_ = STARTED;
    cv_.NotifyOne();
    MEDIA_LOG_D("task %s start called", name_.c_str());
#endif
}

void Task::Stop()
{
    OSAL::ScopedLock lock(stateMutex_);
    runningState_ = STOPPED;
    cv_.NotifyOne();
    while (workInProgress_.load()) {}
    MEDIA_LOG_D("task %s stop called", name_.c_str());
}

void Task::StopAsync()
{
    OSAL::ScopedLock lock(stateMutex_);
    runningState_ = STOPPED;
    cv_.NotifyOne();
    MEDIA_LOG_D("task %s stop called", name_.c_str());
}

void Task::Pause()
{
    if (runningState_.load() != STARTED) {
        return;
    }
    OSAL::ScopedLock lock(stateMutex_);
    MEDIA_LOG_D("task %s Pause called", name_.c_str());
    if (runningState_.load() == STARTED) {
        pauseDone_ = false;
        runningState_ = PAUSED;
        while (!pauseDone_.load()) {}
    }
    MEDIA_LOG_D("task %s Pause done.", name_.c_str());
}

void Task::PauseAsync()
{
    if (runningState_.load() != STARTED) {
        return;
    }
    OSAL::ScopedLock lock(stateMutex_);
    MEDIA_LOG_D("task %s Pause called", name_.c_str());
    runningState_ = PAUSED;
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
        if (runningState_.load() == STARTED) {
            workInProgress_ = true;
            handler_();
            workInProgress_ = false;
            continue;
        }
        if (runningState_.load() == STOPPED) {
            MEDIA_LOG_D("task %s stopped, exit task", name_.c_str());
            break;
        }
        if (runningState_.load() == PAUSED) {
            OSAL::ScopedLock lock(cvMutex_);
            pauseDone_ = true;
            cv_.Wait(lock, [this] { return runningState_.load() != PAUSED; });
        }
    }
}
} // namespace OSAL
} // namespace Media
} // namespace OHOS
