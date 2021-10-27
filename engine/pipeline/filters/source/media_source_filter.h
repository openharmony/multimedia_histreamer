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

#ifndef MEDIA_PIPELINE_MEDIA_SOURCE_FILTER_H
#define MEDIA_PIPELINE_MEDIA_SOURCE_FILTER_H

#include <memory>
#include <string>

#include "source.h"
#include "foundation/error_code.h"
#include "osal/thread/task.h"
#include "utils/blocking_queue.h"
#include "utils/constants.h"
#include "utils/type_define.h"
#include "utils/utils.h"
#include "pipeline/core/filter_base.h"
#include "plugin/core/plugin_manager.h"
#include "plugin/interface/source_plugin.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
using SourceType = OHOS::Media::SourceType;
using MediaSource = OHOS::Media::Source;

class MediaSourceFilter : public FilterBase {
public:
    explicit MediaSourceFilter(const std::string& name);
    ~MediaSourceFilter() override;

    std::vector<WorkMode> GetWorkModes() override;
    ErrorCode PullData(const std::string& outPort, uint64_t offset, size_t size, AVBufferPtr& data) override;
    virtual ErrorCode SetSource(const std::shared_ptr<MediaSource>& source);
    ErrorCode InitPlugin(const std::shared_ptr<MediaSource>& source);
    virtual ErrorCode SetBufferSize(size_t size);
    bool IsSeekable() const;
    ErrorCode Prepare() override;
    ErrorCode Start() override;
    ErrorCode Stop() override;
    void FlushStart() override;
    void FlushEnd() override;

private:
    void InitPorts() override;
    void ActivateMode();
    static std::string GetUriSuffix(const std::string& uri);
    ErrorCode DoNegotiate(const std::shared_ptr<MediaSource>& source);
    void ReadLoop();
    void ParseProtocol(const std::shared_ptr<MediaSource>& source);
    ErrorCode CreatePlugin(const std::shared_ptr<Plugin::PluginInfo>& info, const std::string& name,
                           Plugin::PluginManager& manager);
    ErrorCode FindPlugin(const std::shared_ptr<MediaSource>& source);

    std::shared_ptr<OSAL::Task> taskPtr_;
    std::string protocol_;
    std::string uri_;
    bool isSeekable_;
    uint64_t position_;
    size_t bufferSize_;
    std::shared_ptr<Plugin::Source> plugin_;
    std::shared_ptr<Allocator> pluginAllocator_;
    std::shared_ptr<Plugin::PluginInfo> pluginInfo_;
};
} // namespace Pipeline
} // namespace Media
} // namespace OHOS

#endif
