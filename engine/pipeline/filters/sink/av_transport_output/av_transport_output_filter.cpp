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

#define HST_LOG_TAG "AVOutputFilter"
#include "pipeline/filters/sink/av_transport_output/av_transport_output_filter.h"
#include "pipeline/filters/common/plugin_utils.h"
#include "foundation/log.h"
#include "pipeline/factory/filter_factory.h"
#include "plugin/common/plugin_attr_desc.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
using namespace Plugin;

static AutoRegisterFilter<AVOutputFilter> g_registerFilterHelper("builtin.avtransport.avoutput");

AVOutputFilter::AVOutputFilter(const std::string& name) : FilterBase(name), plugin_(nullptr), pluginInfo_(nullptr)
{
    MEDIA_LOG_I("ctor called");
    filterType_ = FilterType::AV_OUTPUT;
}

AVOutputFilter::~AVOutputFilter()
{
    MEDIA_LOG_I("dtor called");
    OSAL::ScopedLock lock(outputFilterMutex_);
    if (plugin_ != nullptr) {
        plugin_->Deinit();
    }
}

std::vector<WorkMode> AVOutputFilter::GetWorkModes()
{
    return {WorkMode::PUSH};
}

ErrorCode AVOutputFilter::SetParameter(int32_t key, const Any& value)
{
    MEDIA_LOG_I("SetParameter key " PUBLIC_LOG_D32, key);
    Tag tag;
    if (!TranslateIntoParameter(key, tag)) {
        MEDIA_LOG_E("This key is invalid!");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    {
        OSAL::ScopedLock lock(outputFilterMutex_);
        paramsMap_[tag] = value;
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AVOutputFilter::GetParameter(int32_t key, Any& value)
{
    Tag tag;
    if (!TranslateIntoParameter(key, tag)) {
        MEDIA_LOG_E("This key is invalid!");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    {
        OSAL::ScopedLock lock(outputFilterMutex_);
        value = paramsMap_[tag];
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AVOutputFilter::Prepare()
{
    MEDIA_LOG_I("Prepare entered.");
    if (state_ != FilterState::INITIALIZED) {
        MEDIA_LOG_E("The current state is invalid");
        return ErrorCode::ERROR_INVALID_STATE;
    }
    state_ = FilterState::PREPARING;
    ErrorCode err = FindPlugin();
    if (err != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("Find plugin fail");
        state_ = FilterState::INITIALIZED;
        return err;
    }
    err = InitPlugin();
    if (err != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("Init plugin fail");
        state_ = FilterState::INITIALIZED;
        return err;
    }
    err = ConfigPlugin();
    if (err != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("Configure downStream fail");
        state_ = FilterState::INITIALIZED;
        return err;
    }
    err = PreparePlugin();
    if (err != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("Prepare plugin fail");
        state_ = FilterState::INITIALIZED;
        return err;
    }
    state_ = FilterState::READY;
    MEDIA_LOG_I("Prepare end.");
    return err;
}

ErrorCode AVOutputFilter::Start()
{
    MEDIA_LOG_I("Start");
    OSAL::ScopedLock lock(outputFilterMutex_);
    if (state_ != FilterState::READY && state_ != FilterState::PAUSED) {
        MEDIA_LOG_E("The current state is invalid");
        return ErrorCode::ERROR_INVALID_STATE;
    }
    if (plugin_ == nullptr) {
        MEDIA_LOG_E("plugin is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (TranslatePluginStatus(plugin_->Start()) != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("The plugin start fail!");
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    state_ = FilterState::RUNNING;
    return ErrorCode::SUCCESS;
}

ErrorCode AVOutputFilter::Stop()
{
    MEDIA_LOG_I("Stop");
    OSAL::ScopedLock lock(outputFilterMutex_);
    if (state_ != FilterState::RUNNING) {
        MEDIA_LOG_E("The current state is invalid");
        return ErrorCode::ERROR_INVALID_STATE;
    }
    if (plugin_ == nullptr) {
        MEDIA_LOG_E("plugin is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (TranslatePluginStatus(plugin_->Stop()) != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("The plugin stop fail!");
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    state_ = FilterState::READY;
    return ErrorCode::SUCCESS;
}

ErrorCode AVOutputFilter::Pause()
{
    MEDIA_LOG_I("Pause");
    OSAL::ScopedLock lock(outputFilterMutex_);
    if (state_ != FilterState::RUNNING) {
        MEDIA_LOG_E("The current state is invalid");
        return ErrorCode::ERROR_INVALID_STATE;
    }
    if (plugin_ == nullptr) {
        MEDIA_LOG_E("plugin is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (TranslatePluginStatus(plugin_->Stop()) != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("The plugin stop fail!");
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    state_ = FilterState::PAUSED;
    return ErrorCode::SUCCESS;
}

ErrorCode AVOutputFilter::Resume()
{
    MEDIA_LOG_I("Resume");
    return ErrorCode::SUCCESS;
}

void AVOutputFilter::InitPorts()
{
    MEDIA_LOG_I("InitPorts");
    auto inPort = std::make_shared<InPort>(this);
    {
        OSAL::ScopedLock lock(outputFilterMutex_);
        inPorts_.push_back(inPort);
    }
}

ErrorCode AVOutputFilter::FindPlugin()
{
    OSAL::ScopedLock lock(outputFilterMutex_);
    std::string mime;
    if (paramsMap_.find(Tag::MIME) == paramsMap_.end() ||
        !paramsMap_[Tag::MIME].SameTypeWith(typeid(std::string))) {
        MEDIA_LOG_E("Must set mime correctly first");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    mime = Plugin::AnyCast<std::string>(paramsMap_[Tag::MIME]);
    auto nameList = PluginManager::Instance().ListPlugins(PluginType::AVTRANS_OUTPUT);
    for (const std::string& name : nameList) {
        auto info = PluginManager::Instance().GetPluginInfo(PluginType::AVTRANS_OUTPUT, name);
        if (mime != info->inCaps[0].mime) {
            continue;
        }
        if (CreatePlugin(info) == ErrorCode::SUCCESS) {
            MEDIA_LOG_I("CreatePlugin " PUBLIC_LOG_S " success", name_.c_str());
            return ErrorCode::SUCCESS;
        }
    }
    MEDIA_LOG_I("Cannot find any plugin");
    return ErrorCode::ERROR_UNSUPPORTED_FORMAT;
}

bool AVOutputFilter::Negotiate(const std::string& inPort, const std::shared_ptr<const Plugin::Capability>& upstreamCap,
    Plugin::Capability& negotiatedCap, const Plugin::Meta& upstreamParams, Plugin::Meta& downstreamParams)
{
    MEDIA_LOG_I("Negotiate");
    if (pluginInfo_ == nullptr) {
        MEDIA_LOG_E("pluginInfo_ is nullptr");
        return false;
    }
    negotiatedCap = pluginInfo_->inCaps[0];
    return true;
}

ErrorCode AVOutputFilter::CreatePlugin(const std::shared_ptr<PluginInfo>& selectedInfo)
{
    MEDIA_LOG_I("CreatePlugin");
    if (selectedInfo == nullptr || selectedInfo->name.empty()) {
        MEDIA_LOG_E("selectedInfo is nullptr or pluginName is invalid!");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    if ((plugin_ != nullptr) && (pluginInfo_ != nullptr)) {
        if (selectedInfo->name == pluginInfo_->name && TranslatePluginStatus(plugin_->Reset()) == ErrorCode::SUCCESS) {
            MEDIA_LOG_I("Reuse last plugin: " PUBLIC_LOG_S, selectedInfo->name.c_str());
            return ErrorCode::SUCCESS;
        }
        if (TranslatePluginStatus(plugin_->Deinit()) != ErrorCode::SUCCESS) {
            MEDIA_LOG_E("Deinit last plugin: " PUBLIC_LOG_S " error", pluginInfo_->name.c_str());
        }
    }
    plugin_ = PluginManager::Instance().CreateAvTransOutputPlugin(selectedInfo->name);
    if (plugin_ == nullptr) {
        MEDIA_LOG_E("PluginManager CreatePlugin " PUBLIC_LOG_S " fail", selectedInfo->name.c_str());
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    pluginInfo_ = selectedInfo;
    MEDIA_LOG_I("Create new plugin: " PUBLIC_LOG_S " success", pluginInfo_->name.c_str());
    return ErrorCode::SUCCESS;
}

bool AVOutputFilter::Configure(const std::string& inPort, const std::shared_ptr<const Plugin::Meta>& upstreamMeta,
    Plugin::Meta& upstreamParams, Plugin::Meta& downstreamParams)
{
    MEDIA_LOG_I("DoConfigure");
    return true;
}

ErrorCode AVOutputFilter::InitPlugin()
{
    MEDIA_LOG_I("InitPlugin");
    OSAL::ScopedLock lock(outputFilterMutex_);
    if (plugin_ == nullptr) {
        MEDIA_LOG_E("plugin is nullptr!");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    return TranslatePluginStatus(plugin_->Init());
}

ErrorCode AVOutputFilter::ConfigPlugin()
{
    MEDIA_LOG_I("Configure");
    ErrorCode err = SetPluginParams();
    if (err != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("Set Plugin fail!");
        return err;
    }
    err = SetEventCallBack();
    if (err != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("Plugin SetEventCallBack fail!");
        return err;
    }
    err = SetDataCallBack();
    if (err != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("Plugin SetDataCallBack fail!");
        return err;
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AVOutputFilter::PreparePlugin()
{
    OSAL::ScopedLock lock(outputFilterMutex_);
    if (plugin_ == nullptr) {
        MEDIA_LOG_E("plugin is nullptr!");
        return ErrorCode::ERROR_INVALID_PARAMETER_TYPE;
    }
    return TranslatePluginStatus(plugin_->Prepare());
}

ErrorCode AVOutputFilter::PushData(const std::string& inPort, const AVBufferPtr& buffer, int64_t offset)
{
    OSAL::ScopedLock lock(outputFilterMutex_);
    if (buffer == nullptr || plugin_ == nullptr) {
        MEDIA_LOG_E("buffer or plugin is nullptr!");
        return ErrorCode::ERROR_INVALID_PARAMETER_TYPE;
    }
    plugin_->PushData(inPort, buffer, offset);
    MEDIA_LOG_E("push buffer to plugin.");
    return ErrorCode::SUCCESS;
}

ErrorCode AVOutputFilter::SetPluginParams()
{
    OSAL::ScopedLock lock(outputFilterMutex_);
    if (plugin_ == nullptr) {
        MEDIA_LOG_E("plugin is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (paramsMap_.find(Tag::MEDIA_DESCRIPTION) != paramsMap_.end()) {
        plugin_->SetParameter(Tag::MEDIA_DESCRIPTION, paramsMap_[Tag::MEDIA_DESCRIPTION]);
    }
    if (paramsMap_.find(Tag::AUDIO_CHANNELS) != paramsMap_.end()) {
        plugin_->SetParameter(Tag::AUDIO_CHANNELS, paramsMap_[Tag::AUDIO_CHANNELS]);
    }
    if (paramsMap_.find(Tag::AUDIO_SAMPLE_RATE) != paramsMap_.end()) {
        plugin_->SetParameter(Tag::AUDIO_SAMPLE_RATE, paramsMap_[Tag::AUDIO_SAMPLE_RATE]);
    }
    if (paramsMap_.find(Tag::AUDIO_CHANNEL_LAYOUT) != paramsMap_.end()) {
        plugin_->SetParameter(Tag::AUDIO_CHANNEL_LAYOUT, paramsMap_[Tag::AUDIO_CHANNEL_LAYOUT]);
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AVOutputFilter::SetEventCallBack()
{
    OSAL::ScopedLock lock(outputFilterMutex_);
    if (plugin_ == nullptr) {
        MEDIA_LOG_E("plugin is nullptr!");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    plugin_->SetCallback(this);
    return ErrorCode::SUCCESS;
}

ErrorCode AVOutputFilter::SetDataCallBack()
{
    OSAL::ScopedLock lock(outputFilterMutex_);
    if (plugin_ == nullptr) {
        MEDIA_LOG_E("plugin is nullptr!");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    plugin_->SetDataCallback(std::bind(&AVOutputFilter::OnDataCallback, this, std::placeholders::_1));
    return ErrorCode::SUCCESS;
}

void AVOutputFilter::OnDataCallback(std::shared_ptr<Plugin::Buffer> buffer)
{
    OSAL::ScopedLock lock(outputFilterMutex_);
    if (buffer == nullptr) {
        MEDIA_LOG_E("buffer is nullptr!");
        return;
    }
    OnEvent(Event{name_, EventType::EVENT_BUFFER_PROGRESS, buffer});
}
} // namespace Pipeline
} // namespace Media
} // namespace OHOS