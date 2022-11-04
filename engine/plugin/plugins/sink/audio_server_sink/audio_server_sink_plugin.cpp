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

#define HST_LOG_TAG "AudioServerSinkPlugin"

#include "audio_server_sink_plugin.h"
#include <functional>
#include "audio_errors.h"
#include "foundation/cpp_ext/algorithm_ext.h"
#include "foundation/log.h"
#include "foundation/osal/thread/scoped_lock.h"
#include "foundation/osal/utils/util.h"
#include "plugin/common/plugin_time.h"
#include "utils/constants.h"
#include "pipeline/core/plugin_attr_desc.h"

namespace {
using namespace OHOS::Media::Plugin;
constexpr uint32_t DEFAULT_OUTPUT_CHANNELS = 2;
constexpr AudioChannelLayout DEFAULT_OUTPUT_CHANNEL_LAYOUT = AudioChannelLayout::STEREO;
const std::pair<OHOS::AudioStandard::AudioSamplingRate, uint32_t> g_auSampleRateMap[] = {
    {OHOS::AudioStandard::SAMPLE_RATE_8000, 8000},
    {OHOS::AudioStandard::SAMPLE_RATE_11025, 11025},
    {OHOS::AudioStandard::SAMPLE_RATE_12000, 12000},
    {OHOS::AudioStandard::SAMPLE_RATE_16000, 16000},
    {OHOS::AudioStandard::SAMPLE_RATE_22050, 22050},
    {OHOS::AudioStandard::SAMPLE_RATE_24000, 24000},
    {OHOS::AudioStandard::SAMPLE_RATE_32000, 32000},
    {OHOS::AudioStandard::SAMPLE_RATE_44100, 44100},
    {OHOS::AudioStandard::SAMPLE_RATE_48000, 48000},
    {OHOS::AudioStandard::SAMPLE_RATE_64000, 64000},
    {OHOS::AudioStandard::SAMPLE_RATE_96000, 96000},
};

const std::pair<AudioInterruptMode, OHOS::AudioStandard::InterruptMode> g_auInterruptMap[] = {
    {AudioInterruptMode::SHARE_MODE, OHOS::AudioStandard::InterruptMode::SHARE_MODE},
    {AudioInterruptMode::INDEPENDENT_MODE, OHOS::AudioStandard::InterruptMode::INDEPENDENT_MODE},
};

const std::vector<std::tuple<AudioSampleFormat, OHOS::AudioStandard::AudioSampleFormat, AVSampleFormat>> g_aduFmtMap = {
    {AudioSampleFormat::S8, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_NONE},
    {AudioSampleFormat::U8, OHOS::AudioStandard::AudioSampleFormat::SAMPLE_U8, AV_SAMPLE_FMT_U8},
    {AudioSampleFormat::S8P, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_NONE},
    {AudioSampleFormat::U8P, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_U8P},
    {AudioSampleFormat::S16, OHOS::AudioStandard::AudioSampleFormat::SAMPLE_S16LE, AV_SAMPLE_FMT_S16},
    {AudioSampleFormat::U16, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_NONE},
    {AudioSampleFormat::S16P, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_S16P},
    {AudioSampleFormat::U16P, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_NONE},
    {AudioSampleFormat::S24, OHOS::AudioStandard::AudioSampleFormat::SAMPLE_S24LE, AV_SAMPLE_FMT_NONE},
    {AudioSampleFormat::U24, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_NONE},
    {AudioSampleFormat::S24P, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_NONE},
    {AudioSampleFormat::U24P, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_NONE},
    {AudioSampleFormat::S32, OHOS::AudioStandard::AudioSampleFormat::SAMPLE_S32LE, AV_SAMPLE_FMT_S32},
    {AudioSampleFormat::U32, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_NONE},
    {AudioSampleFormat::S32P, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_S32P},
    {AudioSampleFormat::U32P, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_NONE},
    {AudioSampleFormat::F32, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_FLT},
    {AudioSampleFormat::F32P, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_FLTP},
    {AudioSampleFormat::F64, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_DBL},
    {AudioSampleFormat::F64P, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_DBLP},
    {AudioSampleFormat::S64, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_S64},
    {AudioSampleFormat::U64, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_NONE},
    {AudioSampleFormat::S64P, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_S64P},
    {AudioSampleFormat::U64P, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_NONE},
};

const std::pair<OHOS::AudioStandard::AudioChannel, uint32_t> g_auChannelsMap[] = {
    {OHOS::AudioStandard::MONO, 1},
    {OHOS::AudioStandard::STEREO, 2},
};

bool SampleRateEnum2Num (OHOS::AudioStandard::AudioSamplingRate enumVal, uint32_t& numVal)
{
    for (const auto& item : g_auSampleRateMap) {
        if (item.first == enumVal) {
            numVal = item.second;
            return true;
        }
    }
    numVal = 0;
    return false;
}

bool SampleRateNum2Enum (uint32_t numVal, OHOS::AudioStandard::AudioSamplingRate& enumVal)
{
    for (const auto& item : g_auSampleRateMap) {
        if (item.second == numVal) {
            enumVal = item.first;
            return true;
        }
    }
    return false;
}

bool ChannelNumNum2Enum(uint32_t numVal, OHOS::AudioStandard::AudioChannel& enumVal)
{
    for (const auto& item : g_auChannelsMap) {
        if (item.second == numVal) {
            enumVal = item.first;
            return true;
        }
    }
    return false;
}

void AudioInterruptMode2InterruptMode(AudioInterruptMode audioInterruptMode,
                                      OHOS::AudioStandard::InterruptMode& interruptMode)
{
    for (const auto& item : g_auInterruptMap) {
        if (item.first == audioInterruptMode) {
            interruptMode = item.second;
        }
    }
}

std::shared_ptr<AudioSinkPlugin> AudioServerSinkPluginCreater(const std::string& name)
{
    return std::make_shared<OHOS::Media::Plugin::AuSrSinkPlugin::AudioServerSinkPlugin>(name);
}

void UpdateSupportedSampleRate(Capability& inCaps)
{
    auto supportedSampleRateList = OHOS::AudioStandard::AudioRenderer::GetSupportedSamplingRates();
    if (!supportedSampleRateList.empty()) {
        DiscreteCapability<uint32_t> values;
        for (const auto& rate : supportedSampleRateList) {
            uint32_t sampleRate = 0;
            if (SampleRateEnum2Num(rate, sampleRate)) {
                values.push_back(sampleRate);
            }
        }
        if (!values.empty()) {
            inCaps.AppendDiscreteKeys<uint32_t>(Capability::Key::AUDIO_SAMPLE_RATE, values);
        }
    }
}

void UpdateSupportedSampleFormat(Capability& inCaps)
{
    DiscreteCapability<AudioSampleFormat> values(g_aduFmtMap.size());
    for (const auto& item : g_aduFmtMap) {
        if (std::get<1>(item) != OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH ||
            std::get<2>(item) != AV_SAMPLE_FMT_NONE) {
            values.emplace_back(std::get<0>(item));
        }
    }
    inCaps.AppendDiscreteKeys(Capability::Key::AUDIO_SAMPLE_FORMAT, values);
}

Status AudioServerSinkRegister(const std::shared_ptr<Register>& reg)
{
    AudioSinkPluginDef definition;
    definition.name = "AudioServerSink";
    definition.description = "Audio sink for audio server of media standard";
    definition.rank = 100; // 100: max rank
    definition.creator = AudioServerSinkPluginCreater;
    Capability inCaps(OHOS::Media::MEDIA_MIME_AUDIO_RAW);
    UpdateSupportedSampleRate(inCaps);
    UpdateSupportedSampleFormat(inCaps);
    definition.inCaps.push_back(inCaps);
    return reg->AddPlugin(definition);
}

PLUGIN_DEFINITION(AudioServerSink, LicenseType::APACHE_V2, AudioServerSinkRegister, [] {});

inline void ResetAudioRendererParams(OHOS::AudioStandard::AudioRendererParams& param)
{
    using namespace OHOS::AudioStandard;
    param.sampleFormat = INVALID_WIDTH;
    param.sampleRate = SAMPLE_RATE_8000;
    param.channelCount = MONO;
    param.encodingType = ENCODING_INVALID;
}
} // namespace

namespace OHOS {
namespace Media {
namespace Plugin {
namespace AuSrSinkPlugin {
using namespace OHOS::Media::Plugin;


AudioServerSinkPlugin::AudioRendererCallbackImpl::AudioRendererCallbackImpl(Callback* cb, bool& isPaused)
    : callback_(cb), isPaused_(isPaused)
{
}

void AudioServerSinkPlugin::AudioRendererCallbackImpl::OnInterrupt(
    const OHOS::AudioStandard::InterruptEvent& interruptEvent)
{
    if (interruptEvent.forceType == OHOS::AudioStandard::INTERRUPT_FORCE) {
        switch (interruptEvent.hintType) {
            case OHOS::AudioStandard::INTERRUPT_HINT_PAUSE:
                isPaused_ = true;
                break;
            default:
                isPaused_ = false;
                break;
        }
    }
    auto audioInterruptEvent = AudioInterruptEvent {
        static_cast<uint32_t>(interruptEvent.eventType),
        static_cast<uint32_t>(interruptEvent.forceType),
        static_cast<uint32_t>(interruptEvent.hintType)
    };
    callback_->OnEvent(PluginEvent{PluginEventType::INTERRUPT, audioInterruptEvent, "Audio interrupt event"});
}

void AudioServerSinkPlugin::AudioRendererCallbackImpl::OnStateChange(const OHOS::AudioStandard::RendererState state,
    const AudioStandard::StateChangeCmdType __attribute__((unused)) cmdType)
{
    MEDIA_LOG_D("RenderState is " PUBLIC_LOG_U32, static_cast<uint32_t>(state));
}

AudioServerSinkPlugin::AudioServerSinkPlugin(std::string name)
    : Plugin::AudioSinkPlugin(std::move(name)), audioRenderer_(nullptr)
{
    rendererParams_.encodingType = AudioStandard::ENCODING_PCM;
}

AudioServerSinkPlugin::~AudioServerSinkPlugin()
{
    MEDIA_LOG_I("~AudioServerSinkPlugin() entered.");
    ReleaseRender();
}

Status AudioServerSinkPlugin::Init()
{
    MEDIA_LOG_I("Init entered.");
    OSAL::ScopedLock lock(renderMutex_);
    if (audioRenderer_ == nullptr) {
        AudioStandard::AppInfo appInfo;
        appInfo.appPid = appPid_;
        appInfo.appUid = appUid_;
        MEDIA_LOG_I("Create audio renderer for apppid_ " PUBLIC_LOG_D32 " appuid_ " PUBLIC_LOG_D32 " contentType "
            PUBLIC_LOG_D32 " streamUsage " PUBLIC_LOG_D32 " rendererFlags " PUBLIC_LOG_D32 " audioInterruptMode_ "
            PUBLIC_LOG_U32, appPid_, appUid_, audioRenderInfo_.contentType, audioRenderInfo_.streamUsage,
            audioRenderInfo_.rendererFlags, static_cast<uint32_t>(audioInterruptMode_));
        rendererOptions_.rendererInfo.contentType = static_cast<AudioStandard::ContentType>(
            audioRenderInfo_.contentType);
        rendererOptions_.rendererInfo.streamUsage = static_cast<AudioStandard::StreamUsage>(
            audioRenderInfo_.streamUsage);
        rendererOptions_.rendererInfo.rendererFlags = audioRenderInfo_.rendererFlags;
        rendererOptions_.streamInfo.samplingRate = AudioStandard::SAMPLE_RATE_8000;
        rendererOptions_.streamInfo.encoding = AudioStandard::ENCODING_PCM;
        rendererOptions_.streamInfo.format = AudioStandard::SAMPLE_S16LE;
        rendererOptions_.streamInfo.channels = AudioStandard::MONO;
        audioRenderer_ = AudioStandard::AudioRenderer::Create(rendererOptions_, appInfo);
        if (audioRenderer_ == nullptr) {
            MEDIA_LOG_E("Create audioRenderer_ fail");
            return Status::ERROR_UNKNOWN;
        }
        audioRenderer_->SetInterruptMode(audioInterruptMode_);
        if (audioRendererCallback_ == nullptr) {
            audioRendererCallback_ = std::make_shared<AudioRendererCallbackImpl>(callback_, isForcePaused_);
            audioRenderer_->SetRendererCallback(audioRendererCallback_);
        }
    }
    return Status::OK;
}

void AudioServerSinkPlugin::ReleaseRender()
{
    OSAL::ScopedLock lock(renderMutex_);
    if (audioRenderer_ != nullptr && audioRenderer_->GetStatus() != AudioStandard::RendererState::RENDERER_RELEASED) {
        if (!audioRenderer_->Release()) {
            MEDIA_LOG_W("release audio render failed");
            return;
        }
    }
    audioRenderer_.reset();
}

Status AudioServerSinkPlugin::Deinit()
{
    MEDIA_LOG_I("Deinit entered.");
    ReleaseRender();
    return Status::OK;
}

Status AudioServerSinkPlugin::Prepare()
{
    MEDIA_LOG_I("Prepare entered.");
    FALSE_RETURN_V_MSG_E(fmtSupported_, Status::ERROR_INVALID_PARAMETER, "sample fmt is not supported");
    if (bitsPerSample_ == 8 || bitsPerSample_ == 24) { // 8 24
        needReformat_ = true;
        rendererParams_.sampleFormat = reStdDestFmt_;
    }
    auto types = AudioStandard::AudioRenderer::GetSupportedEncodingTypes();
    if (!CppExt::AnyOf(types.begin(), types.end(), [](AudioStandard::AudioEncodingType tmp) -> bool {
        return tmp == AudioStandard::ENCODING_PCM;
    })) {
        MEDIA_LOG_E("audio renderer do not support pcm encoding");
        return Status::ERROR_INVALID_PARAMETER;
    }
    MEDIA_LOG_I("set param with fmt " PUBLIC_LOG_D32 " sampleRate " PUBLIC_LOG_D32 " channel " PUBLIC_LOG_D32
        " encode type " PUBLIC_LOG_D32,
        rendererParams_.sampleFormat, rendererParams_.sampleRate, rendererParams_.channelCount,
        rendererParams_.encodingType);
    {
        OSAL::ScopedLock lock(renderMutex_);
        auto ret = audioRenderer_->SetParams(rendererParams_);
        if (ret != AudioStandard::SUCCESS) {
            MEDIA_LOG_E("audio renderer SetParams() fail with " PUBLIC_LOG_D32, ret);
            return Status::ERROR_UNKNOWN;
        }
    }
    if (needReformat_) {
        resample_ = std::make_shared<Ffmpeg::Resample>();
        Ffmpeg::ResamplePara resamplePara {
            channels_,
            sampleRate_,
            bitsPerSample_,
            static_cast<int64_t>(channelLayout_),
            reSrcFfFmt_,
            samplesPerFrame_,
            reFfDestFmt_,
        };
        FALSE_RETURN_V_MSG(resample_->Init(resamplePara) == Status::OK, Status::ERROR_UNKNOWN, "Resample init error");
    }
    return Status::OK;
}

bool AudioServerSinkPlugin::StopRender()
{
    OSAL::ScopedLock lock(renderMutex_);
    if (audioRenderer_) {
        return audioRenderer_->Stop();
    }
    return true;
}

Status AudioServerSinkPlugin::Reset()
{
    MEDIA_LOG_I("Reset entered.");
    if (!StopRender()) {
        MEDIA_LOG_E("stop render error");
        return Status::ERROR_UNKNOWN;
    }
    ResetAudioRendererParams(rendererParams_);
    fmtSupported_ = false;
    reSrcFfFmt_ = AV_SAMPLE_FMT_NONE;
    channels_ = 0;
    bitRate_ = 0;
    sampleRate_ = 0;
    samplesPerFrame_ = 0;
    needReformat_ = false;
    if (resample_) {
        resample_.reset();
    }
    return Status::OK;
}

Status AudioServerSinkPlugin::Start()
{
    MEDIA_LOG_I("Start entered.");
    bool ret = false;
    OSAL::ScopedLock lock(renderMutex_);
    {
        if (audioRenderer_ == nullptr) {
            return Status::ERROR_WRONG_STATE;
        }
        ret = audioRenderer_->Start();
    }
    if (ret) {
        MEDIA_LOG_I("audioRenderer_ Start() success");
        return Status::OK;
    } else {
        MEDIA_LOG_E("audioRenderer_ Start() fail");
    }
    return Status::ERROR_UNKNOWN;
}

Status AudioServerSinkPlugin::Stop()
{
    MEDIA_LOG_I("Stop entered.");
    if (StopRender()) {
        MEDIA_LOG_I("stop render success");
        return Status::OK;
    } else {
        MEDIA_LOG_E("stop render failed");
    }
    return Status::ERROR_UNKNOWN;
}

Status AudioServerSinkPlugin::GetParameter(Tag tag, ValueType& para)
{
    MEDIA_LOG_I("GetParameter entered, key: " PUBLIC_LOG_S, Pipeline::Tag2String(tag));
    AudioStandard::AudioRendererParams params;
    OSAL::ScopedLock lock(renderMutex_);
    switch (tag) {
        case Tag::AUDIO_SAMPLE_RATE:
            if (audioRenderer_ && audioRenderer_->GetParams(params) == AudioStandard::SUCCESS) {
                if (params.sampleRate != rendererParams_.sampleRate) {
                    MEDIA_LOG_W("samplingRate has changed from " PUBLIC_LOG_U32 " to " PUBLIC_LOG_U32,
                                rendererParams_.sampleRate, params.sampleRate);
                }
                para = params.sampleRate;
            }
            break;
        case Tag::AUDIO_OUTPUT_CHANNELS:
            para = DEFAULT_OUTPUT_CHANNELS; // get the real output channels from audio server here
            MEDIA_LOG_I("Get outputChannels: " PUBLIC_LOG_U32, DEFAULT_OUTPUT_CHANNELS);
            break;
        case Tag::AUDIO_OUTPUT_CHANNEL_LAYOUT:
            para = DEFAULT_OUTPUT_CHANNEL_LAYOUT; // get the real output channel layout from audio server here
            MEDIA_LOG_I("Get outputChannelLayout: " PUBLIC_LOG_U64, DEFAULT_OUTPUT_CHANNEL_LAYOUT);
            break;
        case Tag::MEDIA_BITRATE:
            para = bitRate_;
            break;
        case Tag::AUDIO_SAMPLE_FORMAT:
            if (audioRenderer_ && audioRenderer_->GetParams(params) == AudioStandard::SUCCESS) {
                if (params.sampleFormat != rendererParams_.sampleFormat) {
                    MEDIA_LOG_W("sampleFormat has changed from " PUBLIC_LOG_U32 " to " PUBLIC_LOG_U32,
                                rendererParams_.sampleFormat, params.sampleFormat);
                }
                para = params.sampleFormat;
            }
            break;
        default:
            MEDIA_LOG_I("Unknown key");
            break;
    }
    return Status::OK;
}

bool AudioServerSinkPlugin::AssignSampleRateIfSupported(uint32_t sampleRate)
{
    sampleRate_ = sampleRate;
    AudioStandard::AudioSamplingRate aRate = AudioStandard::SAMPLE_RATE_8000;
    if (!SampleRateNum2Enum(sampleRate, aRate)) {
        MEDIA_LOG_E("sample rate " PUBLIC_LOG_U32 "not supported", sampleRate);
        return false;
    }
    auto supportedSampleRateList = OHOS::AudioStandard::AudioRenderer::GetSupportedSamplingRates();
    if (supportedSampleRateList.empty()) {
        MEDIA_LOG_E("GetSupportedSamplingRates() fail");
        return false;
    }
    for (const auto& rate : supportedSampleRateList) {
        if (rate == aRate) {
            rendererParams_.sampleRate = rate;
            MEDIA_LOG_D("sampleRate: " PUBLIC_LOG_U32, rendererParams_.sampleRate);
            return true;
        }
    }
    return false;
}

bool AudioServerSinkPlugin::AssignChannelNumIfSupported(uint32_t channelNum)
{
    AudioStandard::AudioChannel aChannel = AudioStandard::MONO;
    if (!ChannelNumNum2Enum(channelNum, aChannel)) {
        MEDIA_LOG_E("channel num " PUBLIC_LOG_U32 "not supported", channelNum);
        return false;
    }
    auto supportedChannelsList = OHOS::AudioStandard::AudioRenderer::GetSupportedChannels();
    if (supportedChannelsList.empty()) {
        MEDIA_LOG_E("GetSupportedChannels() fail");
        return false;
    }
    for (const auto& channel : supportedChannelsList) {
        if (channel == aChannel) {
            rendererParams_.channelCount = channel;
            MEDIA_LOG_D("channelCount: " PUBLIC_LOG_U32, rendererParams_.channelCount);
            return true;
        }
    }
    return false;
}

bool AudioServerSinkPlugin::AssignSampleFmtIfSupported(Plugin::AudioSampleFormat sampleFormat)
{
    const auto& item = std::find_if(g_aduFmtMap.begin(), g_aduFmtMap.end(), [&sampleFormat] (const auto& tmp) -> bool {
        return std::get<0>(tmp) == sampleFormat;
    });
    auto stdFmt = std::get<1>(*item);
    if (stdFmt == OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH) {
        if (std::get<2>(*item) == AV_SAMPLE_FMT_NONE) { // 2
            fmtSupported_ = false;
        } else {
            fmtSupported_ = true;
            needReformat_ = true;
            reSrcFfFmt_ = std::get<2>(*item); // 2
            rendererParams_.sampleFormat = reStdDestFmt_;
        }
    } else {
        auto supportedFmts = OHOS::AudioStandard::AudioRenderer::GetSupportedFormats();
        if (CppExt::AnyOf(supportedFmts.begin(), supportedFmts.end(), [&stdFmt](const auto& tmp) -> bool {
            return tmp == stdFmt;
        })) {
            fmtSupported_ = true;
            needReformat_ = false;
            rendererParams_.sampleFormat = stdFmt;
        } else {
            fmtSupported_ = false;
            needReformat_ = false;
        }
    }
    return fmtSupported_;
}

void AudioServerSinkPlugin::SetInterruptMode(AudioStandard::InterruptMode interruptMode)
{
    OSAL::ScopedLock lock(renderMutex_);
    if (audioRenderer_) {
        audioRenderer_->SetInterruptMode(interruptMode);
    }
}

Status AudioServerSinkPlugin::SetParameter(Tag tag, const ValueType& para)
{
    MEDIA_LOG_I("SetParameter entered, key: " PUBLIC_LOG_S, Pipeline::Tag2String(tag));
    switch (tag) {
        case Tag::AUDIO_SAMPLE_RATE:
            FALSE_RETURN_V_MSG_E(para.SameTypeWith(typeid(uint32_t)), Status::ERROR_MISMATCHED_TYPE,
                "sample rate type should be uint32_t");
            FALSE_RETURN_V_MSG_E(AssignSampleRateIfSupported(Plugin::AnyCast<uint32_t>(para)),
                Status::ERROR_INVALID_PARAMETER, "sampleRate isn't supported");
            break;
        case Tag::AUDIO_OUTPUT_CHANNELS:
            FALSE_RETURN_V_MSG_E(para.SameTypeWith(typeid(uint32_t)), Status::ERROR_MISMATCHED_TYPE,
                "channels type should be uint32_t");
            channels_ = Plugin::AnyCast<uint32_t>(para);
            MEDIA_LOG_I("Set outputChannels: " PUBLIC_LOG_U32, channels_);
            FALSE_RETURN_V_MSG_E(AssignChannelNumIfSupported(channels_), Status::ERROR_INVALID_PARAMETER,
                "channel isn't supported");
            break;
        case Tag::MEDIA_BITRATE:
            FALSE_RETURN_V_MSG_E(para.SameTypeWith(typeid(int64_t)), Status::ERROR_MISMATCHED_TYPE,
                "bit rate type should be int64_t");
            bitRate_ = Plugin::AnyCast<int64_t>(para);
            break;
        case Tag::AUDIO_SAMPLE_FORMAT:
            FALSE_RETURN_V_MSG_E(para.SameTypeWith(typeid(AudioSampleFormat)), Status::ERROR_MISMATCHED_TYPE,
                "AudioSampleFormat type should be AudioSampleFormat");
            FALSE_RETURN_V_MSG_E(AssignSampleFmtIfSupported(Plugin::AnyCast<AudioSampleFormat>(para)),
                Status::ERROR_INVALID_PARAMETER, "sampleFmt isn't supported by audio renderer or resample lib");
            break;
        case Tag::AUDIO_OUTPUT_CHANNEL_LAYOUT:
            FALSE_RETURN_V_MSG_E(para.SameTypeWith(typeid(AudioChannelLayout)), Status::ERROR_MISMATCHED_TYPE,
                "channel layout type should be AudioChannelLayout");
            channelLayout_ = Plugin::AnyCast<AudioChannelLayout>(para);
            MEDIA_LOG_I("Set outputChannelLayout: " PUBLIC_LOG_U64, channelLayout_);
            break;
        case Tag::AUDIO_SAMPLE_PER_FRAME:
            FALSE_RETURN_V_MSG_E(para.SameTypeWith(typeid(uint32_t)), Status::ERROR_MISMATCHED_TYPE,
                "SAMPLE_PER_FRAME type should be uint32_t");
            samplesPerFrame_ = Plugin::AnyCast<uint32_t>(para);
            break;
        case Tag::BITS_PER_CODED_SAMPLE:
            FALSE_RETURN_V_MSG_E(para.SameTypeWith(typeid(uint32_t)), Status::ERROR_MISMATCHED_TYPE,
                                 "BITS_PER_CODED_SAMPLE type should be uint32_t");
            bitsPerSample_ = Plugin::AnyCast<uint32_t>(para);
            break;
        case Tag::MEDIA_SEEKABLE:
            FALSE_RETURN_V_MSG_E(para.SameTypeWith(typeid(Seekable)), Status::ERROR_MISMATCHED_TYPE,
                                 "MEDIA_SEEKABLE type should be Seekable");
            seekable_ = Plugin::AnyCast<Plugin::Seekable>(para);
            break;
        case Tag::APP_PID:
            FALSE_RETURN_V_MSG_E(para.SameTypeWith(typeid(int32_t)), Status::ERROR_MISMATCHED_TYPE,
                "APP_PID type should be int32_t");
            appPid_ = Plugin::AnyCast<int32_t>(para);
            break;
        case Tag::APP_UID:
            FALSE_RETURN_V_MSG_E(para.SameTypeWith(typeid(int32_t)), Status::ERROR_MISMATCHED_TYPE,
                "APP_UID type should be int32_t");
            appUid_ = Plugin::AnyCast<int32_t>(para);
            break;
        case Tag::AUDIO_RENDER_INFO:
            FALSE_RETURN_V_MSG_E(para.SameTypeWith(typeid(AudioRenderInfo)), Status::ERROR_MISMATCHED_TYPE,
                                 "AUDIO_RENDER_INFO type should be AudioRenderInfo");
            audioRenderInfo_ = Plugin::AnyCast<AudioRenderInfo>(para);
            break;
        case Tag::AUDIO_INTERRUPT_MODE:
            FALSE_RETURN_V_MSG_E(para.SameTypeWith(typeid(AudioInterruptMode)), Status::ERROR_MISMATCHED_TYPE,
                                 "AUDIO_INTERRUPT_MODE type should be AudioInterruptMode");
            AudioInterruptMode2InterruptMode(Plugin::AnyCast<AudioInterruptMode>(para), audioInterruptMode_);
            SetInterruptMode(audioInterruptMode_);
            break;
        default:
            MEDIA_LOG_I("Unknown key");
            break;
    }
    return Status::OK;
}

Status AudioServerSinkPlugin::GetVolume(float& volume)
{
    MEDIA_LOG_I("GetVolume entered.");
    OSAL::ScopedLock lock(renderMutex_);
    if (audioRenderer_ != nullptr) {
        volume = audioRenderer_->GetVolume();
        return Status::OK;
    }
    return Status::ERROR_WRONG_STATE;
}

Status AudioServerSinkPlugin::SetVolume(float volume)
{
    MEDIA_LOG_I("SetVolume entered.");
    OSAL::ScopedLock lock(renderMutex_);
    if (audioRenderer_ != nullptr) {
        int32_t ret = audioRenderer_->SetVolume(volume);
        if (ret != OHOS::AudioStandard::SUCCESS) {
            MEDIA_LOG_E("set volume failed with code " PUBLIC_LOG_D32, ret);
            return Status::ERROR_UNKNOWN;
        }
        return Status::OK;
    }
    return Status::ERROR_WRONG_STATE;
}

Status AudioServerSinkPlugin::Resume()
{
    MEDIA_LOG_I("Resume entered.");
    return Start();
}

Status AudioServerSinkPlugin::Pause()
{
    MEDIA_LOG_I("Pause entered.");
    OSAL::ScopedLock lock(renderMutex_);
    if (audioRenderer_ != nullptr) {
        if (audioRenderer_->Pause()) {
            MEDIA_LOG_I("audio renderer pause success");
            return Status::OK;
        } else {
            MEDIA_LOG_E("audio renderer pause fail");
        }
    }
    return Status::ERROR_WRONG_STATE;
}

Status AudioServerSinkPlugin::GetLatency(uint64_t& hstTime)
{
    hstTime = 0; // set latency as 0 since latency of audio system is not reliable
    return Status::OK;
}

Status AudioServerSinkPlugin::Write(const std::shared_ptr<Buffer>& input)
{
    FALSE_RETURN_V_MSG_W(input != nullptr && !input->IsEmpty(), Status::OK, "Receive empty buffer."); // return ok
    auto mem = input->GetMemory();
    auto srcBuffer = mem->GetReadOnlyData();
    auto destBuffer = const_cast<uint8_t*>(srcBuffer);
    auto srcLength = mem->GetSize();
    auto destLength = srcLength;
    if (needReformat_ && resample_ && srcLength >0) {
        FALSE_LOG(resample_->Convert(srcBuffer, srcLength, destBuffer, destLength) == Status::OK);
    }
    MEDIA_LOG_DD("write data size " PUBLIC_LOG_ZU, destLength);
    while (isForcePaused_ && seekable_ == Seekable::SEEKABLE) {
        OSAL::SleepFor(5); // 5ms
        continue;
    }
    int32_t ret = 0;
    OSAL::ScopedLock lock(renderMutex_);
    FALSE_RETURN_V(audioRenderer_ != nullptr, Status::ERROR_WRONG_STATE);
    for (; destLength > 0;) {
        ret = audioRenderer_->Write(destBuffer, destLength);
        if (ret < 0) {
            MEDIA_LOG_E("Write data error ret is: " PUBLIC_LOG_D32, ret);
            break;
        } else if (static_cast<size_t>(ret) < destLength) {
            OSAL::SleepFor(5); // 5ms
        }
        destBuffer += ret;
        destLength -= ret;
        MEDIA_LOG_DD("written data size " PUBLIC_LOG_D32, ret);
    }
    return ret >= 0 ? Status::OK : Status::ERROR_UNKNOWN;
}

Status AudioServerSinkPlugin::Flush()
{
    MEDIA_LOG_I("Flush entered.");
    OSAL::ScopedLock lock(renderMutex_);
    if (audioRenderer_ == nullptr) {
        return Status::ERROR_WRONG_STATE;
    }
    if (audioRenderer_->Flush()) {
        MEDIA_LOG_I("audioRenderer_ Flush() success");
        return Status::OK;
    }
    MEDIA_LOG_E("audioRenderer_ Flush() fail");
    return Status::ERROR_UNKNOWN;
}

Status AudioServerSinkPlugin::Drain()
{
    MEDIA_LOG_I("Drain entered.");
    OSAL::ScopedLock lock(renderMutex_);
    if (audioRenderer_ == nullptr) {
        return Status::ERROR_WRONG_STATE;
    }
    auto res = audioRenderer_->Drain();
    uint64_t latency = 0;
    audioRenderer_->GetLatency(latency);
    latency /= 1000; // 1000 cast into ms
    if (!res || latency > 50) { // 50 latency too large
        MEDIA_LOG_W("drain failed or latency is too large, will sleep " PUBLIC_LOG_U64 " ms, aka. latency", latency);
        OSAL::SleepFor(latency);
    }
    MEDIA_LOG_I("audioRenderer_ Drain() success");
    return Status::OK;
}
} // namespace AuSrSinkPlugin
} // Plugin
} // namespace Media
} // namespace OHOS
