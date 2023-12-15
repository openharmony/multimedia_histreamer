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

#define HST_LOG_TAG "Filter"

#include "filter/filter.h"
#include <algorithm>

namespace OHOS {
namespace Media {
namespace Pipeline {
Filter::Filter(std::string name, FilterType type)
    : name_(std::move(name)), filterType_(std::move(type))
{
}

void Filter::Init(const std::shared_ptr<EventReceiver>& receiver, const std::shared_ptr<FilterCallback>& callback)
{
    receiver_ = receiver;
    callback_ = callback;
}

Status Filter::Prepare()
{
    for (auto iter : nextFiltersMap_) {
        for (auto filter : iter.second) {
            filter->Prepare();
        }
    }
    return Status::OK;
}

Status Filter::Start()
{
    for (auto iter : nextFiltersMap_) {
        for (auto filter : iter.second) {
            filter->Start();
        }
    }
    return Status::OK;
}

Status Filter::Pause()
{
    for (auto iter : nextFiltersMap_) {
        for (auto filter : iter.second) {
            filter->Pause();
        }
    }
    return Status::OK;
}

Status Filter::Resume()
{
    for (auto iter : nextFiltersMap_) {
        for (auto filter : iter.second) {
            filter->Resume();
        }
    }
    return Status::OK;
}

Status Filter::Stop()
{
    for (auto iter : nextFiltersMap_) {
        for (auto filter : iter.second) {
            filter->Stop();
        }
    }
    return Status::OK;
}

Status Filter::Flush()
{
    for (auto iter : nextFiltersMap_) {
        for (auto filter : iter.second) {
            filter->Flush();
        }
    }
    return Status::OK;
}

Status Filter::Release()
{
    for (auto iter : nextFiltersMap_) {
        for (auto filter : iter.second) {
            filter->Release();
        }
    }
    nextFiltersMap_.clear();
    return Status::OK;
}

void Filter::SetParameter(const std::shared_ptr<Meta>& meta)
{
    meta_ = meta;
}

void Filter::GetParameter(std::shared_ptr<Meta>& meta)
{
    meta = meta_;
}

Status Filter::LinkNext(const std::shared_ptr<Filter>&, StreamType)
{
    return Status::OK;
}

Status Filter::UpdateNext(const std::shared_ptr<Filter>&, StreamType)
{
    return Status::OK;
}

Status Filter::UnLinkNext(const std::shared_ptr<Filter>&, StreamType)
{
    return Status::OK;
}

FilterType Filter::GetFilterType()
{
    return filterType_;
};

Status Filter::OnLinked(StreamType, const std::shared_ptr<Meta>&, const std::shared_ptr<FilterLinkCallback>&)
{
    return Status::OK;
};

Status Filter::OnUpdated(StreamType, const std::shared_ptr<Meta>&, const std::shared_ptr<FilterLinkCallback>&)
{
    return Status::OK;
}

Status Filter::OnUnLinked(StreamType, const std::shared_ptr<FilterLinkCallback>&)
{
    return Status::OK;
}

} // namespace Pipeline
} // namespace Media
} // namespace OHOS
