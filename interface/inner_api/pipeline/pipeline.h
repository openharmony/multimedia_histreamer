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

#ifndef HISTREAMER_PIPELINE_CORE_PIPELINE_H
#define HISTREAMER_PIPELINE_CORE_PIPELINE_H

#include <string>
#include <vector>
#include <memory>
#include "common/status.h"
#include "filter/filter.h"
#include "osal/task/mutex.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
class FilterCallback;

class Pipeline : public EventReceiver {
public:
    ~Pipeline();

    void Init(const std::shared_ptr<EventReceiver>& receiver, const std::shared_ptr<FilterCallback>& callback);

    Status Prepare();

    Status Start();

    Status Pause();

    Status Resume();

    Status Stop();

    Status Flush();

    Status Release();

    Status AddHeadFilters(std::vector<std::shared_ptr<Filter>> filters);

    Status RemoveHeadFilter(const std::shared_ptr<Filter>& filter);

    Status LinkFilters(const std::shared_ptr<Filter>& preFilter,
                       const std::vector<std::shared_ptr<Filter>>& filters, StreamType type);

    Status UpdateFilters(const std::shared_ptr<Filter>& preFilter,
                         const std::vector<std::shared_ptr<Filter>>& filters, StreamType type);

    Status UnLinkFilters(const std::shared_ptr<Filter>& preFilter,
                         const std::vector<std::shared_ptr<Filter>>& filters, StreamType type);

    void OnEvent(const Event& event) override;
private:
    FilterState state_ {FilterState::CREATED};
    Mutex mutex_ {};
    std::vector<std::shared_ptr<Filter>> filters_ {};
    std::shared_ptr<EventReceiver> eventReceiver_ {nullptr};
    std::shared_ptr<FilterCallback> filterCallback_ {nullptr};
};
} // namespace Pipeline
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PIPELINE_CORE_PIPELINE_H
