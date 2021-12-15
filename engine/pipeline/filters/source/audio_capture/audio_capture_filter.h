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

#ifdef RECORDER_SUPPORT

#ifndef MEDIA_PIPELINE_AUDIO_CAPTURE_FILTER_H
#define MEDIA_PIPELINE_AUDIO_CAPTURE_FILTER_H

#include <memory>
#include <string>

#include "foundation/error_code.h"
#include "osal/thread/task.h"
#include "osal/utils/util.h"
#include "utils/constants.h"
#include "utils/type_define.h"
#include "utils/utils.h"
#include "pipeline/core/filter_base.h"
#include "plugin/core/plugin_manager.h"
#include "plugin/interface/source_plugin.h"
#include "plugin/common/plugin_audio_tags.h"

namespace OHOS {
namespace Media {
namespace Pipeline {

class AudioCaptureFilter : public FilterBase {
public:
    explicit AudioCaptureFilter(const std::string& name);
    ~AudioCaptureFilter() override;

    std::vector<WorkMode> GetWorkModes() override;
    virtual ErrorCode SetAudioSource(OHOS::Media::Plugin::AudioSourceType source, int32_t &sourceId);
    ErrorCode SetParameter(int32_t key, const Plugin::Any& value) override;
    ErrorCode GetParameter(int32_t key, Plugin::Any& value) override;
    ErrorCode Prepare() override;
    ErrorCode Start() override;
    ErrorCode Stop() override;

private:
    void InitPorts() override;
    ErrorCode InitPlugin();
    void ReadLoop();
    ErrorCode CreatePlugin(const std::shared_ptr<Plugin::PluginInfo>& info, const std::string& name,
                           Plugin::PluginManager& manager);

    std::shared_ptr<OSAL::Task> taskPtr_ {nullptr};
    std::shared_ptr<Plugin::Source> plugin_ {nullptr};
    std::shared_ptr<Allocator> pluginAllocator_ {nullptr};
    std::shared_ptr<Plugin::PluginInfo> pluginInfo_ {nullptr};
};
} // namespace Pipeline
} // namespace Media
} // namespace OHOS

#endif

#endif