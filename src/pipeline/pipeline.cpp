/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#define HST_LOG_TAG "Pipeline"

#include <queue>
#include <stack>
#include "pipeline/pipeline.h"
#include "osal/task/autolock.h"
#include "osal/task/jobutils.h"
#include "common/log.h"
#include "osal/utils/hitrace_utils.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
Pipeline::~Pipeline() {}
void Pipeline::Init(const std::shared_ptr<EventReceiver>& receiver, const std::shared_ptr<FilterCallback>& callback)
{
    state_ = FilterState::INITIALIZED;
    eventReceiver_ = receiver;
    filterCallback_ = callback;
}

Status Pipeline::Prepare()
{
    state_ = FilterState::PREPARING;
    SubmitJobOnce([&] {
        AutoLock lock(mutex_);
        for (auto it = filters_.begin(); it != filters_.end(); ++it) {
            auto rtv = (*it)->Prepare();
            FALSE_RETURN_V(rtv == Status::OK, rtv);
        }
        return Status::OK;
    });
    return Status::OK;
}

Status Pipeline::Start()
{
    state_ = FilterState::RUNNING;
    SubmitJobOnce([&] {
        AutoLock lock(mutex_);
        for (auto it = filters_.begin(); it != filters_.end(); ++it) {
            auto rtv = (*it)->Start();
            FALSE_RETURN_V(rtv == Status::OK, rtv);
        }
        return Status::OK;
    });
    return Status::OK;
}

Status Pipeline::Pause()
{
    if (state_ == FilterState::PAUSED) {
        return Status::OK;
    }
    if (state_ != FilterState::READY && state_ != FilterState::RUNNING) {
        return Status::ERROR_UNKNOWN;
    }
    state_ = FilterState::PAUSED;
    SubmitJobOnce([&] {
        AutoLock lock(mutex_);
        for (auto it = filters_.begin(); it != filters_.end(); ++it) {
            if ((*it)->Pause() != Status::OK) {
            }
        }
        return Status::OK;
    });
    return Status::OK;
}

Status Pipeline::Resume()
{
    SubmitJobOnce([&] {
        AutoLock lock(mutex_);
        for (auto it = filters_.begin(); it != filters_.end(); ++it) {
            auto rtv = (*it)->Resume();
            FALSE_RETURN_V(rtv == Status::OK, rtv);
        }
        state_ = FilterState::RUNNING;
        return Status::OK;
    });
    return Status::OK;
}

Status Pipeline::Stop()
{
    state_ = FilterState::INITIALIZED;
    SubmitJobOnce([&] {
        AutoLock lock(mutex_);
        for (auto it = filters_.begin(); it != filters_.end(); ++it) {
            if (*it == nullptr) {
                MEDIA_LOG_E("Pipeline error: " PUBLIC_LOG_ZU, filters_.size());
                continue;
            }
            auto rtv = (*it)->Stop();
            FALSE_RETURN_V(rtv == Status::OK, rtv);
        }
        MEDIA_LOG_I("Stop finished, filter number: " PUBLIC_LOG_ZU, filters_.size());
        return Status::OK;
    });
    return Status::OK;
}

Status Pipeline::Flush()
{
    SubmitJobOnce([&] {
        AutoLock lock(mutex_);
        for (auto it = filters_.begin(); it != filters_.end(); ++it) {
            (*it)->Flush();
        }
        return Status::OK;
    });
    return Status::OK;
}

Status Pipeline::Release()
{
    state_ = FilterState::CREATED;
    SubmitJobOnce([&] {
        AutoLock lock(mutex_);
        for (auto it = filters_.begin(); it != filters_.end(); ++it) {
            (*it)->Release();
        }
        filters_.clear();
        return Status::OK;
    });
    return Status::OK;
}

Status Pipeline::AddHeadFilters(std::vector<std::shared_ptr<Filter>> filtersIn)
{
    std::vector<std::shared_ptr<Filter>> filtersToAdd;
    for (auto& filterIn : filtersIn) {
        bool matched = false;
        for (const auto& filter : filters_) {
            if (filterIn == filter) {
                matched = true;
                break;
            }
        }
        if (!matched) {
            filtersToAdd.push_back(filterIn);
        }
    }
    if (filtersToAdd.empty()) {
        MEDIA_LOG_I("filter already exists");
        return Status::OK;
    }
    SubmitJobOnce([&] {
        AutoLock lock(mutex_);
        this->filters_.insert(this->filters_.end(), filtersToAdd.begin(), filtersToAdd.end());
    });
    return Status::OK;
}

Status Pipeline::RemoveHeadFilter(const std::shared_ptr<Filter>& filter)
{
    SubmitJobOnce([&] {
        AutoLock lock(mutex_);
        auto it = std::find_if(filters_.begin(), filters_.end(),
                               [&filter](const std::shared_ptr<Filter>& filterPtr) { return filterPtr == filter; });
        if (it != filters_.end()) {
            filters_.erase(it);
        }
        filter->Release();
        return Status::OK;
    });
    return Status::OK;
}

Status Pipeline::LinkFilters(const std::shared_ptr<Filter> &preFilter,
                             const std::vector<std::shared_ptr<Filter>> &nextFilters,
                             StreamType type) {
    for (auto nextFilter : nextFilters) {
        preFilter->LinkNext(nextFilter, type);
    }
    return Status::OK;
}

Status Pipeline::UpdateFilters(const std::shared_ptr<Filter> &preFilter,
                               const std::vector<std::shared_ptr<Filter>> &nextFilters,
                               StreamType type) {
    for (auto nextFilter : nextFilters) {
        preFilter->UpdateNext(nextFilter, type);
    }
    return Status::OK;
}

Status Pipeline::UnLinkFilters(const std::shared_ptr<Filter> &preFilter,
                               const std::vector<std::shared_ptr<Filter>> &nextFilters,
                               StreamType type) {
    for (auto nextFilter : nextFilters) {
        preFilter->UnLinkNext(nextFilter, type);
    }
    return Status::OK;
}

void Pipeline::OnEvent(const Event& event)
{
}

} // namespace Pipeline
} // namespace Media
} // namespace OHOS
