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
#define HST_LOG_TAG "AudioCapturePlugin"

#include "audio_capture_plugin.h"
#include <algorithm>
#include <cmath>
#include "audio_type_translate.h"
#include "foundation/log.h"
#include "plugin/common/plugin_time.h"
#include "utils/utils.h"
#include "utils/constants.h"

namespace {
// register plugins
using namespace OHOS::Media;

#define MAX_CAPTURE_BUFFER_SIZE 100000
#define TIME_SEC_TO_NS  1000000000

std::shared_ptr<Plugin::SourcePlugin> AudioCapturePluginCreater(const std::string &name)
{
    return std::make_shared<OHOS::Media::AuCapturePlugin::AudioCapturePlugin>(name);
}

void UpdateSupportedSampleRate(Plugin::Capability &outCaps)
{
    auto supportedSampleRateList = OHOS::AudioStandard::AudioCapturer::GetSupportedSamplingRates();
    if (!supportedSampleRateList.empty()) {
        Plugin::DiscreteCapability<uint32_t> values;
        for (const auto& rate : supportedSampleRateList) {
            uint32_t sampleRate = 0;
            if (AuCapturePlugin::SampleRateEnum2Num(rate, sampleRate)) {
                values.push_back(sampleRate);
            }
        }
        if (!values.empty()) {
            outCaps.AppendDiscreteKeys<uint32_t>(Plugin::Capability::Key::AUDIO_SAMPLE_RATE, values);
        }
    }
}

void UpdateSupportedChannels(Plugin::Capability &outCaps)
{
    auto supportedChannelsList = OHOS::AudioStandard::AudioCapturer::GetSupportedChannels();
    if (!supportedChannelsList.empty()) {
        Plugin::DiscreteCapability<uint32_t> values;
        for (const auto& channel : supportedChannelsList) {
            uint32_t channelNum = 0;
            if (AuCapturePlugin::ChannelNumEnum2Num(channel, channelNum)) {
                values.push_back(channelNum);
            }
        }
        if (!values.empty()) {
            outCaps.AppendDiscreteKeys<uint32_t>(Plugin::Capability::Key::AUDIO_CHANNELS, values);
        }
    }
}

void UpdateSupportedSampleFormat(Plugin::Capability &outCaps)
{
    auto supportedFormatsList = OHOS::AudioStandard::AudioCapturer::GetSupportedFormats();
    if (!supportedFormatsList.empty()) {
        Plugin::DiscreteCapability<Plugin::AudioSampleFormat> values;
        for (const auto& fmt : supportedFormatsList) {
            auto pFmt = Plugin::AudioSampleFormat::U8;
            if (AuCapturePlugin::SampleFmt2PluginFmt(fmt, pFmt)) {
                values.emplace_back(pFmt);
            }
        }
        if (!values.empty()) {
            outCaps.AppendDiscreteKeys<Plugin::AudioSampleFormat>(Plugin::Capability::Key::AUDIO_SAMPLE_FORMAT,
                                                                 values);
        }
    }
}

Plugin::Status AudioCaptureRegister(const std::shared_ptr<Plugin::Register> &reg)
{
    Plugin::SourcePluginDef definition;
    definition.name = "AudioCapture";
    definition.description = "Audio capture from audio service";
    definition.rank = 100; // 100: max rank
    definition.inputType = Plugin::SrcInputType::AUD_MIC;
    definition.creator = AudioCapturePluginCreater;
    Plugin::Capability outCaps(OHOS::Media::MEDIA_MIME_AUDIO_RAW);
    UpdateSupportedSampleRate(outCaps);
    UpdateSupportedChannels(outCaps);
    UpdateSupportedSampleFormat(outCaps);
    definition.outCaps.push_back(outCaps);
    // add es outCaps later
    return reg->AddPlugin(definition);
}
PLUGIN_DEFINITION(AudioCapture, Plugin::LicenseType::APACHE_V2, AudioCaptureRegister, [] {});
}

namespace OHOS {
namespace Media {
namespace AuCapturePlugin {
using namespace OHOS::Media::Plugin;

void* AudioCaptureAllocator::Alloc(size_t size)
{
    if (size == 0) {
        return nullptr;
    }
    return reinterpret_cast<void*>(new (std::nothrow) uint8_t[size]); // NOLINT: cast
}

void AudioCaptureAllocator::Free(void* ptr) // NOLINT: void*
{
    if (ptr != nullptr) {
        delete[](uint8_t*) ptr;
    }
}

AudioCapturePlugin::AudioCapturePlugin(std::string name) : SourcePlugin(std::move(name))
{
    MEDIA_LOG_D("IN");
}

AudioCapturePlugin::~AudioCapturePlugin()
{
    MEDIA_LOG_D("IN");
}

Status AudioCapturePlugin::Init()
{
    MEDIA_LOG_D("IN");
    mAllocator_ = std::make_shared<AudioCaptureAllocator>();
    if (audioCapturer_ == nullptr) {
        audioCapturer_ = AudioStandard::AudioCapturer::Create(AudioStandard::AudioStreamType::STREAM_MUSIC);
        if (audioCapturer_ == nullptr) {
            MEDIA_LOG_E("Create audioCapturer fail");
            return Status::ERROR_UNKNOWN;
        }
    }
    return Status::OK;
}

Status AudioCapturePlugin::Deinit()
{
    MEDIA_LOG_D("IN");
    if (audioCapturer_) {
        if (audioCapturer_->GetStatus() == AudioStandard::CapturerState::CAPTURER_RUNNING) {
            if (!audioCapturer_->Stop()) {
                MEDIA_LOG_E("Stop audioCapturer fail");
            }
        }
        if (!audioCapturer_->Release()) {
            MEDIA_LOG_E("Release audioCapturer fail");
        }
        audioCapturer_ = nullptr;
    }
    return Status::OK;
}

Status AudioCapturePlugin::Prepare()
{
    MEDIA_LOG_D("IN");
    AudioStandard::AudioCapturerParams params;
    params.audioEncoding = AudioStandard::ENCODING_INVALID;
    auto supportedEncodingTypes = OHOS::AudioStandard::AudioCapturer::GetSupportedEncodingTypes();
    for (auto& supportedEncodingType : supportedEncodingTypes) {
        if (supportedEncodingType == AudioStandard::ENCODING_PCM) {
            params.audioEncoding = AudioStandard::ENCODING_PCM;
            break;
        }
    }
    if (params.audioEncoding != AudioStandard::ENCODING_PCM) {
        MEDIA_LOG_E("audioCapturer do not support pcm encoding");
        return Status::ERROR_UNKNOWN;
    }
    if (audioCapturer_->SetParams(params)) {
        MEDIA_LOG_E("audioCapturer SetParams() fail");
        return Status::ERROR_UNKNOWN;
    }
    size_t size;
    if (audioCapturer_->GetBufferSize(size)) {
        MEDIA_LOG_E("audioCapturer GetBufferSize() fail");
        return Status::ERROR_UNKNOWN;
    }
    if (size >= MAX_CAPTURE_BUFFER_SIZE) {
        MEDIA_LOG_E("bufferSize is too big: %zu", size);
        return Status::ERROR_INVALID_PARAMETER;
    }
    bufferSize_ = size;
    MEDIA_LOG_D("bufferSize is: %zu", bufferSize_);
    return Status::OK;
}

Status AudioCapturePlugin::Reset()
{
    MEDIA_LOG_D("IN");
    if (audioCapturer_->GetStatus() == AudioStandard::CapturerState::CAPTURER_RUNNING) {
        if (!audioCapturer_->Stop()) {
            MEDIA_LOG_E("Stop audioCapturer fail");
            return Status::ERROR_UNKNOWN;
        }
    }
    bufferSize_ = 0;
    curTimestampNs_ = 0;
    stopTimestampNs_ = 0;
    totalPauseTimeNs_ = 0;
    bitRate_ = 0;
    isStop_ = false;
    return Status::OK;
}

Status AudioCapturePlugin::Start()
{
    MEDIA_LOG_D("IN");
    if (audioCapturer_->GetStatus() != AudioStandard::CapturerState::CAPTURER_PREPARED) {
        MEDIA_LOG_E("audioCapturer need to prepare first");
        return Status::ERROR_WRONG_STATE;
    }
    if (!audioCapturer_->Start()) {
        MEDIA_LOG_E("Start audioCapturer fail");
        return Status::ERROR_UNKNOWN;
    }
    if (isStop_) {
        if (GetAudioTime(curTimestampNs_) != Status::OK) {
            MEDIA_LOG_E("Get auido time fail");
        }
        if (curTimestampNs_ < stopTimestampNs_) {
            MEDIA_LOG_E("Get wrong audio time");
        }
        totalPauseTimeNs_ += std::fabs(curTimestampNs_ - stopTimestampNs_);
        isStop_ = false;
    }
    return Status::OK;
}

Status AudioCapturePlugin::Stop()
{
    MEDIA_LOG_D("IN");
    enum AudioStandard::CapturerState state = audioCapturer_->GetStatus();
    if (state != AudioStandard::CapturerState::CAPTURER_RUNNING) {
        MEDIA_LOG_E("audioCapturer need to prepare first");
        return Status::ERROR_WRONG_STATE;
    }
    if (!audioCapturer_->Stop()) {
        MEDIA_LOG_E("Stop audioCapturer fail");
        return Status::ERROR_UNKNOWN;
    }
    if (!isStop_) {
        stopTimestampNs_ = curTimestampNs_;
        isStop_ = true;
    }
    return Status::OK;
}

bool AudioCapturePlugin::IsParameterSupported(Tag tag)
{
    MEDIA_LOG_D("IN");
    return false;
}

Status AudioCapturePlugin::GetParameter(Tag tag, ValueType& value)
{
    MEDIA_LOG_D("IN");
    AudioStandard::AudioCapturerParams params;
    if (!audioCapturer_ || !audioCapturer_->GetParams(params)) {
        MEDIA_LOG_E("audioCapturer GetParams() fail");
        return Status::ERROR_UNKNOWN;
    }
    switch (tag) {
        case Tag::AUDIO_SAMPLE_RATE: {
            if (params.samplingRate != capturerParams_.samplingRate) {
                MEDIA_LOG_W("samplingRate has changed from %u to %u",
                            capturerParams_.samplingRate, params.samplingRate);
            }
            value = params.samplingRate;
            break;
        }
        case Tag::AUDIO_CHANNELS: {
            if (params.audioChannel != capturerParams_.audioChannel) {
                MEDIA_LOG_W("audioChannel has changed from %u to %u",
                            capturerParams_.audioChannel, params.audioChannel);
            }
            value = params.audioChannel;
            break;
        }
        case Tag::MEDIA_BITRATE: {
            value = bitRate_;
            break;
        }
        case Tag::AUDIO_SAMPLE_FORMAT: {
            if (params.audioSampleFormat != capturerParams_.audioSampleFormat) {
                MEDIA_LOG_W("audioSampleFormat has changed from %u to %u",
                            capturerParams_.audioSampleFormat, params.audioSampleFormat);
            }
            value = params.audioSampleFormat;
            break;
        }
        default:
            MEDIA_LOG_I("Unknown key");
            break;
    }
    return Status::OK;
}

bool AudioCapturePlugin::AssignSampleRateIfSupported(uint32_t sampleRate)
{
    AudioStandard::AudioSamplingRate aRate = AudioStandard::SAMPLE_RATE_8000;
    if (!AuCapturePlugin::SampleRateNum2Enum(sampleRate, aRate)) {
        MEDIA_LOG_E("sample rate %" PRIu32 "not supported", sampleRate);
        return false;
    }
    auto supportedSampleRateList = OHOS::AudioStandard::AudioCapturer::GetSupportedSamplingRates();
    if (supportedSampleRateList.empty()) {
        MEDIA_LOG_E("GetSupportedSamplingRates() fail");
        return false;
    }
    return std::any_of(supportedSampleRateList.begin(), supportedSampleRateList.end(),
        [aRate, this] (const auto& rate) {
        if (rate == aRate) {
            capturerParams_.samplingRate = rate;
        }
        return rate == aRate;
    });
}

bool AudioCapturePlugin::AssignChannelNumIfSupported(uint32_t channelNum)
{
    if (channelNum > 2) { // 2
        MEDIA_LOG_E("Unsupported channelNum: %" PRIu32, channelNum);
        return false;
    }
    AudioStandard::AudioChannel aChannel = AudioStandard::MONO;
    if (!AuCapturePlugin::ChannelNumNum2Enum(channelNum, aChannel)) {
        MEDIA_LOG_E("sample rate %" PRIu32 "not supported", channelNum);
        return false;
    }
    auto supportedChannelsList = OHOS::AudioStandard::AudioCapturer::GetSupportedChannels();
    if (supportedChannelsList.empty()) {
        MEDIA_LOG_E("GetSupportedChannels() fail");
        return false;
    }
    return std::any_of(supportedChannelsList.begin(), supportedChannelsList.end(),
        [aChannel, this] (const auto& channel) {
        if (channel == aChannel) {
            capturerParams_.audioChannel = channel;
        }
        return channel == aChannel;
    });
}

bool AudioCapturePlugin::AssignSampleFmtIfSupported(Plugin::AudioSampleFormat sampleFormat)
{
    AudioStandard::AudioSampleFormat aFmt = AudioStandard::AudioSampleFormat::INVALID_WIDTH;
    if (!AuCapturePlugin::PluginFmt2SampleFmt(sampleFormat, aFmt)) {
        MEDIA_LOG_E("sample format %" PRIu8 "not supported", sampleFormat);
        return false;
    }
    auto supportedFormatsList = OHOS::AudioStandard::AudioCapturer::GetSupportedFormats();
    if (supportedFormatsList.empty()) {
        MEDIA_LOG_E("GetSupportedFormats() fail");
        return false;
    }
    return std::any_of(supportedFormatsList.begin(), supportedFormatsList.end(),
        [aFmt, this] (const auto& fmt) {
        if (aFmt == fmt) {
            capturerParams_.audioSampleFormat = fmt;
            MEDIA_LOG_D("audioSampleFormat: %u", capturerParams_.audioSampleFormat);
        }
        return aFmt == fmt;
    });
}

Status AudioCapturePlugin::SetParameter(Tag tag, const ValueType& value)
{
    switch (tag) {
        case Tag::AUDIO_SAMPLE_RATE: {
            if (value.Type() == typeid(uint32_t)) {
                if (!AssignSampleRateIfSupported(Plugin::AnyCast<uint32_t>(value))) {
                    MEDIA_LOG_E("sampleRate is not supported by audiocapturer");
                    return Status::ERROR_INVALID_PARAMETER;
                }
            }
            break;
        }
        case Tag::AUDIO_CHANNELS: {
            if (value.Type() == typeid(uint32_t)) {
                if (!AssignChannelNumIfSupported(Plugin::AnyCast<uint32_t>(value))) {
                    MEDIA_LOG_E("channelNum is not supported by audiocapturer");
                    return Status::ERROR_INVALID_PARAMETER;
                }
            }
            break;
        }
        case Tag::MEDIA_BITRATE: {
            if (value.Type() == typeid(int64_t)) {
                bitRate_ = Plugin::AnyCast<int64_t>(value);
                MEDIA_LOG_D("bitRate_: %u", bitRate_);
            }
            break;
        }
        case Tag::AUDIO_SAMPLE_FORMAT: {
            if (value.Type() == typeid(AudioSampleFormat)) {
                if (!AssignSampleFmtIfSupported(Plugin::AnyCast<AudioSampleFormat>(value))) {
                    MEDIA_LOG_E("sampleFormat is not supported by audiocapturer");
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

std::shared_ptr<Allocator> AudioCapturePlugin::GetAllocator()
{
    MEDIA_LOG_D("IN");
    return mAllocator_;
}

Status AudioCapturePlugin::SetCallback(Callback* cb)
{
    MEDIA_LOG_D("IN");
    UNUSED_VARIABLE(cb);
    return Status::ERROR_UNIMPLEMENTED;
}

Status AudioCapturePlugin::SetSource(std::shared_ptr<MediaSource> source)
{
    UNUSED_VARIABLE(source);
    return Status::ERROR_UNIMPLEMENTED;
}

Status AudioCapturePlugin::GetAudioTime(uint64_t &audioTimeNs)
{
    if (!audioCapturer_) {
        return Status::ERROR_WRONG_STATE;
    }
    OHOS::AudioStandard::Timestamp timeStamp;
    auto timeBase = OHOS::AudioStandard::Timestamp::Timestampbase::MONOTONIC;
    if (!audioCapturer_->GetAudioTime(timeStamp, timeBase)) {
        MEDIA_LOG_E("audioCapturer GetAudioTime() fail");
        return Status::ERROR_UNKNOWN;
    }
    if (timeStamp.time.tv_sec < 0 || timeStamp.time.tv_nsec < 0) {
        return Status::ERROR_INVALID_PARAMETER;
    }
    if ((UINT64_MAX - timeStamp.time.tv_nsec) / TIME_SEC_TO_NS > timeStamp.time.tv_sec) {
        return Status::ERROR_INVALID_PARAMETER;
    }
    audioTimeNs = timeStamp.time.tv_sec * TIME_SEC_TO_NS + timeStamp.time.tv_nsec;
    return Status::OK;
}

Status AudioCapturePlugin::Read(std::shared_ptr<Buffer>& buffer, size_t expectedLen)
{
    auto bufferMeta = buffer->GetBufferMeta();
    if (!bufferMeta || bufferMeta->GetType() != BufferMetaType::AUDIO) {
        return Status::ERROR_INVALID_PARAMETER;
    }
    std::shared_ptr<Memory> bufData;
    if (buffer->IsEmpty()) {
        bufData = buffer->AllocMemory(GetAllocator(), expectedLen);
    } else {
        bufData = buffer->GetMemory();
    }
    if (bufData->GetSize() <= 0) {
        return Status::ERROR_NO_MEMORY;
    }
    bool isBlocking = true;
    auto size = audioCapturer_->Read(bufData->GetWritableAddr(expectedLen), expectedLen, isBlocking);
    if (size <= 0) {
        MEDIA_LOG_D("audioCapturer Read() fail");
        return Status::ERROR_NOT_ENOUGH_DATA;
    }
    auto ret = GetAudioTime(curTimestampNs_);
    if (ret != Status::OK) {
        MEDIA_LOG_E("Get audio timestamp fail");
        return ret;
    }
    Ns2HstTime(curTimestampNs_ + totalPauseTimeNs_, reinterpret_cast<int64_t &>(buffer->pts));
    bufferSize_ = size;
    return ret;
}

Status AudioCapturePlugin::GetSize(size_t& size)
{
    if (bufferSize_ == 0) {
        return Status::ERROR_INVALID_PARAMETER;
    }
    size = bufferSize_;
    MEDIA_LOG_D("bufferSize_: %zu", size);
    return Status::OK;
}

bool AudioCapturePlugin::IsSeekable()
{
    return false;
}

Status AudioCapturePlugin::SeekTo(uint64_t offset)
{
    UNUSED_VARIABLE(offset);
    return Status::ERROR_UNIMPLEMENTED;
}
} // namespace Plugin
} // namespace Media
} // namespace OHOS
