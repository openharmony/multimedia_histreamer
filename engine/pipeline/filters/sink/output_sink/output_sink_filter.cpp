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

#define HST_LOG_TAG "OutputSinkFilter"

#include "output_sink_filter.h"

#include "common/plugin_utils.h"
#include "factory/filter_factory.h"
#include "foundation/log.h"
#include "utils/steady_clock.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
static AutoRegisterFilter<OutputSinkFilter> g_registerFilterHelper("builtin.recorder.output_sink");

OutputSinkFilter::OutputSinkFilter(std::string name) : FilterBase(std::move(name)) {}

OutputSinkFilter::~OutputSinkFilter() {}

void OutputSinkFilter::Init(EventReceiver *receiver, FilterCallback *callback)
{
    FilterBase::Init(receiver, callback);
    outPorts_.clear();
}
bool OutputSinkFilter::Negotiate(const std::string &inPort,
                                 const std::shared_ptr<const Plugin::Capability>& upstreamCap,
                                 Plugin::Capability& negotiatedCap,
                                 const Plugin::TagMap& upstreamParams,
                                 Plugin::TagMap& downstreamParams)
{
    auto candidatePlugins = FindAvailablePlugins(*upstreamCap, Plugin::PluginType::OUTPUT_SINK);
    if (candidatePlugins.empty()) {
        MEDIA_LOG_E("no available output sink plugin");
        return false;
    }
    std::shared_ptr<Plugin::PluginInfo> selectedPluginInfo = nullptr;
    for (const auto& candidate : candidatePlugins) {
        const auto& tmp = candidate.first->extra[PLUGIN_INFO_EXTRA_OUTPUT_TYPE];
        if (!tmp.SameTypeWith(typeid(Plugin::OutputType))) {
            continue;
        }
        if (Plugin::AnyCast<Plugin::OutputType>(tmp) == outputType_) {
            if (selectedPluginInfo == nullptr) {
                selectedPluginInfo = candidate.first;
                negotiatedCap = candidate.second;
            } else if (candidate.first->rank > selectedPluginInfo->rank) {
                selectedPluginInfo = candidate.first;
                negotiatedCap = candidate.second;
            }
        }
    }
    if (selectedPluginInfo == nullptr) {
        MEDIA_LOG_W("no available output sink plugin with output type of %" PUBLIC_OUTPUT "d",
                    static_cast<int32_t>(outputType_));
        return false;
    }
    auto res = UpdateAndInitPluginByInfo<Plugin::OutputSink>(plugin_, pluginInfo_, selectedPluginInfo,
    [](const std::string& name) -> std::shared_ptr<Plugin::OutputSink> {
        return Plugin::PluginManager::Instance().CreateOutputSinkPlugin(name);
    });
    return res;
}

bool OutputSinkFilter::Configure(const std::string& inPort, const std::shared_ptr<const Plugin::Meta>& upstreamMeta)
{
    PROFILE_BEGIN("Output sink configure begin");
    if (plugin_ == nullptr || pluginInfo_ == nullptr) {
        MEDIA_LOG_E("cannot configure decoder when no plugin available");
        return false;
    }

    auto err = ErrorCode::SUCCESS;
    if (fd_ == -1) { // always use fd firstly
        if (!outputPath_.empty()) {
            err = TranslatePluginStatus(plugin_->SetSink(outputPath_));
        }
    } else {
        err = TranslatePluginStatus(plugin_->SetSink(fd_));
    }

    if (err != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("Output sink configure error");
        OnEvent({name_, EventType::EVENT_ERROR, err});
        return false;
    }
    state_ = FilterState::READY;
    OnEvent({name_, EventType::EVENT_READY});
    MEDIA_LOG_I("Output sink send EVENT_READY");
    PROFILE_END("Output sink configure end");
    return true;
}

ErrorCode OutputSinkFilter::SetOutputPath(const std::string &path)
{
    if (outputType_ != Plugin::OutputType::UNKNOWN && outputType_ != Plugin::OutputType::URI) {
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    if (path.empty()) {
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    auto ret = ErrorCode::SUCCESS;
    if (plugin_ != nullptr) {
        ret = TranslatePluginStatus(plugin_->SetSink(path));
    }
    if (ret != ErrorCode::SUCCESS) {
        return ret;
    }
    outputPath_ = path;
    outputType_ = Plugin::OutputType::URI;
    return ErrorCode::SUCCESS;
}

ErrorCode OutputSinkFilter::SetFd(int32_t fd)
{
    if (outputType_ != Plugin::OutputType::UNKNOWN && outputType_ != Plugin::OutputType::FD) {
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    if (fd < 0) {
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    auto ret = ErrorCode::SUCCESS;
    if (plugin_ != nullptr) {
        ret = TranslatePluginStatus(plugin_->SetSink(fd));
    }
    if (ret != ErrorCode::SUCCESS) {
        return ret;
    }
    fd_ = fd;
    outputType_ = Plugin::OutputType::FD;
    return ErrorCode::SUCCESS;
}

ErrorCode OutputSinkFilter::PushData(const std::string &inPort, AVBufferPtr buffer, int64_t offset)
{
    auto ret = ErrorCode::SUCCESS;
    if (offset >= 0 && offset != currentPos_) {
        if (!plugin_->IsSeekable()) {
            MEDIA_LOG_E("plugin %" PUBLIC_OUTPUT "s does not support seekable", pluginInfo_->name.c_str());
            return ErrorCode::ERROR_INVALID_OPERATION;
        } else {
            ret = TranslatePluginStatus(plugin_->SeekTo(offset));
            if (ret != ErrorCode::SUCCESS) {
                MEDIA_LOG_E("plugin %" PUBLIC_OUTPUT "s seek to %" PUBLIC_OUTPUT PRId64 " failed",
                            pluginInfo_->name.c_str(), offset);
                return ErrorCode::ERROR_INVALID_OPERATION;
            }
            currentPos_ = offset;
        }
    }
    if (!buffer->IsEmpty()) {
        ret = TranslatePluginStatus(plugin_->Write(buffer));
        if (ret != ErrorCode::SUCCESS) {
            MEDIA_LOG_E("write to plugin failed with error code %" PUBLIC_OUTPUT "d", to_underlying(ret));
            return ret;
        }
        currentPos_ += buffer->GetMemory()->GetSize();
    }
    if (buffer->flag & BUFFER_FLAG_EOS) {
        plugin_->Flush();
        Event event {
            .srcFilter = name_,
            .type = EventType::EVENT_COMPLETE,
        };
        MEDIA_LOG_D("file sink push data send event_complete");
        OnEvent(event);
    }
    return ErrorCode::SUCCESS;
}

ErrorCode OutputSinkFilter::Stop()
{
    currentPos_ = 0;
    fd_ = -1;
    outputPath_.clear();
    return ErrorCode::SUCCESS;
}
} // Pipeline
} // Media
} // OHOS
#endif