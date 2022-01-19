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

#define HST_LOG_TAG "AudioSinkFilter"
#include "audio_sink_filter.h"
#include "common/plugin_utils.h"
#include "factory/filter_factory.h"
#include "foundation/log.h"
#include "foundation/osal/utils/util.h"
#include "pipeline/filters/common/plugin_settings.h"
#include "pipeline/core/clock_manager.h"
#include "plugin/common/plugin_time.h"
#include "plugin/common/plugin_types.h"
#include "plugin/core/plugin_meta.h"
#include "utils/steady_clock.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
static AutoRegisterFilter<AudioSinkFilter> g_registerFilterHelper("builtin.player.audiosink");

AudioSinkFilter::AudioSinkFilter(const std::string& name) : FilterBase(name)
{
    filterType_ = FilterType::AUDIO_SINK;
    MEDIA_LOG_I("audio sink ctor called");
}
AudioSinkFilter::~AudioSinkFilter()
{
    MEDIA_LOG_D("audio sink dtor called");
    if (plugin_) {
        plugin_->Stop();
        plugin_->Deinit();
    }
}

void AudioSinkFilter::Init(EventReceiver* receiver, FilterCallback* callback)
{
    FilterBase::Init(receiver, callback);
    outPorts_.clear();
    ClockManager::Instance().RegisterProvider(shared_from_this());
}

ErrorCode AudioSinkFilter::SetPluginParameter(Tag tag, const Plugin::ValueType& value)
{
    return TranslatePluginStatus(plugin_->SetParameter(tag, value));
}

ErrorCode AudioSinkFilter::SetParameter(int32_t key, const Plugin::Any& value)
{
    if (state_.load() == FilterState::CREATED) {
        return ErrorCode::ERROR_AGAIN;
    }
    Tag tag = Tag::INVALID;
    if (!TranslateIntoParameter(key, tag)) {
        MEDIA_LOG_I("SetParameter key %d is out of boundary", key);
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    RETURN_AGAIN_IF_NULL(plugin_);
    return SetPluginParameter(tag, value);
}

ErrorCode AudioSinkFilter::GetParameter(int32_t key, Plugin::Any& value)
{
    if (state_.load() == FilterState::CREATED) {
        return ErrorCode::ERROR_AGAIN;
    }
    Tag tag = Tag::INVALID;
    if (!TranslateIntoParameter(key, tag)) {
        MEDIA_LOG_I("GetParameter key %d is out of boundary", key);
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    RETURN_AGAIN_IF_NULL(plugin_);
    return TranslatePluginStatus(plugin_->GetParameter(tag, value));
}

bool AudioSinkFilter::Negotiate(const std::string& inPort, const std::shared_ptr<const Plugin::Capability>& upstreamCap,
                                Capability& upstreamNegotiatedCap)
{
    MEDIA_LOG_I("audio sink negotiate started");
    PROFILE_BEGIN("Audio Sink Negotiate begin");
    auto candidatePlugins = FindAvailablePlugins(*upstreamCap, Plugin::PluginType::AUDIO_SINK);
    if (candidatePlugins.empty()) {
        MEDIA_LOG_E("no available audio sink plugin");
        return false;
    }
    // always use first one
    std::shared_ptr<Plugin::PluginInfo> selectedPluginInfo = candidatePlugins[0].first;
    for (const auto& onCap : selectedPluginInfo->inCaps) {
        if (onCap.keys.count(CapabilityID::AUDIO_SAMPLE_FORMAT) == 0) {
            MEDIA_LOG_E("each in caps of sink must contains valid audio sample format");
            return false;
        }
    }

    upstreamNegotiatedCap = candidatePlugins[0].second;

    auto res = UpdateAndInitPluginByInfo<Plugin::AudioSink>(plugin_, pluginInfo_, selectedPluginInfo,
        [](const std::string& name) -> std::shared_ptr<Plugin::AudioSink> {
        return Plugin::PluginManager::Instance().CreateAudioSinkPlugin(name);
    });
    PROFILE_END("Audio Sink Negotiate end");
    return res;
}

bool AudioSinkFilter::Configure(const std::string& inPort, const std::shared_ptr<const Plugin::Meta>& upstreamMeta)
{
    PROFILE_BEGIN("Audio sink configure begin");
    if (plugin_ == nullptr || pluginInfo_ == nullptr) {
        MEDIA_LOG_E("cannot configure decoder when no plugin available");
        return false;
    }

    auto err = ConfigureToPreparePlugin(upstreamMeta);
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

ErrorCode AudioSinkFilter::ConfigureWithMeta(const std::shared_ptr<const Plugin::Meta>& meta)
{
    auto parameterMap = PluginParameterTable::FindAllowedParameterMap(filterType_);
    for (const auto& keyPair : parameterMap) {
        Plugin::ValueType outValue;
        if (meta->GetData(static_cast<Plugin::MetaID>(keyPair.first), outValue) && keyPair.second.second(outValue)) {
            SetPluginParameter(keyPair.first, outValue);
        } else {
            MEDIA_LOG_W("parameter %s in meta is not found or type mismatch", keyPair.second.first.c_str());
        }
    }
    return ErrorCode::SUCCESS;
}
ErrorCode AudioSinkFilter::ConfigureToPreparePlugin(const std::shared_ptr<const Plugin::Meta>& meta)
{
    auto err = ConfigureWithMeta(meta);
    if (err != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("sink configuration failed ");
        return err;
    }
    err = TranslatePluginStatus(plugin_->Prepare());
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, "sink prepare failed");

    return ErrorCode::SUCCESS;
}

void AudioSinkFilter::ReportCurrentPosition(int64_t pts)
{
    if (plugin_) {
        OnEvent({EVENT_AUDIO_PROGRESS, static_cast<int64_t>(pts)});
    }
}

ErrorCode AudioSinkFilter::PushData(const std::string& inPort, AVBufferPtr buffer, int64_t offset)
{
    MEDIA_LOG_D("audio sink push data started, state: %d", state_.load());
    if (isFlushing || state_.load() == FilterState::INITIALIZED) {
        MEDIA_LOG_I("audio sink is flushing ignore this buffer");
        return ErrorCode::SUCCESS;
    }
    if (state_.load() != FilterState::RUNNING) {
        pushThreadIsBlocking = true;
        OSAL::ScopedLock lock(mutex_);
        startWorkingCondition_.Wait(lock, [this] {
            return state_ == FilterState::RUNNING || state_ == FilterState::INITIALIZED || isFlushing;
        });
        pushThreadIsBlocking = false;
    }
    if (isFlushing || state_.load() == FilterState::INITIALIZED) {
        MEDIA_LOG_I("PushData return due to: isFlushing = %d, state = %d", isFlushing, static_cast<int>(state_.load()));
        return ErrorCode::SUCCESS;
    }
    auto err = TranslatePluginStatus(plugin_->Write(buffer));
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, "audio sink write failed");
    ReportCurrentPosition(static_cast<int64_t>(buffer->pts));
    if ((buffer->flag & BUFFER_FLAG_EOS) != 0) {
        constexpr int waitTimeForPlaybackCompleteMs = 60;
        OHOS::Media::OSAL::SleepFor(waitTimeForPlaybackCompleteMs);
        Event event{
            .type = EVENT_AUDIO_COMPLETE,
        };
        MEDIA_LOG_D("audio sink push data send event_complete");
        OnEvent(event);
    }
    if (buffer->pts != -1) {
        UpdateLatestPts(buffer->pts);
    } else {
    }

    MEDIA_LOG_D("audio sink push data end");
    return ErrorCode::SUCCESS;
}

ErrorCode AudioSinkFilter::Start()
{
    MEDIA_LOG_I("start called");
    if (state_ != FilterState::READY && state_ != FilterState::PAUSED) {
        MEDIA_LOG_W("sink is not ready when start, state: %d", static_cast<int32_t>(state_.load()));
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    auto err = FilterBase::Start();
    if (err != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("audio sink filter start error");
        return err;
    }
    err = TranslatePluginStatus(plugin_->Start());
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, "audio sink plugin start failed");
    if (pushThreadIsBlocking.load()) {
        startWorkingCondition_.NotifyOne();
    }
    frameCnt_ = 0;
    return ErrorCode::SUCCESS;
}

ErrorCode AudioSinkFilter::Stop()
{
    MEDIA_LOG_I("audio sink stop start");
    FilterBase::Stop();
    if (plugin_ != nullptr) {
        plugin_->Stop();
    }
    if (pushThreadIsBlocking.load()) {
        startWorkingCondition_.NotifyOne();
    }
    MEDIA_LOG_I("audio sink stop finish");
    return ErrorCode::SUCCESS;
}

ErrorCode AudioSinkFilter::Pause()
{
    MEDIA_LOG_I("audio sink filter pause start");
    // only worked when state is working
    if (state_ != FilterState::READY && state_ != FilterState::RUNNING) {
        MEDIA_LOG_W("audio sink cannot pause when not working");
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    auto err = FilterBase::Pause();
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, "audio sink pause failed");
    err = TranslatePluginStatus(plugin_->Pause());
    MEDIA_LOG_D("audio sink filter pause end");
    return err;
}
ErrorCode AudioSinkFilter::Resume()
{
    MEDIA_LOG_I("audio sink filter resume");
    // only worked when state is paused
    if (state_ == FilterState::PAUSED) {
        state_ = FilterState::RUNNING;
        if (pushThreadIsBlocking) {
            startWorkingCondition_.NotifyOne();
        }
        if (frameCnt_ > 0) {
            frameCnt_ = 0;
        }
        return TranslatePluginStatus(plugin_->Resume());
    }
    return ErrorCode::SUCCESS;
}

void AudioSinkFilter::FlushStart()
{
    MEDIA_LOG_I("audio sink flush start entered");
    isFlushing = true;
    if (pushThreadIsBlocking) {
        startWorkingCondition_.NotifyOne();
    }
    plugin_->Pause();
    plugin_->Flush();
}

void AudioSinkFilter::FlushEnd()
{
    MEDIA_LOG_I("audio sink flush end entered");
    plugin_->Resume();
    isFlushing = false;
}

ErrorCode AudioSinkFilter::SetVolume(float volume)
{
    if (state_ != FilterState::READY && state_ != FilterState::RUNNING && state_ != FilterState::PAUSED) {
        MEDIA_LOG_E("audio sink filter cannot set volume in state %d", static_cast<int32_t>(state_.load()));
        return ErrorCode::ERROR_AGAIN;
    }
    MEDIA_LOG_I("set volume %.3f", volume);
    return TranslatePluginStatus(plugin_->SetVolume(volume));
}

ErrorCode AudioSinkFilter::UpdateLatestPts(int64_t pts)
{
    uint64_t latencyNano {10};
    int64_t nowNs {0};
    Plugin::Status status = plugin_->GetLatency(latencyNano);
    if (status != Plugin::Status::OK) {
        MEDIA_LOG_E("audio sink GetLatency fail errorcode = %d", to_underlying(TranslatePluginStatus(status)));
        return TranslatePluginStatus(status);
    }
    nowNs = SteadyClock::GetCurrentTimeNanoSec();
    if (INT64_MAX - nowNs < latencyNano) { // overflow
        return ErrorCode::ERROR_UNKNOWN;
    }
    latestSysClock_ = nowNs + latencyNano;
    latestPts_ = pts;
    frameCnt_++;
    return ErrorCode::SUCCESS;
}

ErrorCode AudioSinkFilter::CheckPts(int64_t pts, int64_t& delta)
{
    int64_t ptsOut {0};
    if (latestSysClock_ == 0 && latestPts_ == 0) {
        delta = 0;
        return ErrorCode::SUCCESS;
    }
    if (GetCurrentPosition(ptsOut) != ErrorCode::SUCCESS) {
        return ErrorCode::ERROR_UNKNOWN;
    }
    delta = pts - ptsOut;
    return ErrorCode::SUCCESS;
}

ErrorCode AudioSinkFilter::GetCurrentPosition(int64_t& position)
{
    int64_t nowNs {0};
    if (latestPts_ == 0 && frameCnt_ == 0) {
        position = 0;
        return ErrorCode::SUCCESS;
    }
    nowNs = SteadyClock::GetCurrentTimeNanoSec();
    if (INT64_MAX - (nowNs - latestSysClock_) < latestPts_) { // overflow
        return ErrorCode::ERROR_UNKNOWN;
    }
    position = nowNs - latestSysClock_ + latestPts_ ;
    return ErrorCode::SUCCESS;
}

ErrorCode AudioSinkFilter::GetCurrentTimeNano(int64_t& nowNano)
{
    nowNano = SteadyClock::GetCurrentTimeNanoSec();
    return ErrorCode::SUCCESS;
}
} // namespace Pipeline
} // namespace Media
} // namespace OHOS
