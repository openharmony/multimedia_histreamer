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

#define HST_LOG_TAG "MuxerFilter"

#include "muxer_filter.h"

#include "data_collector.h"
#include "data_spliter.h"
#include "factory/filter_factory.h"
#include "foundation/log.h"
#include "common/plugin_settings.h"
#include "common/plugin_utils.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
namespace {
std::vector<std::shared_ptr<Plugin::PluginInfo>> Intersections(
    const std::vector<std::shared_ptr<Plugin::PluginInfo>>& caps1,
    const std::vector<std::pair<std::shared_ptr<Plugin::PluginInfo>, Plugin::Capability>>& caps2)
{
    std::vector<std::shared_ptr<Plugin::PluginInfo>> intersections;
    for (const auto& cap1 : caps1) {
        for (const auto& cap2 : caps2) {
            if (cap1->name == cap2.first->name) {
                intersections.emplace_back(cap1);
            }
        }
    }
    return intersections;
}
}
static AutoRegisterFilter<MuxerFilter> g_registerFilterHelper("builtin.recorder.muxer");

MuxerFilter::MuxerFilter(std::string name) : FilterBase(std::move(name)),
    muxerDataSink_(std::make_shared<MuxerDataSink>())
{
    filterType_ = FilterType::MUXER;
}

MuxerFilter::~MuxerFilter() {}
void MuxerFilter::Init(EventReceiver* receiver, FilterCallback* callback)
{
    this->eventReceiver_ = receiver;
    this->callback_ = callback;
    inPorts_.clear();
    outPorts_.clear();
    outPorts_.emplace_back(std::make_shared<Pipeline::OutPort>(this, PORT_NAME_DEFAULT));
    muxerDataSink_->muxerFilter_ = this;
    state_ = FilterState::INITIALIZED;
}
bool MuxerFilter::UpdateAndInitPluginByInfo(const std::shared_ptr<Plugin::PluginInfo>& selectedPluginInfo)
{
    if (selectedPluginInfo == nullptr) {
        MEDIA_LOG_W("no available info to update plugin");
        return false;
    }
    if (plugin_ != nullptr) {
        if (targetPluginInfo_ != nullptr && targetPluginInfo_->name == selectedPluginInfo->name) {
            if (plugin_->Reset() == Plugin::Status::OK) {
                return true;
            }
            MEDIA_LOG_W("reuse previous plugin %s failed, will create new plugin", targetPluginInfo_->name.c_str());
        }
        plugin_->Deinit();
    }

    plugin_ = Plugin::PluginManager::Instance().CreateMuxerPlugin(selectedPluginInfo->name);
    if (plugin_ == nullptr) {
        MEDIA_LOG_E("cannot create plugin %s", selectedPluginInfo->name.c_str());
        return false;
    }
    auto err = TranslatePluginStatus(plugin_->Init());
    if (err != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("muxer plugin init error");
        return false;
    }
    targetPluginInfo_ = selectedPluginInfo;
    return true;
}

bool MuxerFilter::Negotiate(const std::string& inPort, const std::shared_ptr<const Capability>& upstreamCap,
                            Capability& upstreamNegotiatedCap)
{
    if (state_ != FilterState::PREPARING) {
        MEDIA_LOG_W("decoder filter is not in preparing when negotiate");
        return false;
    }
    capabilityCache_.emplace_back(std::make_pair(inPort, *upstreamCap));
    if (capabilityCache_.size() < inPorts_.size()) {
        return true;
    }
    MEDIA_LOG_I("all track caps has been received, start negotiating downstream");
    auto candidate = FindAvailablePluginsByOutputMime(containerMime_, Plugin::PluginType::MUXER);
    for (const auto& cache: capabilityCache_) {
        auto tmp = FindAvailablePlugins(cache.second, Plugin::PluginType::MUXER);
        candidate = Intersections(candidate, tmp);
        if (candidate.empty()) {
            break;
        }
    }
    if (candidate.empty()) {
        MEDIA_LOG_E("cannot find any available plugins");
        return false;
    }
    auto muxerCap = std::make_shared<Capability>(containerMime_);
    Capability downCap;
    if (!outPorts_[0]->Negotiate(muxerCap, downCap)) {
        MEDIA_LOG_E("downstream of muxer filter negotiate failed");
        return false;
    }
    // always use the first candidate plugin info
    return UpdateAndInitPluginByInfo(candidate[0]);
}
ErrorCode MuxerFilter::AddTrackThenConfigure(const std::pair<std::string, Plugin::Meta>& metaPair)
{
    uint32_t trackId = 0;
    ErrorCode ret = TranslatePluginStatus(plugin_->AddTrack(trackId));
    if (ret != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("muxer plugin add track failed");
        return ret;
    }
    portTrackIdMap_.emplace_back(std::make_pair(metaPair.first, trackId));

    auto parameterMap = PluginParameterTable::FindAllowedParameterMap(filterType_);
    for (const auto& keyPair : parameterMap) {
        Plugin::ValueType outValue;
        if (metaPair.second.GetData(static_cast<Plugin::MetaID>(keyPair.first), outValue) &&
            keyPair.second.second(outValue)) {
            plugin_->SetTrackParameter(trackId, keyPair.first, outValue);
        } else {
            MEDIA_LOG_W("parameter %s in meta is not found or type mismatch", keyPair.second.first.c_str());
        }
    }
    return ErrorCode::SUCCESS;
}

ErrorCode MuxerFilter::ConfigureToStart()
{
    ErrorCode ret;
    for (const auto& cache: metaCache_) {
        ret = AddTrackThenConfigure(cache);
        if (ret != ErrorCode::SUCCESS) {
            MEDIA_LOG_E("add and configure for track from inPort %s failed", cache.first.c_str());
            return ret;
        }
    }
    // todo add other global meta

    ret = TranslatePluginStatus(plugin_->Prepare());
    if (ret != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("muxer plugin prepare failed");
        return ret;
    }
    ret = TranslatePluginStatus(plugin_->Start());
    if (ret != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("muxer plugin start failed");
    }
    return ret;
}
bool MuxerFilter::Configure(const std::string& inPort, const std::shared_ptr<const Plugin::Meta>& upstreamMeta)
{
    std::string tmp;
    if (!upstreamMeta->GetString(Plugin::MetaID::MIME, tmp)) {
        MEDIA_LOG_E("stream meta must contain mime, which is not found in current stream from port %s", inPort.c_str());
        return false;
    }
    metaCache_.emplace_back(std::make_pair(inPort, *upstreamMeta));
    if (metaCache_.size() < inPorts_.size()) {
        return true;
    }
    if (plugin_ == nullptr) {
        MEDIA_LOG_E("cannot configure when no plugin available");
        return false;
    }

    auto meta = std::make_shared<Plugin::Meta>();
    meta->SetString(Plugin::MetaID::MIME, containerMime_);
    if (!outPorts_[0]->Configure(meta)) {
        MEDIA_LOG_E("downstream of muxer filter configure failed");
        return false;
    }
    plugin_->SetDataSink(muxerDataSink_);
    auto ret = ConfigureToStart();
    if (ret != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("muxer filter configure and start error");
        OnEvent({EVENT_ERROR, ret});
        return false;
    }
    state_ = FilterState::READY;
    OnEvent({EVENT_READY});
    MEDIA_LOG_I("audio decoder send EVENT_READY");
    return true;
}

ErrorCode MuxerFilter::SetOutputFormat(std::string containerMime)
{
    containerMime_ = std::move(containerMime);
    return ErrorCode::SUCCESS;
}

ErrorCode MuxerFilter::AddTrack(std::shared_ptr<InPort> &trackPort)
{
    if (state_ != FilterState::INITIALIZED) {
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    trackPort = std::make_shared<InPort>(this, std::string(PORT_NAME_DEFAULT) + std::to_string(inPorts_.size()));
    inPorts_.emplace_back(trackPort);
    return ErrorCode::SUCCESS;
}

ErrorCode MuxerFilter::SetMaxDurationUs(uint64_t maxDurationUs)
{
    return ErrorCode::SUCCESS;
}

ErrorCode MuxerFilter::SplitMuxBegin()
{
    return ErrorCode::SUCCESS;
}

ErrorCode MuxerFilter::SplitMuxEnd()
{
    return ErrorCode::SUCCESS;
}

ErrorCode MuxerFilter::PushData(const std::string& inPort, AVBufferPtr buffer, int64_t offset)
{
    if (state_ != FilterState::READY && state_ != FilterState::PAUSED && state_ != FilterState::RUNNING) {
        MEDIA_LOG_W("pushing data to decoder when state is %d", static_cast<int>(state_.load()));
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    // todo we should consider more tracks
    if (!hasWriteHeader_) {
        plugin_->WriteHeader();
        hasWriteHeader_ = true;
    }
    if (buffer->GetMemory()->GetSize() != 0) {
        plugin_->WriteFrame(buffer);
    }

    if (buffer->flag & BUFFER_FLAG_EOS) {
        plugin_->WriteTrailer();
        hasWriteHeader_ = false;
    }
    return ErrorCode::SUCCESS;
}

Plugin::Status MuxerFilter::MuxerDataSink::WriteAt(int64_t offset, const std::shared_ptr<Plugin::Buffer> &buffer)
{
    if (muxerFilter_ != nullptr) {
        muxerFilter_->outPorts_[0]->PushData(buffer, offset);
    }
    return Plugin::Status::OK;
}
} // Pipeline
} // Media
} // OHOS
#endif