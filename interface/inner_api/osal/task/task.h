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

#ifndef HISTREAMER_FOUNDATION_OSAL_TASK_H
#define HISTREAMER_FOUNDATION_OSAL_TASK_H

#include <atomic>
#include <functional>
#include <string>

#include "osal/task/condition_variable.h"
#include "osal/task/mutex.h"

#ifndef MEDIA_FOUNDATION_FFRT
    #include "osal/task/thread.h"
#endif

namespace OHOS {
namespace Media {

enum class TaskPriority : int {
    LOW,
    NORMAL,
    MIDDLE,
    HIGH,
    HIGHEST,
};

class TaskNative;

class Task {
public:
    explicit Task(std::string name, TaskPriority priority = TaskPriority::HIGH);

    explicit Task(std::string name, std::function<void()> job, TaskPriority priority = TaskPriority::HIGH);

    virtual ~Task();

    virtual void Start();

    virtual void Stop();

    virtual void StopAsync();

    virtual void Pause();

    virtual void PauseAsync();

    virtual void DoTask();

    void RegisterJob(std::function<void()> job);

    bool IsTaskRunning() { return runningState_ == RunningState::STARTED; }

private:
    enum class RunningState {
        STARTED,
        PAUSING,
        PAUSED,
        STOPPING,
        STOPPED,
    };

    void Run();

    const std::string name_;
    const TaskPriority priority_;
    std::atomic<RunningState> runningState_{RunningState::PAUSED};
    std::function<void()> job_ = [this] { DoTask(); };
#ifdef MEDIA_FOUNDATION_FFRT
    std::unique_ptr<ffrt::thread> loop_;
#else
    std::unique_ptr<Thread> loop_;
#endif
    Mutex stateMutex_{};
    ConditionVariable syncCond_{};
};
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_FOUNDATION_OSAL_TASK_H
