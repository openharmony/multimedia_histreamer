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

#define HST_LOG_TAG "AudioCaptureFilter"

#include "video_capture_filter.h"
#include "foundation/log.h"
#include "compatible_check.h"
#include "factory/filter_factory.h"
#include "plugin/interface/source_plugin.h"
#include "plugin/core/plugin_meta.h"
#include "common/plugin_utils.h"
#include "utils/type_define.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
using namespace Plugin;

static AutoRegisterFilter<VideoCaptureFilter> g_registerFilterHelper("builtin.recorder.videocapture");

VideoCaptureFilter::VideoCaptureFilter(const std::string& name)
    : FilterBase(name),
      taskPtr_(nullptr),
      plugin_(nullptr),
      pluginAllocator_(nullptr),
      pluginInfo_(nullptr)
{
    filterType_ = FilterType::CAPTURE_SOURCE;
    MEDIA_LOG_D("ctor called");
}

VideoCaptureFilter::~VideoCaptureFilter()
{
    MEDIA_LOG_D("dtor called");
    if (taskPtr_) {
        taskPtr_->Stop();
    }
    if (plugin_) {
        plugin_->Deinit();
    }
}

std::vector<WorkMode> VideoCaptureFilter::GetWorkModes()
{
    return {WorkMode::PUSH};
}

ErrorCode VideoCaptureFilter::InitPlugin()
{
    MEDIA_LOG_D("IN");
    ErrorCode err = TranslatePluginStatus(plugin_->Init());
    if (err != ErrorCode::SUCCESS) {
        return err;
    }
    plugin_->SetCallback(this);
    pluginAllocator_ = plugin_->GetAllocator();
    return err;
}

ErrorCode VideoCaptureFilter::SetParameter(int32_t key, const Plugin::Any& value)
{
    return ErrorCode::SUCCESS;
}

ErrorCode VideoCaptureFilter::GetParameter(int32_t key, Plugin::Any& value)
{
    return ErrorCode::SUCCESS;
}

ErrorCode VideoCaptureFilter::Prepare()
{
    MEDIA_LOG_I("Prepare entered.");
    if (!taskPtr_) {
        taskPtr_ = std::make_shared<OSAL::Task>("DataReader");
        taskPtr_->RegisterHandler(std::bind(&VideoCaptureFilter::ReadLoop, this));
    }
    if (plugin_ == nullptr) {
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    auto err = TranslatePluginStatus(plugin_->Prepare());
    if (err == ErrorCode::SUCCESS) {
        MEDIA_LOG_D("media source send EVENT_READY");
        OnEvent(Event{name_, EventType::EVENT_READY, {}});
    }
    return err;
}

ErrorCode VideoCaptureFilter::Start()
{
    MEDIA_LOG_I("Start entered.");
    if (taskPtr_) {
        taskPtr_->Start();
    }
    return plugin_ ? TranslatePluginStatus(plugin_->Start()) : ErrorCode::ERROR_INVALID_OPERATION;
}

ErrorCode VideoCaptureFilter::Stop()
{
    MEDIA_LOG_I("Stop entered.");
    if (taskPtr_) {
        taskPtr_->Stop();
    }
    ErrorCode ret = ErrorCode::SUCCESS;
    if (plugin_) {
        ret = TranslatePluginStatus(plugin_->Stop());
    }
    return ret;
}

ErrorCode VideoCaptureFilter::SendEos()
{
    MEDIA_LOG_I("SendEos entered.");
    return ErrorCode::SUCCESS;
}

void VideoCaptureFilter::InitPorts()
{
    MEDIA_LOG_D("IN");
    auto outPort = std::make_shared<OutPort>(this);
    outPorts_.push_back(outPort);
}

void VideoCaptureFilter::ReadLoop()
{
}

ErrorCode VideoCaptureFilter::CreatePlugin(const std::shared_ptr<PluginInfo>& info, const std::string& name,
                                           PluginManager& manager)
{
    if ((plugin_ != nullptr) && (pluginInfo_ != nullptr)) {
        if (info->name == pluginInfo_->name && TranslatePluginStatus(plugin_->Reset()) == ErrorCode::SUCCESS) {
            MEDIA_LOG_I("Reuse last plugin: %s", name.c_str());
            return ErrorCode::SUCCESS;
        }
        if (TranslatePluginStatus(plugin_->Deinit()) != ErrorCode::SUCCESS) {
            MEDIA_LOG_E("Deinit last plugin: %s error", pluginInfo_->name.c_str());
        }
    }
    plugin_ = manager.CreateSourcePlugin(name);
    if (plugin_ == nullptr) {
        MEDIA_LOG_E("PluginManager CreatePlugin %s fail", name.c_str());
        return ErrorCode::ERROR_UNKNOWN;
    }
    pluginInfo_ = info;
    MEDIA_LOG_I("Create new plugin: \"%s\" success", pluginInfo_->name.c_str());
    return ErrorCode::SUCCESS;
}
} // namespace Pipeline
} // namespace Media
} // namespace OHOS

#endif