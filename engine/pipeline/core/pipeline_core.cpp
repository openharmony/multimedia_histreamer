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

#include "pipeline_core.h"
#include <queue>
#include "foundation/log.h"
#include "osal/thread/scoped_lock.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
std::shared_ptr<const Plugin::Meta> OHOS::Media::Pipeline::MetaBundle::GeTrackMeta(int32_t trackId)
{
    for (auto& ptr : trackMeta_) {
        uint32_t found = 0;
        if (ptr->GetUint32(Plugin::MetaID::TRACK_ID, found) && found == trackId) {
            return ptr;
        }
    }
    return nullptr;
}

void MetaBundle::UpdateGlobalMeta(const Plugin::Meta& meta)
{
    if (globalMeta_ == nullptr) {
        globalMeta_ = std::make_shared<Plugin::Meta>();
    }
    globalMeta_->Update(meta);
}

void MetaBundle::UpdateTrackMeta(const Plugin::Meta& meta)
{
    uint32_t trackId = 0;
    if (!meta.GetUint32(Plugin::MetaID::TRACK_ID, trackId)) {
        MEDIA_LOG_W("update stream meta with invalid meta, which contains no track id, will ignore this meta");
        return;
    }
    for (const auto& tmp : trackMeta_) {
        uint32_t tid = 0;
        if (tmp->GetUint32(Plugin::MetaID::TRACK_ID, tid) && trackId == tid) {
            tmp->Update(meta);
            return;
        }
    }
    auto ptr = std::make_shared<Plugin::Meta>();
    ptr->Update(meta);
    trackMeta_.emplace_back(ptr);
}

PipelineCore::PipelineCore(const std::string& name)
    : name_(name), eventReceiver_(nullptr), filterCallback_(nullptr), metaBundle_(std::make_shared<MetaBundle>())
{
}

const std::string& PipelineCore::GetName()
{
    return name_;
}

const EventReceiver* PipelineCore::GetOwnerPipeline() const
{
    return eventReceiver_;
}

void PipelineCore::Init(EventReceiver* receiver, FilterCallback* callback)
{
    eventReceiver_ = receiver;
    filterCallback_ = callback;
    state_ = FilterState::INITIALIZED;
}

ErrorCode PipelineCore::Prepare()
{
    state_ = FilterState::PREPARING;
    ErrorCode rtv = ErrorCode::SUCCESS;
    OSAL::ScopedLock lock(mutex_);
    for (auto it = filters_.rbegin(); it != filters_.rend(); ++it) {
        auto& filterPtr = *it;
        if (filterPtr) {
            if ((rtv = filterPtr->Prepare()) != ErrorCode::SUCCESS) {
                break;
            }
        } else {
            MEDIA_LOG_E("invalid pointer in filters.");
        }
    }
    return rtv;
}

ErrorCode PipelineCore::Start()
{
    state_ = FilterState::RUNNING;
    for (auto it = filters_.rbegin(); it != filters_.rend(); ++it) {
        auto rtv = (*it)->Start();
        FALSE_RETURN_V(rtv == ErrorCode::SUCCESS, rtv);
    }
    return ErrorCode::SUCCESS;
}

ErrorCode PipelineCore::Pause()
{
    if (state_ == FilterState::PAUSED) {
        return ErrorCode::SUCCESS;
    }
    if (state_ != FilterState::READY && state_ != FilterState::RUNNING) {
        return ErrorCode::ERROR_STATE;
    }
    state_ = FilterState::PAUSED;
    for (auto it = filters_.rbegin(); it != filters_.rend(); ++it) {
        if ((*it)->Pause() != ErrorCode::SUCCESS) {
            MEDIA_LOG_I("pause filter: %s", (*it)->GetName().c_str());
        }
    }
    return ErrorCode::SUCCESS;
}

ErrorCode PipelineCore::Resume()
{
    for (auto it = filters_.rbegin(); it != filters_.rend(); ++it) {
        MEDIA_LOG_I("Resume filter: %s", (*it)->GetName().c_str());
        auto rtv = (*it)->Resume();
        FALSE_RETURN_V(rtv == ErrorCode::SUCCESS, rtv);
    }
    state_ = FilterState::RUNNING;
    return ErrorCode::SUCCESS;
}

ErrorCode PipelineCore::Stop()
{
    readyEventCnt_ = 0;
    state_ = FilterState::INITIALIZED;
    filtersToRemove_.clear();
    filtersToRemove_.reserve(filters_.size());
    for (auto it = filters_.rbegin(); it != filters_.rend(); ++it) {
        if (*it == nullptr) {
            MEDIA_LOG_E("PipelineCore error: %zu", filters_.size());
            continue;
        }
        auto filterName = (*it)->GetName();
        auto rtv = (*it)->Stop();
        FALSE_RETURN_V(rtv == ErrorCode::SUCCESS, rtv);
    }
    for (const auto& filter : filtersToRemove_) {
        RemoveFilter(filter);
    }
    MEDIA_LOG_I("Stop finished, filter number: %zu", filters_.size());
    return ErrorCode::SUCCESS;
}

void PipelineCore::FlushStart()
{
    for (auto it = filters_.rbegin(); it != filters_.rend(); ++it) {
        (*it)->FlushStart();
    }
}

void PipelineCore::FlushEnd()
{
    for (auto it = filters_.rbegin(); it != filters_.rend(); ++it) {
        (*it)->FlushEnd();
    }
}

FilterState PipelineCore::GetState()
{
    return state_;
}

ErrorCode PipelineCore::AddFilters(std::initializer_list<Filter*> filtersIn)
{
    std::vector<Filter*> filtersToAdd;
    for (auto& filterIn : filtersIn) {
        bool matched = false;
        for (auto& filter : filters_) {
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
        return ErrorCode::ERROR_ALREADY_EXISTS;
    }
    {
        OSAL::ScopedLock lock(mutex_);
        this->filters_.insert(this->filters_.end(), filtersToAdd.begin(), filtersToAdd.end());
    }
    InitFilters(filtersToAdd);
    return ErrorCode::SUCCESS;
}

ErrorCode PipelineCore::RemoveFilter(Filter* filter)
{
    auto it = std::find_if(filters_.begin(), filters_.end(),
                           [&filter](const Filter* filterPtr) { return filterPtr == filter; });
    ErrorCode rtv = ErrorCode::ERROR_INVALID_PARAM_VALUE;
    if (it != filters_.end()) {
        MEDIA_LOG_I("RemoveFilter %s", (*it)->GetName().c_str());
        filters_.erase(it);
        rtv = ErrorCode::SUCCESS;
    }
    return rtv;
}

ErrorCode PipelineCore::RemoveFilterChain(Filter* firstFilter)
{
    if (!firstFilter) {
        return ErrorCode::ERROR_NULL_POINTER;
    }
    std::queue<Filter*> levelFilters;
    levelFilters.push(firstFilter);
    while (!levelFilters.empty()) {
        auto filter = levelFilters.front();
        levelFilters.pop();
        filter->UnlinkPrevFilters();
        filtersToRemove_.push_back(filter);
        for (auto&& nextFilter : filter->GetNextFilters()) {
            levelFilters.push(nextFilter);
        }
    }
    return ErrorCode::SUCCESS;
}

ErrorCode PipelineCore::LinkFilters(std::initializer_list<Filter*> filters)
{
    std::vector<Filter*> filtersToLink;
    std::vector<Filter*>(filters).swap(filtersToLink);
    int count = std::max((int)(filtersToLink.size()) - 1, 0);
    for (int i = 0; i < count; i++) {
        filtersToLink[i]->GetOutPort(PORT_NAME_DEFAULT)->Connect(filtersToLink[i + 1]->GetInPort(PORT_NAME_DEFAULT));
        filtersToLink[i + 1]->GetInPort(PORT_NAME_DEFAULT)->Connect(filtersToLink[i]->GetOutPort(PORT_NAME_DEFAULT));
    }
    return ErrorCode::SUCCESS;
}

ErrorCode PipelineCore::LinkPorts(std::shared_ptr<OutPort> port1, std::shared_ptr<InPort> port2)
{
    FAIL_RETURN(port1->Connect(port2));
    FAIL_RETURN(port2->Connect(port1));
    return ErrorCode::SUCCESS;
}

void PipelineCore::OnEvent(Event event)
{
    if (event.type != EVENT_READY) {
        CALL_PTR_FUNC(eventReceiver_, OnEvent, event);
        return;
    }

    readyEventCnt_++;
    MEDIA_LOG_I("OnEvent readyCnt: %zu / %zu", readyEventCnt_, filters_.size());
    if (readyEventCnt_ == filters_.size()) {
        CALL_PTR_FUNC(eventReceiver_, OnEvent, event);
        readyEventCnt_ = 0;
    }
}

void PipelineCore::InitFilters(const std::vector<Filter*>& filters)
{
    for (auto& filter : filters) {
        filter->Init(this, filterCallback_);
    }
}
} // namespace Pipeline
} // namespace Media
} // namespace OHOS
