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
#include "foundation/log.h"
#include "utils/constants.h"

namespace {
using namespace OHOS::Media::Plugin;

constexpr uint64_t TIME_MS_TO_NS = 1000000;

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

const std::pair<OHOS::AudioStandard::AudioSampleFormat, AudioSampleFormat> g_aduFmtMap[] = {
    {OHOS::AudioStandard::SAMPLE_U8, AudioSampleFormat::U8},
    {OHOS::AudioStandard::SAMPLE_S16LE, AudioSampleFormat::S16},
    {OHOS::AudioStandard::SAMPLE_S24LE, AudioSampleFormat::S24},
    {OHOS::AudioStandard::SAMPLE_S32LE, AudioSampleFormat::S32}
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

bool ChannelNumEnum2Num(OHOS::AudioStandard::AudioChannel enumVal, uint32_t& numVal)
{
    for (const auto& item : g_auChannelsMap) {
        if (item.first == enumVal) {
            numVal = item.second;
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

bool SampleFmt2PluginFmt(OHOS::AudioStandard::AudioSampleFormat aFmt, AudioSampleFormat& pFmt)
{
    for (const auto& item : g_aduFmtMap) {
        if (item.first == aFmt) {
            pFmt = item.second;
            return true;
        }
    }
    return false;
}

bool PluginFmt2SampleFmt(AudioSampleFormat pFmt, OHOS::AudioStandard::AudioSampleFormat& aFmt)
{
    for (const auto& item : g_aduFmtMap) {
        if (item.second == pFmt) {
            aFmt = item.first;
            return true;
        }
    }
    return false;
}

std::shared_ptr<AudioSinkPlugin> AudioServerSinkPluginCreater(const std::string& name)
{
    return std::make_shared<OHOS::Media::AuSrSinkPlugin::AudioServerSinkPlugin>(name);
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

void UpdateSupportedChannels(Capability& inCaps)
{
    auto supportedChannelsList = OHOS::AudioStandard::AudioRenderer::GetSupportedChannels();
    if (!supportedChannelsList.empty()) {
        DiscreteCapability<uint32_t> values;
        for (const auto& channel : supportedChannelsList) {
            uint32_t channelNum = 0;
            if (ChannelNumEnum2Num(channel, channelNum)) {
                values.push_back(channelNum);
            }
        }
        if (!values.empty()) {
            inCaps.AppendDiscreteKeys<uint32_t>(Capability::Key::AUDIO_CHANNELS, values);
        }
    }
}

void UpdateSupportedSampleFormat(Capability& inCaps)
{
    auto supportedFormatsList = OHOS::AudioStandard::AudioRenderer::GetSupportedFormats();
    if (!supportedFormatsList.empty()) {
        DiscreteCapability<AudioSampleFormat> values;
        for (const auto& fmt : supportedFormatsList) {
            auto pFmt = AudioSampleFormat::U8;
            if (SampleFmt2PluginFmt(fmt, pFmt)) {
                values.emplace_back(pFmt);
            }
        }
        if (!values.empty()) {
            inCaps.AppendDiscreteKeys<AudioSampleFormat>(Capability::Key::AUDIO_SAMPLE_FORMAT, values);
        }
    }
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
    UpdateSupportedChannels(inCaps);
    UpdateSupportedSampleFormat(inCaps);
    definition.inCaps.push_back(inCaps);
    return reg->AddPlugin(definition);
}

PLUGIN_DEFINITION(AudioServerSink, LicenseType::APACHE_V2, AudioServerSinkRegister, [] {});
} // namespace

namespace OHOS {
namespace Media {
namespace AuSrSinkPlugin {
using namespace OHOS::Media::Plugin;

AudioServerSinkPlugin::AudioServerSinkPlugin(std::string name)
    : Plugin::AudioSinkPlugin(std::move(name)), audioRenderer_(nullptr)
{
}

AudioServerSinkPlugin::~AudioServerSinkPlugin()
{
    MEDIA_LOG_I("~AudioServerSinkPlugin() entered.");
    (void)Deinit();
}

Status AudioServerSinkPlugin::Init()
{
    MEDIA_LOG_I("Init entered.");
    if (audioRenderer_ == nullptr) {
        audioRenderer_ = AudioStandard::AudioRenderer::Create(AudioStandard::AudioStreamType::STREAM_MUSIC);
        if (audioRenderer_ == nullptr) {
            MEDIA_LOG_E("Create audioRenderer_ fail");
            return Status::ERROR_UNKNOWN;
        }
    }
    return Status::OK;
}

Status AudioServerSinkPlugin::Deinit()
{
    MEDIA_LOG_I("Deinit entered.");
    if (audioRenderer_ != nullptr) {
        if (audioRenderer_->Release()) {
            MEDIA_LOG_I("audioRenderer_ released");
        } else {
            MEDIA_LOG_E("Release audioRenderer_ fail");
        }
        audioRenderer_ = nullptr;
        return Status::OK;
    }
    return Status::ERROR_UNKNOWN;
}

Status AudioServerSinkPlugin::Prepare()
{
    MEDIA_LOG_I("Prepare entered.");
    AudioStandard::AudioRendererParams params;
    auto supportedEncodingTypes = AudioStandard::AudioRenderer::GetSupportedEncodingTypes();
    for (auto& supportedEncodingType : supportedEncodingTypes) {
        if (supportedEncodingType == AudioStandard::ENCODING_PCM) {
            params.encodingType = AudioStandard::ENCODING_PCM;
            break;
        }
    }
    if (params.encodingType != AudioStandard::ENCODING_PCM) {
        MEDIA_LOG_E("audioRenderer_ do not support pcm encoding");
        return Status::ERROR_UNKNOWN;
    }
    if (audioRenderer_->SetParams(params)) {
        MEDIA_LOG_I("audioRenderer_ SetParams() success");
    } else {
        MEDIA_LOG_E("audioRenderer_ SetParams() fail");
        return Status::ERROR_UNKNOWN;
    }
    return Status::OK;
}

Status AudioServerSinkPlugin::Reset()
{
    MEDIA_LOG_I("Reset entered.");
    if (audioRenderer_->GetStatus() == AudioStandard::RendererState::RENDERER_RUNNING) {
        if (!audioRenderer_->Stop()) {
            MEDIA_LOG_E("Stop audioRenderer_ fail");
            return Status::ERROR_UNKNOWN;
        }
        MEDIA_LOG_I("Stop audioRenderer_ success");
    }
    bitRate_ = 0;
    return Status::OK;
}

Status AudioServerSinkPlugin::Start()
{
    MEDIA_LOG_I("Start entered.");
    if (audioRenderer_ != nullptr) {
        if (audioRenderer_->Start()) {
            MEDIA_LOG_I("audioRenderer_ Start() success");
            return Status::OK;
        } else {
            MEDIA_LOG_E("audioRenderer_ Start() fail");
        }
    }
    return Status::ERROR_UNKNOWN;
}

Status AudioServerSinkPlugin::Stop()
{
    MEDIA_LOG_I("Stop entered.");
    if (audioRenderer_ != nullptr) {
        if (audioRenderer_->Stop()) {
            MEDIA_LOG_I("audioRenderer_ Stop() success");
            return Status::OK;
        } else {
            MEDIA_LOG_E("audioRenderer_ Stop() fail");
        }
    }
    return Status::ERROR_UNKNOWN;
}

Status AudioServerSinkPlugin::GetParameter(Tag tag, ValueType& value)
{
    MEDIA_LOG_I("GetParameter entered.");
    AudioStandard::AudioRendererParams params;
    if (!audioRenderer_ || !audioRenderer_->GetParams(params)) {
        MEDIA_LOG_E("audioRenderer_ GetParams() fail");
        return Status::ERROR_UNKNOWN;
    }
    switch (tag) {
        case Tag::AUDIO_SAMPLE_RATE: {
            if (params.sampleRate != rendererParams_.sampleRate) {
                MEDIA_LOG_W("samplingRate has changed from %" PUBLIC_OUTPUT "u to %" PUBLIC_OUTPUT "u",
                            rendererParams_.sampleRate, params.sampleRate);
            }
            value = params.sampleRate;
            break;
        }
        case Tag::AUDIO_CHANNELS: {
            if (params.channelCount != rendererParams_.channelCount) {
                MEDIA_LOG_W("channelCount has changed from %" PUBLIC_OUTPUT "u to %" PUBLIC_OUTPUT "u",
                            rendererParams_.channelCount, params.channelCount);
            }
            value = params.channelCount;
            break;
        }
        case Tag::MEDIA_BITRATE: {
            value = bitRate_;
            break;
        }
        case Tag::AUDIO_SAMPLE_FORMAT: {
            if (params.sampleFormat != rendererParams_.sampleFormat) {
                MEDIA_LOG_W("sampleFormat has changed from %" PUBLIC_OUTPUT "u to %" PUBLIC_OUTPUT "u",
                            rendererParams_.sampleFormat, params.sampleFormat);
            }
            value = params.sampleFormat;
            break;
        }
        default:
            MEDIA_LOG_I("Unknown key");
            break;
    }
    return Status::OK;
}

bool AudioServerSinkPlugin::AssignSampleRateIfSupported(uint32_t sampleRate)
{
    AudioStandard::AudioSamplingRate aRate = AudioStandard::SAMPLE_RATE_8000;
    if (!SampleRateNum2Enum(sampleRate, aRate)) {
        MEDIA_LOG_E("sample rate %" PUBLIC_OUTPUT PRIu32 "not supported", sampleRate);
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
            MEDIA_LOG_D("sampleRate: %" PUBLIC_OUTPUT "u", rendererParams_.sampleRate);
            return true;
        }
    }
    return false;
}

bool AudioServerSinkPlugin::AssignChannelNumIfSupported(uint32_t channelNum)
{
    if (channelNum > 2) { // 2
        MEDIA_LOG_E("Unsupported channelNum: %" PUBLIC_OUTPUT PRIu32, channelNum);
        return false;
    }
    AudioStandard::AudioChannel aChannel = AudioStandard::MONO;
    if (!ChannelNumNum2Enum(channelNum, aChannel)) {
        MEDIA_LOG_E("sample rate %" PUBLIC_OUTPUT PRIu32 "not supported", channelNum);
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
            MEDIA_LOG_D("channelCount: %" PUBLIC_OUTPUT "u", rendererParams_.channelCount);
            return true;
        }
    }
    return false;
}

bool AudioServerSinkPlugin::AssignSampleFmtIfSupported(Plugin::AudioSampleFormat sampleFormat)
{
    AudioStandard::AudioSampleFormat aFmt = AudioStandard::AudioSampleFormat::INVALID_WIDTH;
    if (!PluginFmt2SampleFmt(sampleFormat, aFmt)) {
        MEDIA_LOG_E("sample format %" PUBLIC_OUTPUT "hhu not supported", sampleFormat);
        return false;
    }
    auto supportedFormatsList = OHOS::AudioStandard::AudioRenderer::GetSupportedFormats();
    if (supportedFormatsList.empty()) {
        MEDIA_LOG_E("GetSupportedFormats() fail");
        return false;
    }
    for (const auto& fmt : supportedFormatsList) {
        if (fmt == aFmt) {
            rendererParams_.sampleFormat = fmt;
            MEDIA_LOG_D("sampleFormat: %" PUBLIC_OUTPUT "u", rendererParams_.sampleFormat);
            return true;
        }
    }
    return false;
}

Status AudioServerSinkPlugin::SetParameter(Tag tag, const ValueType& value)
{
    MEDIA_LOG_I("SetParameter entered.");
    switch (tag) {
        case Tag::AUDIO_SAMPLE_RATE: {
            if (value.SameTypeWith(typeid(uint32_t))) {
                if (!AssignSampleRateIfSupported(Plugin::AnyCast<uint32_t>(value))) {
                    MEDIA_LOG_E("sampleRate is not supported by audio renderer");
                    return Status::ERROR_INVALID_PARAMETER;
                }
            }
            break;
        }
        case Tag::AUDIO_CHANNELS: {
            if (value.SameTypeWith(typeid(uint32_t))) {
                if (!AssignChannelNumIfSupported(Plugin::AnyCast<uint32_t>(value))) {
                    MEDIA_LOG_E("channelNum is not supported by audio renderer");
                    return Status::ERROR_INVALID_PARAMETER;
                }
            }
            break;
        }
        case Tag::MEDIA_BITRATE: {
            if (value.SameTypeWith(typeid(int64_t))) {
                bitRate_ = Plugin::AnyCast<int64_t>(value);
                MEDIA_LOG_D("bitRate_: %" PUBLIC_OUTPUT PRId64, bitRate_);
            }
            break;
        }
        case Tag::AUDIO_SAMPLE_FORMAT: {
            if (value.SameTypeWith(typeid(AudioSampleFormat))) {
                if (!AssignSampleFmtIfSupported(Plugin::AnyCast<AudioSampleFormat>(value))) {
                    MEDIA_LOG_E("sampleFormat is not supported by audio renderer");
                    return Status::ERROR_INVALID_PARAMETER;
                }
            }
            break;
        }
        default:
            MEDIA_LOG_I("Unknown key");
            break;
    }
    return Status::OK;
}

Status AudioServerSinkPlugin::GetVolume(float& volume)
{
    MEDIA_LOG_I("GetVolume entered.");
    if (audioRenderer_ != nullptr) {
        volume = audioRenderer_->GetVolume();
        return Status::OK;
    } else {
        return Status::ERROR_UNKNOWN;
    }
}

Status AudioServerSinkPlugin::SetVolume(float volume)
{
    MEDIA_LOG_I("SetVolume entered.");
    if (audioRenderer_ != nullptr) {
        int32_t ret = audioRenderer_->SetVolume(volume);
        return ret == OHOS::AudioStandard::SUCCESS ? Status::OK : Status::ERROR_UNKNOWN;
    } else {
        return Status::ERROR_UNKNOWN;
    }
}

Status AudioServerSinkPlugin::Pause()
{
    MEDIA_LOG_I("Pause entered.");
    if (audioRenderer_ != nullptr) {
        if (audioRenderer_->Pause()) {
            MEDIA_LOG_I("audioRenderer_ Pause() success");
            return Status::OK;
        } else {
            MEDIA_LOG_E("audioRenderer_ Pause() fail");
        }
    }
    return Status::ERROR_UNKNOWN;
}

Status AudioServerSinkPlugin::GetLatency(uint64_t& nanoSec)
{
    MEDIA_LOG_I("GetLatency entered.");
    if (audioRenderer_ != nullptr) {
        uint64_t microSec = 0;
        int32_t ret = audioRenderer_->GetLatency(microSec);
        nanoSec = microSec * TIME_MS_TO_NS;
        return ret == AudioStandard::SUCCESS ? Status::OK : Status::ERROR_UNKNOWN;
    } else {
        return Status::ERROR_UNKNOWN;
    }
}

Status AudioServerSinkPlugin::Write(const std::shared_ptr<Buffer>& input)
{
    MEDIA_LOG_I("Write entered.");
    if (audioRenderer_ == nullptr) {
        MEDIA_LOG_I("audioRenderer_ invalid.");
        return Status::ERROR_UNKNOWN;
    }
    auto mem = input->GetMemory();
    int32_t ret = audioRenderer_->Write(const_cast<uint8_t *>(mem->GetReadOnlyData()), mem->GetSize());
    return (ret > 0) ? Status::ERROR_UNKNOWN : Status::OK;
}

Status AudioServerSinkPlugin::Flush()
{
    MEDIA_LOG_I("Flush entered.");
    if (audioRenderer_ != nullptr) {
        if (audioRenderer_->Flush()) {
            MEDIA_LOG_I("audioRenderer_ Flush() success");
            return Status::OK;
        } else {
            MEDIA_LOG_E("audioRenderer_ Flush() fail");
        }
    }
    return Status::ERROR_UNKNOWN;
}
} // namespace AuSrSinkPlugin
} // namespace Media
} // namespace OHOS