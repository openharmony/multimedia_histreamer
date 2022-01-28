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

#if defined(RECORDER_SUPPORT) && defined(VIDEO_SUPPORT)

#define HST_LOG_TAG "VideoCaptureFilter"

#include "video_capture_filter.h"
#include "foundation/log.h"
#include "factory/filter_factory.h"
#include "common/plugin_utils.h"

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

ErrorCode VideoCaptureFilter::InitAndConfigPlugin(const std::shared_ptr<Plugin::Meta>& audioMeta)
{
    MEDIA_LOG_D("IN");
    ErrorCode err = TranslatePluginStatus(plugin_->Init());
    if (err != ErrorCode::SUCCESS) {
        return err;
    }
    plugin_->SetCallback(this);
    pluginAllocator_ = plugin_->GetAllocator();
    err = TranslatePluginStatus(plugin_->SetParameter(Tag::VIDEO_WIDTH, videoWidth_));
    if (err != ErrorCode::SUCCESS) {
        return err;
    }
    err = TranslatePluginStatus(plugin_->SetParameter(Tag::VIDEO_HEIGHT, videoHeight_));
    if (err != ErrorCode::SUCCESS) {
        return err;
    }
    return ErrorCode::SUCCESS;
}

ErrorCode VideoCaptureFilter::SetParameter(int32_t key, const Plugin::Any& value)
{
#define ASSIGN_PARAMETER_IF_MATCH(type, val, val1) \
do { \
    if (val.Type() == typeid(type)) { \
        val1 = Plugin::AnyCast<type>(val); \
    } \
} while (0)

    auto tag = static_cast<OHOS::Media::Plugin::Tag>(key);
    switch (tag) {
        case Tag::SRC_INPUT_TYPE:
            ASSIGN_PARAMETER_IF_MATCH(Plugin::SrcInputType, value, inputType_);
            break;
        case Tag::VIDEO_WIDTH:
            ASSIGN_PARAMETER_IF_MATCH(uint32_t, value, videoWidth_);
            break;
        case Tag::VIDEO_HEIGHT:
            ASSIGN_PARAMETER_IF_MATCH(uint32_t, value, videoHeight_);
            break;
        case Tag::VIDEO_FRAME_RATE:
            ASSIGN_PARAMETER_IF_MATCH(uint64_t, value, frameRate_);
            break;
        default:
            MEDIA_LOG_W("Unknown key %" PUBLIC_OUTPUT "d", OHOS::Media::to_underlying(tag));
            break;
    }
    return ErrorCode::SUCCESS;
#undef ASSIGN_PARAMETER_IF_MATCH
}

ErrorCode VideoCaptureFilter::GetParameter(int32_t key, Plugin::Any& value)
{
    Tag tag = static_cast<Plugin::Tag>(key);
    switch (tag) {
        case Tag::SRC_INPUT_TYPE: {
            value = inputType_;
            break;
        }
        case Tag::VIDEO_WIDTH: {
            value = videoWidth_;
            break;
        }
        case Tag::VIDEO_HEIGHT: {
            value = videoHeight_;
            break;
        }
        case Tag::VIDEO_FRAME_RATE: {
            value = frameRate_;
            break;
        }
        default:
            MEDIA_LOG_I("Unknown key %" PUBLIC_OUTPUT "d", tag);
            break;
    }
    return ErrorCode::SUCCESS;
}

ErrorCode VideoCaptureFilter::DoConfigure()
{
    auto emptyMeta = std::make_shared<Plugin::Meta>();
    auto audioMeta = std::make_shared<Plugin::Meta>();
    if (!MergeMetaWithCapability(*emptyMeta, capNegWithDownstream_, *audioMeta)) {
        MEDIA_LOG_E("cannot find available capability of plugin %" PUBLIC_OUTPUT "s", pluginInfo_->name.c_str());
        return ErrorCode::ERROR_UNKNOWN;
    }
    if (!outPorts_[0]->Configure(audioMeta)) {
        MEDIA_LOG_E("Configure downstream fail");
        return ErrorCode::ERROR_UNKNOWN;
    }
    return InitAndConfigPlugin(audioMeta);
}

ErrorCode VideoCaptureFilter::Prepare()
{
    MEDIA_LOG_I("Prepare entered.");
    if (!taskPtr_) {
        taskPtr_ = std::make_shared<OSAL::Task>("DataReader");
        taskPtr_->RegisterHandler([this] { ReadLoop(); });
    }
    ErrorCode err = FindPlugin();
    if (err != ErrorCode::SUCCESS || !plugin_) {
        MEDIA_LOG_E("Find plugin fail");
        return err;
    }
    err = DoConfigure();
    if (err != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("DoConfigure fail");
        return err;
    }
    err = TranslatePluginStatus(plugin_->Prepare());
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

ErrorCode VideoCaptureFilter::Pause()
{
    MEDIA_LOG_I("Pause entered.");
    if (taskPtr_) {
        taskPtr_->PauseAsync();
    }
    ErrorCode ret = ErrorCode::SUCCESS;
    if (plugin_) {
        ret = TranslatePluginStatus(plugin_->Stop());
    }
    return ret;
}

ErrorCode VideoCaptureFilter::Resume()
{
    MEDIA_LOG_I("Resume entered.");
    if (taskPtr_) {
        taskPtr_->Start();
    }
    return plugin_ ? TranslatePluginStatus(plugin_->Start()) : ErrorCode::ERROR_INVALID_OPERATION;
}

ErrorCode VideoCaptureFilter::SendEos()
{
    MEDIA_LOG_I("SendEos entered.");
    auto eosBuffer = std::make_shared<AVBuffer>();
    eosBuffer->flag |= BUFFER_FLAG_EOS;
    SendBuffer(eosBuffer);
    isEos_ = true;
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
    if (isEos_.load()) {
        return;
    }
    size_t bufferSize = 0;
    auto ret = plugin_->GetSize(bufferSize);
    if (ret != Status::OK || bufferSize <= 0) {
        MEDIA_LOG_E("Get plugin buffer size fail");
        return;
    }
    AVBufferPtr bufferPtr = std::make_shared<AVBuffer>(BufferMetaType::VIDEO);
    ret = plugin_->Read(bufferPtr, bufferSize);
    if (ret != Status::OK) {
        SendEos();
        return;
    }
    SendBuffer(bufferPtr);
}

ErrorCode VideoCaptureFilter::CreatePlugin(const std::shared_ptr<PluginInfo>& info, const std::string& name,
                                           PluginManager& manager)
{
    if ((plugin_ != nullptr) && (pluginInfo_ != nullptr)) {
        if (info->name == pluginInfo_->name && TranslatePluginStatus(plugin_->Reset()) == ErrorCode::SUCCESS) {
            MEDIA_LOG_I("Reuse last plugin: %" PUBLIC_OUTPUT "s", name.c_str());
            return ErrorCode::SUCCESS;
        }
        if (TranslatePluginStatus(plugin_->Deinit()) != ErrorCode::SUCCESS) {
            MEDIA_LOG_E("Deinit last plugin: %" PUBLIC_OUTPUT "s error", pluginInfo_->name.c_str());
        }
    }
    plugin_ = manager.CreateSourcePlugin(name);
    if (plugin_ == nullptr) {
        MEDIA_LOG_E("PluginManager CreatePlugin %" PUBLIC_OUTPUT "s fail", name.c_str());
        return ErrorCode::ERROR_UNKNOWN;
    }
    pluginInfo_ = info;
    MEDIA_LOG_I("Create new plugin: \"%" PUBLIC_OUTPUT "s\" success", pluginInfo_->name.c_str());
    return ErrorCode::SUCCESS;
}

bool VideoCaptureFilter::DoNegotiate(const CapabilitySet &outCaps)
{
    if (outCaps.empty()) {
        MEDIA_LOG_E("audio capture plugin must have out caps");
        return false;
    }
    for (const auto& outCap : outCaps) {
        auto thisOut = std::make_shared<Plugin::Capability>();
        *thisOut = outCap;
        Plugin::TagMap upstreamParams;
        Plugin::TagMap downstreamParams;
        upstreamParams.emplace(std::make_pair(Tag::VIDEO_FRAME_RATE, frameRate_));
        if (outPorts_[0]->Negotiate(thisOut, capNegWithDownstream_, upstreamParams, downstreamParams)) {
            MEDIA_LOG_I("Negotiate success");
            return true;
        }
    }
    return false;
}

ErrorCode VideoCaptureFilter::FindPlugin()
{
    if (!inputTypeSpecified_) {
        MEDIA_LOG_E("Must set input type first");
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    PluginManager& pluginManager = PluginManager::Instance();
    std::set<std::string> nameList = pluginManager.ListPlugins(PluginType::SOURCE);
    for (const std::string& name : nameList) {
        std::shared_ptr<PluginInfo> info = pluginManager.GetPluginInfo(PluginType::SOURCE, name);
        MEDIA_LOG_I("name: %" PUBLIC_OUTPUT "s, info->name: %" PUBLIC_OUTPUT "s", name.c_str(), info->name.c_str());
        auto val = info->extra[PLUGIN_INFO_EXTRA_INPUT_TYPE];
        if (val.Type() == typeid(Plugin::SrcInputType)) {
            auto supportInputType = OHOS::Media::Plugin::AnyCast<Plugin::SrcInputType>(val);
            if (inputType_ == supportInputType && DoNegotiate(info->outCaps) &&
                CreatePlugin(info, name, pluginManager) == ErrorCode::SUCCESS) {
                MEDIA_LOG_I("CreatePlugin %" PUBLIC_OUTPUT "s success", name_.c_str());
                return ErrorCode::SUCCESS;
            }
        }
    }
    MEDIA_LOG_I("Cannot find any plugin");
    return ErrorCode::ERROR_UNSUPPORTED_FORMAT;
}

void VideoCaptureFilter::SendBuffer(const std::shared_ptr<AVBuffer>& buffer)
{
    OSAL::ScopedLock lock(pushMutex_);
    if (!isEos_.load()) {
        outPorts_[0]->PushData(buffer, -1);
    }
}
} // namespace Pipeline
} // namespace Media
} // namespace OHOS
#endif