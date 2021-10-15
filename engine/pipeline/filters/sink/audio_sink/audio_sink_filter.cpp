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

#define LOG_TAG "AudioSinkFilter"

#include "audio_sink_filter.h"
#include "common/plugin_utils.h"
#include "factory/filter_factory.h"
#include "foundation/log.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
static AutoRegisterFilter<AudioSinkFilter> g_registerFilterHelper("builtin.player.audiosink");

AudioSinkFilter::AudioSinkFilter(const std::string& name) : FilterBase(name)
{
    MEDIA_LOG_I("audio sink ctor called");
}
AudioSinkFilter::~AudioSinkFilter()
{
    MEDIA_LOG_I("audio sink dtor called");
    if (plugin_) {
        plugin_->Stop();
        plugin_->Deinit();
    }
}

void AudioSinkFilter::Init(EventReceiver* receiver, FilterCallback* callback)
{
    FilterBase::Init(receiver, callback);
    outPorts_.clear();
}

ErrorCode AudioSinkFilter::SetPluginParameter(Tag tag, const Plugin::ValueType& value)
{
    return TranslatePluginStatus(plugin_->SetParameter(tag, value));
}

ErrorCode AudioSinkFilter::SetParameter(int32_t key, const Plugin::Any& value)
{
    if (state_.load() == FilterState::CREATED) {
        return ERROR_STATE;
    }
    Tag tag = Tag::INVALID;
    if (!TranslateIntoParameter(key, tag)) {
        MEDIA_LOG_I("SetParameter key %d is out of boundary", key);
        return INVALID_PARAM_VALUE;
    }
    RETURN_PLUGIN_NOT_FOUND_IF_NULL(plugin_);
    return SetPluginParameter(tag, value);
}

template <typename T>
ErrorCode AudioSinkFilter::GetPluginParameter(Tag tag, T& value)
{
    Plugin::Any tmp;
    auto err = TranslatePluginStatus(plugin_->GetParameter(tag, tmp));
    if (err == SUCCESS && tmp.Type() == typeid(T)) {
        value = Plugin::AnyCast<T>(tmp);
    }
    return err;
}

ErrorCode AudioSinkFilter::GetParameter(int32_t key, Plugin::Any& value)
{
    if (state_.load() == FilterState::CREATED) {
        return ERROR_STATE;
    }
    Tag tag = Tag::INVALID;
    if (!TranslateIntoParameter(key, tag)) {
        MEDIA_LOG_I("GetParameter key %d is out of boundary", key);
        return INVALID_PARAM_VALUE;
    }
    RETURN_PLUGIN_NOT_FOUND_IF_NULL(plugin_);
    return TranslatePluginStatus(plugin_->GetParameter(tag, value));
}

bool AudioSinkFilter::Negotiate(const std::string& inPort, const std::shared_ptr<const Plugin::Meta>& inMeta,
                                CapabilitySet& outCaps)
{
    MEDIA_LOG_D("audio sink negotiate started");
    auto creator = [](const std::string& pluginName) {
        return Plugin::PluginManager::Instance().CreateAudioSinkPlugin(pluginName);
    };
    auto err = FindPluginAndUpdate<Plugin::AudioSink>(inMeta, Plugin::PluginType::AUDIO_SINK, plugin_,
                                                      targetPluginInfo_, creator);
    RETURN_TARGET_ERR_MESSAGE_LOG_IF_FAIL(err, false, "cannot find matched plugin");

    outCaps = targetPluginInfo_->inCaps;

    err = ConfigureToPreparePlugin(inMeta);
    if (err != SUCCESS) {
        MEDIA_LOG_E("sink configure error");
        OnEvent({EVENT_ERROR, err});
        return false;
    }
    state_ = FilterState::READY;
    OnEvent({EVENT_READY});
    MEDIA_LOG_I("audio sink send EVENT_READY");
    return true;
}

ErrorCode AudioSinkFilter::ConfigureWithMeta(const std::shared_ptr<const Plugin::Meta>& meta)
{
    uint32_t channels;
    if (meta->GetUint32(Plugin::MetaID::AUDIO_CHANNELS, channels)) {
        MEDIA_LOG_D("found audio channel meta");
        SetPluginParameter(Tag::AUDIO_CHANNELS, channels);
    }
    uint32_t sampleRate;
    if (meta->GetUint32(Plugin::MetaID::AUDIO_SAMPLE_RATE, sampleRate)) {
        MEDIA_LOG_D("found audio sample rate meta");
        SetPluginParameter(Tag::AUDIO_SAMPLE_RATE, sampleRate);
    }
    int64_t bitRate;
    if (meta->GetInt64(Plugin::MetaID::MEDIA_BITRATE, bitRate)) {
        MEDIA_LOG_D("found audio bit rate meta");
        SetPluginParameter(Tag::MEDIA_BITRATE, bitRate);
    }

    auto audioFormat = Plugin::AudioSampleFormat::U8;
    if (meta->GetData<Plugin::AudioSampleFormat>(Plugin::MetaID::AUDIO_SAMPLE_FORMAT, audioFormat)) {
        SetPluginParameter(Tag::AUDIO_SAMPLE_FORMAT, audioFormat);
    }

    auto audioChannelLayout = Plugin::AudioChannelLayout::STEREO;
    if (meta->GetData<Plugin::AudioChannelLayout>(Plugin::MetaID::AUDIO_CHANNEL_LAYOUT, audioChannelLayout)) {
        SetPluginParameter(Tag::AUDIO_CHANNEL_LAYOUT, audioChannelLayout);
    }

    uint32_t samplePerFrame = 0;
    if (meta->GetUint32(Plugin::MetaID::AUDIO_SAMPLE_PRE_FRAME, samplePerFrame)) {
        SetPluginParameter(Tag::AUDIO_SAMPLE_PRE_FRAME, samplePerFrame);
    }
    return SUCCESS;
}
ErrorCode AudioSinkFilter::ConfigureToPreparePlugin(const std::shared_ptr<const Plugin::Meta>& meta)
{
    auto err = TranslatePluginStatus(plugin_->Init());
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, "sink plugin init error.");
    err = ConfigureWithMeta(meta);
    if (err != SUCCESS) {
        MEDIA_LOG_E("sink configuration failed ");
        return err;
    }
    err = TranslatePluginStatus(plugin_->Prepare());
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, "sink prepare failed");

    return SUCCESS;
}

ErrorCode AudioSinkFilter::PushData(const std::string& inPort, AVBufferPtr buffer)
{
    MEDIA_LOG_D("audio sink push data started, state: %d", state_.load());
    if (isFlushing || state_.load() == FilterState::INITIALIZED) {
        MEDIA_LOG_I("audio sink is flushing ignore this buffer");
        return SUCCESS;
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
        return SUCCESS;
    }

    if ((buffer->flag & BUFFER_FLAG_EOS) != 0) {
        Event event{
            .type = EVENT_COMPLETE,
        };
        MEDIA_LOG_D("audio sink push data send event_complete");
        OnEvent(event);
        MEDIA_LOG_D("audio sink push data end");
        return SUCCESS;
    }
    auto err = TranslatePluginStatus(plugin_->Write(buffer));
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, "audio sink write failed");
    MEDIA_LOG_D("audio sink push data end");
    return err;
}

ErrorCode AudioSinkFilter::Start()
{
    MEDIA_LOG_D("start called");
    if (state_ != FilterState::READY && state_ != FilterState::PAUSED) {
        MEDIA_LOG_W("sink is not ready when start, state: %d", state_.load());
        return ERROR_STATE;
    }
    auto err = FilterBase::Start();
    if (err != SUCCESS) {
        MEDIA_LOG_E("audio sink filter start error");
        return err;
    }
    err = TranslatePluginStatus(plugin_->Start());
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, "audio sink plugin start failed");
    if (pushThreadIsBlocking.load()) {
        startWorkingCondition_.NotifyOne();
    }
    return SUCCESS;
}

ErrorCode AudioSinkFilter::Stop()
{
    MEDIA_LOG_I("audio sink stop start");
    FilterBase::Stop();
    plugin_->Stop();
    if (pushThreadIsBlocking.load()) {
        startWorkingCondition_.NotifyOne();
    }
    MEDIA_LOG_I("audio sink stop finish");
    return SUCCESS;
}

ErrorCode AudioSinkFilter::Pause()
{
    MEDIA_LOG_D("audio sink filter pause start");
    // only worked when state is working
    if (state_ != FilterState::READY && state_ != FilterState::RUNNING) {
        MEDIA_LOG_W("audio sink cannot pause when not working");
        return ERROR_STATE;
    }
    auto err = FilterBase::Pause();
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, "audio sink pause failed");
    err = TranslatePluginStatus(plugin_->Pause());
    MEDIA_LOG_D("audio sink filter pause end");
    return err;
}
ErrorCode AudioSinkFilter::Resume()
{
    MEDIA_LOG_D("audio sink filter resume");
    // only worked when state is paused
    if (state_ == FilterState::PAUSED) {
        state_ = FilterState::RUNNING;
        if (pushThreadIsBlocking) {
            startWorkingCondition_.NotifyOne();
        }
        return TranslatePluginStatus(plugin_->Resume());
    }
    return SUCCESS;
}

void AudioSinkFilter::FlushStart()
{
    MEDIA_LOG_D("audio sink flush start entered");
    isFlushing = true;
    if (pushThreadIsBlocking) {
        startWorkingCondition_.NotifyOne();
    }
    plugin_->Flush();
}

void AudioSinkFilter::FlushEnd()
{
    MEDIA_LOG_D("audio sink flush end entered");
    isFlushing = false;
}

ErrorCode AudioSinkFilter::SetVolume(float volume)
{
    if (state_ != FilterState::READY && state_ != FilterState::RUNNING && state_ != FilterState::PAUSED) {
        MEDIA_LOG_E("audio sink filter cannot set volume in state %d", state_.load());
        return ERROR_STATE;
    }
    MEDIA_LOG_W("set volume %.3f", volume);
    return TranslatePluginStatus(plugin_->SetVolume(volume));
}
} // namespace Pipeline
} // namespace Media
} // namespace OHOS
