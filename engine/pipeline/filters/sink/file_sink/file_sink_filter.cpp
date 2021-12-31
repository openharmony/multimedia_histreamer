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

#define HST_LOG_TAG "FileSinkFilter"

#include "file_sink_filter.h"

#include "common/plugin_utils.h"
#include "factory/filter_factory.h"
#include "foundation/log.h"
#include "utils/steady_clock.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
static AutoRegisterFilter<FileSinkFilter> g_registerFilterHelper("builtin.recorder.file_sink");

FileSinkFilter::FileSinkFilter(std::string name) : FilterBase(std::move(name)) {}

FileSinkFilter::~FileSinkFilter() {}

void FileSinkFilter::Init(EventReceiver *receiver, FilterCallback *callback)
{
    FilterBase::Init(receiver, callback);
    outPorts_.clear();
}
bool FileSinkFilter::Negotiate(const std::string &inPort, const std::shared_ptr<const Capability> &upstreamCap,
                               Capability &upstreamNegotiatedCap)
{
    auto candidatePlugins = FindAvailablePlugins(*upstreamCap, Plugin::PluginType::FILE_SINK);
    if (candidatePlugins.empty()) {
        MEDIA_LOG_E("no available file sink plugin");
        return false;
    }
    std::shared_ptr<Plugin::PluginInfo> selectedPluginInfo = candidatePlugins[0].first;
    upstreamNegotiatedCap = candidatePlugins[0].second;
    // find the highest rank plugin
    for (const auto& pair : candidatePlugins) {
        if (pair.first->rank > selectedPluginInfo->rank) {
            selectedPluginInfo = pair.first;
            upstreamNegotiatedCap = pair.second;
        }
    }
    auto res = UpdateAndInitPluginByInfo<Plugin::FileSink>(plugin_, pluginInfo_, selectedPluginInfo,
    [](const std::string& name) -> std::shared_ptr<Plugin::FileSink> {
        return Plugin::PluginManager::Instance().CreateFileSinkPlugin(name);
    });
    return res;
}

bool FileSinkFilter::Configure(const std::string &inPort, const std::shared_ptr<const Plugin::Meta> &upstreamMeta)
{
    PROFILE_BEGIN("Audio sink configure begin");
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
        MEDIA_LOG_E("sink configure error");
        OnEvent({EVENT_ERROR, err});
        return false;
    }
    state_ = FilterState::READY;
    OnEvent({EVENT_READY});
    MEDIA_LOG_I("audio sink send EVENT_READY");
    PROFILE_END("Audio sink configure end");
    return true;
}

ErrorCode FileSinkFilter::SetOutputPath(const std::string &path)
{
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
    return ErrorCode::SUCCESS;
}

ErrorCode FileSinkFilter::SetFd(int32_t fd)
{
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
    return ErrorCode::SUCCESS;
}

ErrorCode FileSinkFilter::PushData(const std::string &inPort, AVBufferPtr buffer, int64_t offset)
{
    auto ret = ErrorCode::SUCCESS;
    if (offset >= 0 && offset != currentPos_) {
        if (!plugin_->IsSeekable()) {
            MEDIA_LOG_E("plugin %s does not support seekable", pluginInfo_->name.c_str());
            return ErrorCode::ERROR_INVALID_OPERATION;
        } else {
            ret = TranslatePluginStatus(plugin_->SeekTo(offset));
            if (ret != ErrorCode::SUCCESS) {
                MEDIA_LOG_E("plugin %s seek to %" PRId64 "failed", pluginInfo_->name.c_str(), offset);
                return ErrorCode::ERROR_INVALID_OPERATION;
            }
            currentPos_ = offset;
        }
    }
    if (!buffer->IsEmpty()) {
        ret = TranslatePluginStatus(plugin_->Write(buffer));
        if (ret != ErrorCode::SUCCESS) {
            MEDIA_LOG_E("write to plugin failed with error code %d", ret);
            return ret;
        }
        currentPos_ += buffer->GetMemory()->GetSize();
    }
    if (buffer->flag & BUFFER_FLAG_EOS) {
        plugin_->Flush();
        Event event {
            .type = EVENT_AUDIO_COMPLETE,
        };
        MEDIA_LOG_D("file sink push data send event_complete");
        OnEvent(event);
    }
    return ErrorCode::SUCCESS;
}

ErrorCode FileSinkFilter::Stop()
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