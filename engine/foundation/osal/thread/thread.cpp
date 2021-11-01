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

#define LOG_TAG "Thread"

#include "thread.h"
#include "foundation/log.h"

namespace OHOS {
namespace Media {
namespace OSAL {
Thread::Thread() noexcept : id_(), name_(), state_()
{
}

Thread::Thread(Thread&& other) noexcept
{
    *this = std::move(other);
}

Thread& Thread::operator=(Thread&& other) noexcept
{
    if (this != &other) {
        id_ = other.id_;
        name_ = std::move(other.name_);
        state_ = std::move(other.state_);
    }
    return *this;
}

Thread::~Thread() noexcept
{
    if (state_) {
        pthread_join(id_, nullptr);
    }
}

Thread::operator bool() const noexcept
{
    return state_ != nullptr;
}

void Thread::SetName(const std::string& name)
{
    name_ = name;
}

bool Thread::CreateThread(const std::function<void()>& func)
{
    state_ = std::unique_ptr<State>(new State);
    state_->func_ = func;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
#if defined(THREAD_STACK_SIZE) and THREAD_STACK_SIZE > 0
    pthread_attr_setstacksize(&attr, THREAD_STACK_SIZE);
    MEDIA_LOG_I("thread stack size set to %d", THREAD_STACK_SIZE);
#endif
    int rtv = pthread_create(&id_, &attr, Thread::Run, state_.get());
    if (rtv == 0) {
        MEDIA_LOG_I("thread %d, id %s create succ", pthread_self(), name_.c_str());
        SetNameInternal();
    } else {
        state_.reset();
        MEDIA_LOG_E("thread create failed");
    }
    return rtv == 0;
}

void Thread::SetNameInternal()
{
#ifdef SUPPORT_PTHREAD_NAME
    if (state_ && !name_.empty()) {
        constexpr int threadNameMaxSize = 15;
        if (name_.size() > threadNameMaxSize) {
            MEDIA_LOG_W("task name %s exceed max size: %d", name_.c_str(), threadNameMaxSize);
            name_ = name_.substr(0, threadNameMaxSize);
        }
        pthread_setname_np(id_, name_.c_str());
    }
#endif
}

void* Thread::Run(void* arg) // NOLINT: void*
{
    auto state = static_cast<State*>(arg);
    if (state && state->func_) {
        state->func_();
    }
    MEDIA_LOG_I("Thread::Run exited...");
    return nullptr;
}
} // namespace OSAL
} // namespace Media
} // namespace OHOS
