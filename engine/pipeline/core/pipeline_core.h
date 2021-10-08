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

#include "filter_base.h"
#include "foundation/error_code.h"
#include "foundation/meta.h"
#include "foundation/utils.h"
#include "pipeline.h"
#include "plugin/common/plugin_types.h"
#include "thread/mutex.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
class MetaBundle {
public:
    MetaBundle() = default;
    ~MetaBundle() = default;
    std::shared_ptr<const Meta> GetGlobalMeta()
    {
        return globalMeta_;
    }

    std::shared_ptr<const Meta> GetStreamMeta(int32_t streamIndex);

    void UpdateGlobalMeta(const Meta& meta);

    void UpdateStreamMeta(const Meta& meta);

private:
    std::shared_ptr<Meta> globalMeta_;
    std::vector<std::shared_ptr<Meta>> streamMeta_;
};

class PipelineCore : public Pipeline {
public:
    explicit PipelineCore(const std::string& name = "pipeline_core");

    ~PipelineCore() override = default;

    const std::string& GetName() override;

    const EventReceiver* GetOwnerPipeline() const override;

    void Init(EventReceiver* receiver, FilterCallback* callback) override;

    ErrorCode Prepare() override;

    ErrorCode Start() override;

    ErrorCode Pause() override;

    ErrorCode Resume() override;

    ErrorCode Stop() override;

    void FlushStart() override;

    void FlushEnd() override;

    FilterState GetState();

    ErrorCode AddFilters(std::initializer_list<Filter*> filters) override;
    ErrorCode RemoveFilter(Filter* filter) override;
    ErrorCode RemoveFilterChain(Filter* firstFilter) override;
    ErrorCode LinkFilters(std::initializer_list<Filter*> filters) override;
    ErrorCode LinkPorts(std::shared_ptr<OutPort> port1, std::shared_ptr<InPort> port2) override;

    void OnEvent(Event event) override;

    void UnlinkPrevFilters() override
    {
    }
    std::vector<Filter*> GetNextFilters() override
    {
        return {};
    }
    ErrorCode PushData(const std::string& inPort, AVBufferPtr buffer) override
    {
        UNUSED_VARIABLE(inPort);
        UNUSED_VARIABLE(buffer);
        return UNIMPLEMENT;
    }
    ErrorCode PullData(const std::string& outPort, uint64_t offset, size_t size, AVBufferPtr& data) override
    {
        UNUSED_VARIABLE(outPort);
        UNUSED_VARIABLE(offset);
        UNUSED_VARIABLE(size);
        UNUSED_VARIABLE(data);
        return UNIMPLEMENT;
    }
    std::vector<WorkMode> GetWorkModes() override
    {
        return {WorkMode::PUSH};
    }

    PInPort GetInPort(const std::string& name) override
    {
        UNUSED_VARIABLE(name);
        return nullptr;
    }
    POutPort GetOutPort(const std::string& name) override
    {
        UNUSED_VARIABLE(name);
        return nullptr;
    }

    ErrorCode SetParameter(int32_t key, const Plugin::Any& value) override
    {
        UNUSED_VARIABLE(key);
        UNUSED_VARIABLE(value);
        return UNIMPLEMENT;
    }
    ErrorCode GetParameter(int32_t key, Plugin::Any& value) override
    {
        UNUSED_VARIABLE(key);
        UNUSED_VARIABLE(value);
        return UNIMPLEMENT;
    }

    void InitFilters(const std::vector<Filter*>& filters);

    std::shared_ptr<MetaBundle> GetMetaBundle()
    {
        return metaBundle_;
    }

private:
    std::string name_;
    size_t readyEventCnt_ {0};
    FilterState state_ {FilterState::CREATED};
    OSAL::Mutex mutex_ {};
    std::vector<Filter*> filters_ {};
    EventReceiver* eventReceiver_;
    FilterCallback* filterCallback_;
    std::shared_ptr<MetaBundle> metaBundle_;
    std::vector<Filter*> filtersToRemove_ {};
};
} // namespace Pipeline
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PIPELINE_CORE_PIPELINE_CORE_H
