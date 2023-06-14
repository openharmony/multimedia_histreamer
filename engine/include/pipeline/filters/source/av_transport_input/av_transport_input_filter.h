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

#ifndef MEDIA_PIPELINE_INPUT_FILTER_H
#define MEDIA_PIPELINE_INPUT_FILTER_H

#include <memory>
#include <string>

#include "foundation/osal/utils/util.h"
#include "foundation/utils/constants.h"
#include "pipeline/core/error_code.h"
#include "pipeline/core/filter_base.h"
#include "plugin/common/plugin_video_tags.h"
#include "plugin/core/plugin_manager.h"
#include "plugin/interface/source_plugin.h"
#include "pipeline/core/type_define.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
class AVInputFilter : public FilterBase {
public:
    explicit AVInputFilter(const std::string& name);
    ~AVInputFilter() override;

    std::vector<WorkMode> GetWorkModes() override;
    ErrorCode SetParameter(int32_t key, const Plugin::Any& value) override;
    ErrorCode GetParameter(int32_t key, Plugin::Any& value) override;
    ErrorCode Prepare() override;
    ErrorCode Start() override;
    ErrorCode Stop() override;
    ErrorCode Pause() override;
    ErrorCode Resume() override;
    ErrorCode PushData(const std::string& inPort, const AVBufferPtr& buffer, int64_t offset) override;

private:
    void InitPorts() override;
    ErrorCode FindPlugin();
    ErrorCode DoNegotiate(const CapabilitySet& outCaps);
    ErrorCode CreatePlugin(const std::shared_ptr<Plugin::PluginInfo>& selectedInfo);
    ErrorCode DoConfigure();
    ErrorCode MergeMeta(const Plugin::Meta& meta, Plugin::Meta& targetMeta);
    ErrorCode ConfigMeta(Plugin::Meta& meta);
    ErrorCode ConfigVideoMeta(Plugin::Meta& meta);
    ErrorCode ConfigAudioMeta(Plugin::Meta& meta);
    ErrorCode ConfigDownStream(const Plugin::Meta& meta);
    ErrorCode InitPlugin();
    ErrorCode ConfigPlugin();
    ErrorCode PreparePlugin();
    ErrorCode SetEventCallBack();
    ErrorCode SetDataCallBack();
    ErrorCode SetPluginParams();
    void OnDataCallback(std::shared_ptr<Plugin::Buffer> buffer);
    OHOS::Media::Plugin::AudioChannelLayout TransAudioChannelLayout(int layout);
    OHOS::Media::Plugin::AudioSampleFormat TransAudioSampleFormat(int sampleFormat);

    std::shared_ptr<Plugin::AvTransInput> plugin_ {nullptr};
    std::shared_ptr<Plugin::PluginInfo> pluginInfo_ {nullptr};
    Capability capNegWithDownstream_ {};
    std::unordered_map<Plugin::Tag, Plugin::Any> paramsMap_;
    OSAL::Mutex inputFilterMutex_ {};
};
} // namespace Pipeline
} // namespace Media
} // namespace OHOS
#endif