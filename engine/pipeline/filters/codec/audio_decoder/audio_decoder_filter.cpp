/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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

#define HST_LOG_TAG "AudioDecoderFilter"

#include "pipeline/filters/codec/audio_decoder/audio_decoder_filter.h"
#include "foundation/log.h"
#include "foundation/osal/utils/util.h"
#include "foundation/utils/steady_clock.h"
#include "pipeline/factory/filter_factory.h"
#include "pipeline/filters/codec/codec_filter_factory.h"

namespace {
constexpr uint32_t DEFAULT_IN_BUFFER_POOL_SIZE = 5;
constexpr uint32_t DEFAULT_OUT_BUFFER_POOL_SIZE = 5;
constexpr int32_t MAX_SAMPLE_PER_FRAME = 10240; // 10240 set max samples per frame
};

namespace OHOS {
namespace Media {
namespace Pipeline {
#ifdef OHOS_LITE
static AutoRegisterFilter<AudioDecoderFilter> g_registerAudioDecoderFilter("builtin.player.audiodecoder",
    [](const std::string& name) { return CreateCodecFilter(name, FilterCodecMode::AUDIO_SYNC_DECODER); });
#else
static AutoRegisterFilter<AudioDecoderFilter> g_registerAudioDecoderFilter("builtin.player.audiodecoder",
    [](const std::string& name) { return CreateCodecFilter(name, FilterCodecMode::AUDIO_ASYNC_DECODER); });
#endif

AudioDecoderFilter::AudioDecoderFilter(const std::string& name, std::shared_ptr<CodecMode> codecMode)
    : CodecFilterBase(name)
{
    MEDIA_LOG_D("audio decoder ctor called");
    filterType_ = FilterType::AUDIO_DECODER;
    bufferMetaType_ = Plugin::BufferMetaType::AUDIO;
    pluginType_ = Plugin::PluginType::AUDIO_DECODER;
    codecMode_ = std::move(codecMode);
}

AudioDecoderFilter::~AudioDecoderFilter()
{
    MEDIA_LOG_D("audio decoder dtor called");
    if (plugin_) {
        plugin_->Stop();
        plugin_->Deinit();
    }
    (void)codecMode_->Release();
}

ErrorCode AudioDecoderFilter::Prepare()
{
    MEDIA_LOG_I("audio decoder prepare called.");
#ifndef OHOH_LITE
    codecMode_->SetBufferPoolSize(static_cast<uint32_t>(DEFAULT_IN_BUFFER_POOL_SIZE),
                                  static_cast<uint32_t>(DEFAULT_OUT_BUFFER_POOL_SIZE));
#endif
    (void)codecMode_->Prepare();
    return CodecFilterBase::Prepare();
}

ErrorCode AudioDecoderFilter::Stop()
{
    MEDIA_LOG_D("audio decoder stop start.");
    FAIL_RETURN(CodecFilterBase::Stop());
    MEDIA_LOG_D("audio decoder stop end.");
    return ErrorCode::SUCCESS;
}

bool AudioDecoderFilter::Negotiate(const std::string& inPort,
                                   const std::shared_ptr<const Plugin::Capability>& upstreamCap,
                                   Plugin::Capability& negotiatedCap,
                                   const Plugin::Meta& upstreamParams,
                                   Plugin::Meta& downstreamParams)
{
    FALSE_RETURN_V(CodecFilterBase::Negotiate(inPort, upstreamCap, negotiatedCap, upstreamParams, downstreamParams),
                   false);
    MEDIA_LOG_D("audio decoder negotiate end");
    return true;
}

bool AudioDecoderFilter::Configure(const std::string& inPort, const std::shared_ptr<const Plugin::Meta>& upstreamMeta,
                                   Plugin::Meta& upstreamParams, Plugin::Meta& downstreamParams)
{
    PROFILE_BEGIN("audio decoder configure begin");
    FALSE_RETURN_V(CodecFilterBase::Configure(inPort, upstreamMeta, upstreamParams, downstreamParams), false);
    PROFILE_END("audio decoder configure end");
    return true;
}

void AudioDecoderFilter::FlushStart()
{
    MEDIA_LOG_D("audio decoder FlushStart entered.");
    codecMode_->FlushStart();
    CodecFilterBase::FlushStart();
    MEDIA_LOG_D("audio decoder FlushStart exit.");
}

void AudioDecoderFilter::FlushEnd()
{
    MEDIA_LOG_I("audio decoder FlushEnd entered");
    codecMode_->FlushEnd();
    CodecFilterBase::FlushEnd();
}

void AudioDecoderFilter::OnInputBufferDone(const std::shared_ptr<Plugin::Buffer>& input)
{
    MEDIA_LOG_D("AudioDecoderFilter::OnInputBufferDone");
}

void AudioDecoderFilter::OnOutputBufferDone(const std::shared_ptr<Plugin::Buffer>& output)
{
    codecMode_->OnOutputBufferDone(output);
}

uint32_t AudioDecoderFilter::CalculateBufferSize(const std::shared_ptr<const Plugin::Meta>& meta)
{
    using namespace OHOS::Media;
    uint32_t samplesPerFrame;
    FALSE_RETURN_V(meta->Get<Plugin::Tag::AUDIO_SAMPLE_PER_FRAME>(samplesPerFrame), 0);

    uint32_t channels;
    FALSE_RETURN_V(meta->Get<Plugin::Tag::AUDIO_CHANNELS>(channels), 0);

    Plugin::AudioSampleFormat format;
    FALSE_RETURN_V(meta->Get<Plugin::Tag::AUDIO_SAMPLE_FORMAT>(format), 0);

    return Pipeline::GetBytesPerSample(format) * samplesPerFrame * channels;
}

std::vector<Capability::Key> AudioDecoderFilter::GetRequiredOutCapKeys()
{
    std::vector<Capability::Key> capKey;
    capKey.push_back(Capability::Key::AUDIO_SAMPLE_FORMAT);
    return capKey;
}

void AudioDecoderFilter::UpdateParams(const std::shared_ptr<const Plugin::Meta>& upMeta,
                                      std::shared_ptr<Plugin::Meta>& meta)
{
    uint32_t samplesPerFrame = 0;
    if (GetPluginParameterLocked(Tag::AUDIO_SAMPLE_PER_FRAME, samplesPerFrame) != ErrorCode::SUCCESS) {
        MEDIA_LOG_W("Can't acquire samples per frame from decoder plugin: " PUBLIC_LOG_S, pluginInfo_->name.c_str());
        samplesPerFrame = MAX_SAMPLE_PER_FRAME;
    }
    FALSE_LOG_MSG(meta->Set<Plugin::Tag::AUDIO_SAMPLE_PER_FRAME>(samplesPerFrame),
                  "Set per sample frame failed.");
    bool useStreamChannelParams {false};
    auto iter = sinkParams_.Find(Plugin::Tag::AUDIO_OUTPUT_CHANNELS);
    if (iter != std::end(sinkParams_) && Plugin::Any::IsSameTypeWith<uint32_t>(iter->second)) {
        auto outputChannels = Plugin::AnyCast<uint32_t>(iter->second);
        uint32_t upChannels {0};
        FALSE_LOG(upMeta->Get<Plugin::Tag::AUDIO_CHANNELS>(upChannels));
        if (upChannels < outputChannels) {
            outputChannels = upChannels;
            useStreamChannelParams = true;
        }
        if (plugin_ != nullptr &&
            plugin_->SetParameter(Plugin::Tag::AUDIO_OUTPUT_CHANNELS, outputChannels) != Plugin::Status::OK) {
            MEDIA_LOG_W("Set outputChannels to plugin " PUBLIC_LOG_S " failed", plugin_->GetName().c_str());
        }
        FALSE_LOG_MSG(meta->Set<Plugin::Tag::AUDIO_OUTPUT_CHANNELS>(outputChannels), "Set channel failed.");
    }
    iter = sinkParams_.Find(Plugin::Tag::AUDIO_OUTPUT_CHANNEL_LAYOUT);
    if (iter != std::end(sinkParams_) && Plugin::Any::IsSameTypeWith<Plugin::AudioChannelLayout>(iter->second)) {
        auto outputChanLayout = Plugin::AnyCast<Plugin::AudioChannelLayout>(iter->second);
        Plugin::AudioChannelLayout upAudioChannelLayout;
        if (useStreamChannelParams && upMeta->Get<Plugin::Tag::AUDIO_CHANNEL_LAYOUT>(upAudioChannelLayout)) {
            outputChanLayout = upAudioChannelLayout;
        }
        if (plugin_ != nullptr &&
            plugin_->SetParameter(Plugin::Tag::AUDIO_OUTPUT_CHANNEL_LAYOUT, outputChanLayout) != Plugin::Status::OK) {
            MEDIA_LOG_W("Set outputChannelLayout to plugin " PUBLIC_LOG_S " failed", plugin_->GetName().c_str());
        }
        FALSE_LOG(meta->Set<Plugin::Tag::AUDIO_OUTPUT_CHANNEL_LAYOUT>(outputChanLayout));
    }
}
} // Pipeline
} // Media
} // OHOS