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

#define LOG_TAG "SdlAudioSinkPlugin"

#include "sdl_audio_sink_plugin.h"
#include <functional>
#include "foundation/constants.h"
#include "foundation/log.h"
#include "plugin/common/plugin_audio_tags.h"
#include "plugin/common/plugin_buffer.h"
#include "plugins/ffmpeg_adapter/utils/ffmpeg_utils.h"

namespace {
using namespace OHOS::Media::Plugin;

constexpr int MAX_AUDIO_FRAME_SIZE = 192000;
std::function<void(void*, uint8_t*, int)> g_audioCallback;

void RegisterAudioCallback(std::function<void(void*, uint8_t*, int)> callback)
{
    g_audioCallback = std::move(callback);
}
void SDLAudioCallback(void* userdata, uint8_t* stream, int len)
{
    g_audioCallback(userdata, stream, len);
}
AVSampleFormat TranslateFormat(AudioSampleFormat format)
{
    switch (format) {
        case AudioSampleFormat::F32:
            return AV_SAMPLE_FMT_FLT;
        case AudioSampleFormat::F32P:
            return AV_SAMPLE_FMT_FLTP;
        case AudioSampleFormat::F64:
            return AV_SAMPLE_FMT_DBL;
        case AudioSampleFormat::F64P:
            return AV_SAMPLE_FMT_DBLP;
        case AudioSampleFormat::S32:
            return AV_SAMPLE_FMT_S32;
        case AudioSampleFormat::S32P:
            return AV_SAMPLE_FMT_S32P;
        case AudioSampleFormat::S16:
            return AV_SAMPLE_FMT_S16;
        case AudioSampleFormat::S16P:
            return AV_SAMPLE_FMT_S16P;
        default:
            return AV_SAMPLE_FMT_NONE;
    }
}

bool IsPlanes(AudioSampleFormat format)
{
    switch (format) {
        case AudioSampleFormat::F32P:
        case AudioSampleFormat::F64P:
        case AudioSampleFormat::S32P:
        case AudioSampleFormat::S16P:
            return true;
        default:
            return false;
    }
}

std::shared_ptr<AudioSinkPlugin> AudioSinkPluginCreator(const std::string& name)
{
    return std::make_shared<SdlAudioSinkPlugin>(name);
}

const Status SdlAudioRegister(const std::shared_ptr<Register>& reg)
{
    AudioSinkPluginDef definition;
    definition.name = "sdl_audio_sink";
    definition.rank = 100; // 100
    definition.inCaps.emplace_back(Capability(OHOS::Media::MEDIA_MIME_AUDIO_RAW));
    definition.creator = AudioSinkPluginCreator;
    return reg->AddPlugin(definition);
}

PLUGIN_DEFINITION(SdlAudioSink, LicenseType::LGPL, SdlAudioRegister, [] {});
} // namespace

namespace OHOS {
namespace Media {
namespace Plugin {
SdlAudioSinkPlugin::SdlAudioSinkPlugin(std::string name)
    : aliasName_(std::move(name)),
      transformCache_((MAX_AUDIO_FRAME_SIZE * 3) / 2), // 3, 2
      mixCache_((MAX_AUDIO_FRAME_SIZE * 3) / 2)        // 3, 2
{
}
Status SdlAudioSinkPlugin::Init()
{
    std::weak_ptr<SdlAudioSinkPlugin> weakPtr(shared_from_this());
    RegisterAudioCallback([weakPtr](void* userdata, uint8_t* stream, int len) {
        auto ptr = weakPtr.lock();
        if (ptr) {
            ptr->AudioCallback(userdata, stream, len);
        }
    });
    return Status::OK;
}

Status SdlAudioSinkPlugin::Deinit()
{
    return Status::OK;
}

Status SdlAudioSinkPlugin::Prepare()
{
    AVSampleFormat outSampleFmt = AV_SAMPLE_FMT_S16;
    uint64_t outChannelLayout = AV_CH_LAYOUT_STEREO;
    int outChannels = av_get_channel_layout_nb_channels(outChannelLayout);
    avFrameSize_ = av_samples_get_buffer_size(nullptr, outChannels, samplesPerFrame_, outSampleFmt, 1);

    rb = MemoryHelper::make_unique<RingBuffer>(avFrameSize_ * 10); // 最大缓存10帧
    rb->Init();

    wantedSpec_.freq = sampleRate_;
    wantedSpec_.format = AUDIO_S16SYS;
    wantedSpec_.channels = outChannels;
    wantedSpec_.samples = samplesPerFrame_;
    wantedSpec_.silence = 0;
    wantedSpec_.callback = SDLAudioCallback;
    if (SDL_OpenAudio(&wantedSpec_, nullptr) < 0) {
        MEDIA_LOG_E("sdl cannot open audio with error: %s", SDL_GetError());
        return Status::ERROR_UNKNOWN;
    }

    SwrContext* swrContext = swr_alloc();
    if (swrContext == nullptr) {
        MEDIA_LOG_E("cannot allocate swr context");
        return Status::ERROR_NO_MEMORY;
    }
    AVSampleFormat sampleFormat = TranslateFormat(audioFormat_);
    MEDIA_LOG_I("configure swr with outChannelLayout 0x%x, outSampleFmt %d, outSampleRate %d inChannelLayout 0x%x, "
                "inSampleFormat %d, inSampleRate %d",
                outChannelLayout, outSampleFmt, sampleRate_, channelMask_, sampleFormat, sampleRate_);
    swrContext = swr_alloc_set_opts(swrContext, outChannelLayout, outSampleFmt, sampleRate_, channelMask_, sampleFormat,
                                    sampleRate_, 0, nullptr);
    if (swr_init(swrContext) != 0) {
        MEDIA_LOG_E("swr init error");
        return Status::ERROR_UNKNOWN;
    }
    swrCtx_ = std::shared_ptr<SwrContext>(swrContext, [](SwrContext* ptr) {
        if (ptr) {
            swr_free(&ptr);
        }
    });
    return Status::OK;
}

Status SdlAudioSinkPlugin::Reset()
{
    return Status::OK;
}

Status SdlAudioSinkPlugin::Start()
{
    MEDIA_LOG_I("SDL SINK start...");
    SDL_PauseAudio(0);
    rb->SetActive(true);
    return Status::OK;
}

Status SdlAudioSinkPlugin::Stop()
{
    SDL_PauseAudio(1);
    Flush();
    SDL_CloseAudio();
    SDL_Quit();
    return Status::OK;
}

bool SdlAudioSinkPlugin::IsParameterSupported(Tag tag)
{
    return false;
}

Status SdlAudioSinkPlugin::GetParameter(Tag tag, ValueType& value)
{
    return Status::ERROR_ALREADY_EXISTS;
}

Status SdlAudioSinkPlugin::SetParameter(Tag tag, const ValueType& value)
{
#define RETURN_ERROR_IF_CHECK_ERROR(typenames)                                                                         \
    if (value.Type() != typeid(typenames)) {                                                                           \
        return Status::ERROR_MISMATCHED_TYPE;                                                                          \
    }

    switch (tag) {
        case Tag::AUDIO_CHANNELS: {
            RETURN_ERROR_IF_CHECK_ERROR(uint32_t);
            channels_ = Plugin::AnyCast<uint32_t>(value);
            break;
        }
        case Tag::AUDIO_SAMPLE_RATE: {
            RETURN_ERROR_IF_CHECK_ERROR(uint32_t);
            sampleRate_ = Plugin::AnyCast<uint32_t>(value);
            break;
        }
        case Tag::AUDIO_SAMPLE_PRE_FRAME: {
            RETURN_ERROR_IF_CHECK_ERROR(uint32_t);
            samplesPerFrame_ = Plugin::AnyCast<uint32_t>(value);
            break;
        }
        case Tag::AUDIO_CHANNEL_LAYOUT: {
            RETURN_ERROR_IF_CHECK_ERROR(AudioChannelLayout);
            auto channelLayout = Plugin::AnyCast<AudioChannelLayout>(value);
            channelMask_ = ConvertChannelLayoutToFFmpeg(channelLayout);
            break;
        }
        case Tag::AUDIO_SAMPLE_FORMAT: {
            RETURN_ERROR_IF_CHECK_ERROR(AudioSampleFormat);
            audioFormat_ = Plugin::AnyCast<AudioSampleFormat>(value);
            break;
        }
        default:
            MEDIA_LOG_I("receive one parameter with unconcern key");
            break;
    }
    return Status::OK;
}

std::shared_ptr<Allocator> SdlAudioSinkPlugin::GetAllocator()
{
    return nullptr;
}

Status SdlAudioSinkPlugin::SetCallback(const std::shared_ptr<Callback>& cb)
{
    return Status::ERROR_ALREADY_EXISTS;
}

Status SdlAudioSinkPlugin::GetMute(bool& mute)
{
    return Status::ERROR_ALREADY_EXISTS;
}

Status SdlAudioSinkPlugin::SetMute(bool mute)
{
    return Status::ERROR_ALREADY_EXISTS;
}

Status SdlAudioSinkPlugin::GetVolume(float& volume)
{
    return Status::ERROR_ALREADY_EXISTS;
}

Status SdlAudioSinkPlugin::SetVolume(float volume)
{
    return Status::ERROR_ALREADY_EXISTS;
}

Status SdlAudioSinkPlugin::GetSpeed(float& speed)
{
    return Status::ERROR_ALREADY_EXISTS;
}

Status SdlAudioSinkPlugin::SetSpeed(float speed)
{
    return Status::ERROR_ALREADY_EXISTS;
}

Status SdlAudioSinkPlugin::Pause()
{
    SDL_PauseAudio(1);
    return Status::OK;
}

Status SdlAudioSinkPlugin::Resume()
{
    SDL_PauseAudio(0);
    return Status::OK;
}

Status SdlAudioSinkPlugin::GetLatency(uint64_t& ms)
{
    return Status::ERROR_ALREADY_EXISTS;
}

Status SdlAudioSinkPlugin::GetFrameSize(size_t& size)
{
    return Status::ERROR_ALREADY_EXISTS;
}

Status SdlAudioSinkPlugin::GetFrameCount(uint32_t& count)
{
    return Status::ERROR_ALREADY_EXISTS;
}

Status SdlAudioSinkPlugin::Write(const std::shared_ptr<Buffer>& inputInfo)
{
    MEDIA_LOG_D("SdlSink Write begin");
    if (inputInfo == nullptr || inputInfo->IsEmpty()) {
        return Status::OK;
    }
    auto bufferData = inputInfo->GetMemory();
    auto dataPtr = transformCache_.data();
    std::vector<const uint8_t*> input(channels_);
    input[0] = static_cast<const uint8_t*>(bufferData->GetReadOnlyData());
    if (IsPlanes(audioFormat_)) {
        size_t planeSize = bufferData->GetSize() / channels_;
        for (auto i = 1; i < channels_; ++i) {
            input[i] = input[i - 1] + planeSize;
        }
    }
    swr_convert(swrCtx_.get(), &dataPtr, MAX_AUDIO_FRAME_SIZE, (const uint8_t**)input.data(), samplesPerFrame_);
    MEDIA_LOG_D("SdlSink Write before ring buffer");
    rb->WriteBuffer(transformCache_.data(), avFrameSize_);
    MEDIA_LOG_D("SdlSink Write end");
    return Status::OK;
}

Status SdlAudioSinkPlugin::Flush()
{
    SDL_ClearQueuedAudio(1);
    return Status::OK;
}

void SdlAudioSinkPlugin::AudioCallback(void* userdata, uint8_t* stream, int len)
{
    MEDIA_LOG_D("sdl audio callback begin");
    auto realLen = rb->ReadBuffer(mixCache_.data(), len);
    if (realLen == 0) {
        MEDIA_LOG_D("sdl audio callback end with 0");
        return;
    }
    SDL_memset(stream, 0, len);
    SDL_MixAudio(stream, mixCache_.data(), realLen, SDL_MIX_MAXVOLUME);
    SDL_PauseAudio(0);
    MEDIA_LOG_D("sdl audio callback end with %zu", realLen);
}
} // namespace Plugin
} // namespace Media
} // namespace OHOS