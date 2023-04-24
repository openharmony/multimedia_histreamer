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

#ifndef HISTREAMER_PIPELINE_CORE_PIPELINE_CORE_H
#define HISTREAMER_PIPELINE_CORE_PIPELINE_CORE_H

#include <algorithm>
#include <atomic>
#include <functional>
#include <initializer_list>
#include <memory>
#include <string>
#include <vector>
#include <stack>
#include "foundation/osal/thread/mutex.h"
#include "pipeline/core/error_code.h"
#include "pipeline/core/pipeline.h"
#include "plugin/common/plugin_types.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
class PipelineCore : public Pipeline {
public:
    ~PipelineCore() override = default;

    void Init(EventReceiver* receiver, FilterCallback* callback) override;

    ErrorCode Prepare() override;

    ErrorCode Start() override;

    ErrorCode Pause() override;

    ErrorCode Resume() override;

    ErrorCode Stop() override;

    void FlushStart() override;

    void FlushEnd() override;

    void OnEvent(const Event& event) override;

    FilterState GetState();

    ErrorCode AddFilters(std::initializer_list<Filter*> filtersIn) override;
    ErrorCode RemoveFilter(Filter* filter) override;
    ErrorCode RemoveFilterChain(Filter* firstFilter) override;
    ErrorCode LinkFilters(std::initializer_list<Filter*> filters) override;
    ErrorCode LinkPorts(std::shared_ptr<OutPort> port1, std::shared_ptr<InPort> port2) override;

    void InitFilters(const std::vector<Filter*>& filters);
private:
    void ReorderFilters();

    void NotifyEvent(const Event& event);

    size_t readyEventCnt_ {0};
    FilterState state_ {FilterState::CREATED};
    OSAL::Mutex mutex_ {};
    std::vector<Filter*> filters_ {};
    EventReceiver* eventReceiver_ {nullptr};
    FilterCallback* filterCallback_ {nullptr};
    std::vector<Filter*> filtersToRemove_ {};
};
} // namespace Pipeline
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PIPELINE_CORE_PIPELINE_CORE_H
