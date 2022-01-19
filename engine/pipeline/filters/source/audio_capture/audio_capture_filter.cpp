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

#define HST_LOG_TAG "AudioCaptureFilter"

#include "audio_capture_filter.h"
#include "foundation/log.h"
#include "factory/filter_factory.h"
#include "common/plugin_utils.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
using namespace Plugin;

static AutoRegisterFilter<AudioCaptureFilter> g_registerFilterHelper("builtin.recorder.audiocapture");

AudioCaptureFilter::AudioCaptureFilter(const std::string& name)
    : FilterBase(name),
      taskPtr_(nullptr),
      plugin_(nullptr),
      pluginAllocator_(nullptr),
      pluginInfo_(nullptr)
{
    filterType_ = FilterType::CAPTURE_SOURCE;
    MEDIA_LOG_D("ctor called");
}

AudioCaptureFilter::~AudioCaptureFilter()
{
    MEDIA_LOG_D("dtor called");
    if (taskPtr_) {
        taskPtr_->Stop();
    }
    if (plugin_) {
        plugin_->Deinit();
    }
}

std::vector<WorkMode> AudioCaptureFilter::GetWorkModes()
{
    return {WorkMode::PUSH};
}

ErrorCode AudioCaptureFilter::InitAndConfigPlugin(const std::shared_ptr<Plugin::Meta>& audioMeta)
{
    MEDIA_LOG_D("IN");
    ErrorCode err = TranslatePluginStatus(plugin_->Init());
    if (err != ErrorCode::SUCCESS) {
        return err;
    }
    plugin_->SetCallback(this);
    pluginAllocator_ = plugin_->GetAllocator();
    uint32_t tmp = 0;
    if (audioMeta->GetUint32(MetaID::AUDIO_SAMPLE_RATE, tmp)) {
        MEDIA_LOG_I("configure plugin with sample rate %" PRIu32, tmp);
        err = TranslatePluginStatus(plugin_->SetParameter(Tag::AUDIO_SAMPLE_RATE, tmp));
        if (err != ErrorCode::SUCCESS) {
            return err;
        }
    }
    if (audioMeta->GetUint32(MetaID::AUDIO_CHANNELS, tmp)) {
        MEDIA_LOG_I("configure plugin with channel %" PRIu32, tmp);
        err = TranslatePluginStatus(plugin_->SetParameter(Tag::AUDIO_CHANNELS, channelNum_));
        if (err != ErrorCode::SUCCESS) {
            return err;
        }
    }
    int64_t bitRate = 0;
    if (audioMeta->GetInt64(MetaID::AUDIO_CHANNELS, bitRate)) {
        MEDIA_LOG_I("configure plugin with channel %" PRId64, bitRate);
        err = TranslatePluginStatus(plugin_->SetParameter(Tag::MEDIA_BITRATE, bitRate));
        if (err != ErrorCode::SUCCESS) {
            return err;
        }
    }
    Plugin::AudioSampleFormat sampleFormat = Plugin::AudioSampleFormat::S16;
    if (audioMeta->GetData<Plugin::AudioSampleFormat>(MetaID::AUDIO_SAMPLE_FORMAT, sampleFormat)) {
        MEDIA_LOG_I("configure plugin with sampleFormat %" PRIu8, sampleFormat);
        return TranslatePluginStatus(plugin_->SetParameter(Tag::AUDIO_SAMPLE_FORMAT, sampleFormat));
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AudioCaptureFilter::SetParameter(int32_t key, const Plugin::Any& value)
{
    auto tag = static_cast<OHOS::Media::Plugin::Tag>(key);
    switch (tag) {
        case Tag::SRC_INPUT_TYPE: {
            if (value.Type() == typeid(Plugin::SrcInputType)) {
                inputType_ = Plugin::AnyCast<Plugin::SrcInputType>(value);
                inputTypeSpecified_ = true;
                MEDIA_LOG_D("inputType_: %s", inputType_.c_str());
            }
            break;
        }
        case OHOS::Media::Plugin::Tag::AUDIO_SAMPLE_RATE: {
            if (value.Type() == typeid(uint32_t)) {
                sampleRate_ = Plugin::AnyCast<uint32_t>(value);
                sampleRateSpecified_ = true;
                MEDIA_LOG_D("sampleRate_: %" PRIu32, sampleRate_);
            }
            break;
        }
        case OHOS::Media::Plugin::Tag::AUDIO_CHANNELS: {
            if (value.Type() == typeid(uint32_t)) {
                channelNum_ = Plugin::AnyCast<uint32_t>(value);
                channelNumSpecified_ = true;
                MEDIA_LOG_D("channelNum_: %" PRIu32, channelNum_);
            }
            break;
        }
        case OHOS::Media::Plugin::Tag::MEDIA_BITRATE: {
            if (value.Type() == typeid(int64_t)) {
                bitRate_ = Plugin::AnyCast<int64_t>(value);
                bitRateSpecified_ = true;
                MEDIA_LOG_D("bitRate_: %" PRId64, bitRate_);
            }
            break;
        }
        case OHOS::Media::Plugin::Tag::AUDIO_SAMPLE_FORMAT: {
            if (value.Type() == typeid(OHOS::Media::Plugin::AudioSampleFormat)) {
                sampleFormat_ = Plugin::AnyCast<OHOS::Media::Plugin::AudioSampleFormat>(value);
                sampleRateSpecified_ = true;
                MEDIA_LOG_D("sampleFormat_: %u", sampleFormat_);
            }
            break;
        }
        default:
            MEDIA_LOG_W("Unknown key %d", OHOS::Media::to_underlying(tag));
            break;
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AudioCaptureFilter::GetParameter(int32_t key, Plugin::Any& value)
{
    Tag tag = static_cast<Plugin::Tag>(key);
    switch (tag) {
        case Tag::SRC_INPUT_TYPE: {
            value = inputType_;
            break;
        }
        case Tag::AUDIO_SAMPLE_RATE: {
            value = sampleRate_;
            break;
        }
        case OHOS::Media::Plugin::Tag::AUDIO_CHANNELS: {
            value = channelNum_;
            break;
        }
        case OHOS::Media::Plugin::Tag::MEDIA_BITRATE: {
            value = bitRate_;
            break;
        }
        case OHOS::Media::Plugin::Tag::AUDIO_SAMPLE_FORMAT: {
            value = sampleFormat_;
            break;
        }
        default:
            MEDIA_LOG_I("Unknown key %d", tag);
            break;
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AudioCaptureFilter::DoConfigure()
{
    auto emptyMeta = std::make_shared<Plugin::Meta>();
    auto audioMeta = std::make_shared<Plugin::Meta>();
    if (!MergeMetaWithCapability(*emptyMeta, capNegWithDownstream_, *audioMeta)) {
        MEDIA_LOG_E("cannot find available capability of plugin %s", pluginInfo_->name.c_str());
        return ErrorCode::ERROR_UNKNOWN;
    }
    if (!outPorts_[0]->Configure(audioMeta)) {
        MEDIA_LOG_E("Configure downstream fail");
        return ErrorCode::ERROR_UNKNOWN;
    }
    return InitAndConfigPlugin(audioMeta);
}

ErrorCode AudioCaptureFilter::Prepare()
{
    MEDIA_LOG_I("Prepare entered.");
    if (!taskPtr_) {
        taskPtr_ = std::make_shared<OSAL::Task>("DataReader");
        taskPtr_->RegisterHandler(std::bind(&AudioCaptureFilter::ReadLoop, this));
    }
    ErrorCode err = FindPlugin();
    if (err != ErrorCode::SUCCESS || !plugin_) {
        MEDIA_LOG_E("Find plugin fail");
        return err;
    }
    err = DoConfigure();
    if (err != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("DoConfigure fail");
        return err;
    }
    err = TranslatePluginStatus(plugin_->Prepare());
    if (err == ErrorCode::SUCCESS) {
        MEDIA_LOG_D("media source send EVENT_READY");
        OnEvent(Event{EVENT_READY, {}});
    }
    return err;
}

ErrorCode AudioCaptureFilter::Start()
{
    MEDIA_LOG_I("Start entered.");
    if (taskPtr_) {
        taskPtr_->Start();
    }
    return plugin_ ? TranslatePluginStatus(plugin_->Start()) : ErrorCode::ERROR_INVALID_OPERATION;
}

ErrorCode AudioCaptureFilter::Stop()
{
    MEDIA_LOG_I("Stop entered.");
    if (taskPtr_) {
        taskPtr_->Stop();
    }
    ErrorCode ret = ErrorCode::SUCCESS;
    if (plugin_) {
        ret = TranslatePluginStatus(plugin_->Stop());
    }
    return ret;
}

ErrorCode AudioCaptureFilter::Pause()
{
    MEDIA_LOG_I("Pause entered.");
    if (taskPtr_) {
        taskPtr_->PauseAsync();
    }
    ErrorCode ret = ErrorCode::SUCCESS;
    if (plugin_) {
        ret = TranslatePluginStatus(plugin_->Stop());
    }
    return ret;
}

ErrorCode AudioCaptureFilter::Resume()
{
    MEDIA_LOG_I("Resume entered.");
    if (taskPtr_) {
        taskPtr_->Start();
    }
    return plugin_ ? TranslatePluginStatus(plugin_->Start()) : ErrorCode::ERROR_INVALID_OPERATION;
}

ErrorCode AudioCaptureFilter::SendEos()
{
    MEDIA_LOG_I("SendEos entered.");
    return ErrorCode::SUCCESS;
}

void AudioCaptureFilter::InitPorts()
{
    MEDIA_LOG_D("IN");
    auto outPort = std::make_shared<OutPort>(this);
    outPorts_.push_back(outPort);
}

void AudioCaptureFilter::ReadLoop()
{
    size_t bufferSize = 0;
    auto ret = plugin_->GetSize(bufferSize);
    if (ret != Status::OK || bufferSize <= 0) {
        MEDIA_LOG_E("Get plugin buffer size fail");
        return;
    }
    AVBufferPtr bufferPtr = std::make_shared<AVBuffer>(BufferMetaType::AUDIO);
    ret = plugin_->Read(bufferPtr, bufferSize);
    if (ret != Status::OK) {
        Stop();
        return;
    }
    outPorts_[0]->PushData(bufferPtr, -1);
}

ErrorCode AudioCaptureFilter::CreatePlugin(const std::shared_ptr<PluginInfo>& info, const std::string& name,
                                           PluginManager& manager)
{
    if ((plugin_ != nullptr) && (pluginInfo_ != nullptr)) {
        if (info->name == pluginInfo_->name && TranslatePluginStatus(plugin_->Reset()) == ErrorCode::SUCCESS) {
            MEDIA_LOG_I("Reuse last plugin: %s", name.c_str());
            return ErrorCode::SUCCESS;
        }
        if (TranslatePluginStatus(plugin_->Deinit()) != ErrorCode::SUCCESS) {
            MEDIA_LOG_E("Deinit last plugin: %s error", pluginInfo_->name.c_str());
        }
    }
    plugin_ = manager.CreateSourcePlugin(name);
    if (plugin_ == nullptr) {
        MEDIA_LOG_E("PluginManager CreatePlugin %s fail", name.c_str());
        return ErrorCode::ERROR_UNKNOWN;
    }
    pluginInfo_ = info;
    MEDIA_LOG_I("Create new plugin: \"%s\" success", pluginInfo_->name.c_str());
    return ErrorCode::SUCCESS;
}

bool AudioCaptureFilter::CheckSampleRate(const Plugin::Capability& cap)
{
    if (!sampleRateSpecified_) {
        return true;
    }
    for (const auto& pairKey : cap.keys) {
        if (pairKey.first != Capability::Key::AUDIO_SAMPLE_RATE ||
            pairKey.second.Type() != typeid(DiscreteCapability<uint32_t>)) {
            continue;
        }
        auto supportedSampleRateList = Plugin::AnyCast<DiscreteCapability<uint32_t>>(pairKey.second);
        for (const auto& rate : supportedSampleRateList) {
            if (rate == sampleRate_) {
                return true;
            }
        }
    }
    return false;
}

bool AudioCaptureFilter::CheckChannels(const Plugin::Capability& cap)
{
    if (!channelNumSpecified_) {
        return true;
    }
    for (const auto& pairKey : cap.keys) {
        if (pairKey.first != Capability::Key::AUDIO_CHANNELS ||
            pairKey.second.Type() != typeid(DiscreteCapability<uint32_t>)) {
            continue;
        }
        auto supportedChannelsList = Plugin::AnyCast<DiscreteCapability<uint32_t>>(pairKey.second);
        for (const auto& channel : supportedChannelsList) {
            if (channel == channelNum_) {
                return true;
            }
        }
    }
    return false;
}

bool AudioCaptureFilter::CheckSampleFormat(const Plugin::Capability& cap)
{
    if (!channelNumSpecified_) {
        return true;
    }
    for (const auto& pairKey : cap.keys) {
        if (pairKey.first != Capability::Key::AUDIO_SAMPLE_FORMAT ||
            pairKey.second.Type() != typeid(DiscreteCapability<Plugin::AudioSampleFormat>)) {
            continue;
        }
        auto supportedSampleFormatList =
            Plugin::AnyCast<DiscreteCapability<Plugin::AudioSampleFormat>>(pairKey.second);
        for (const auto& fmt : supportedSampleFormatList) {
            if (fmt == sampleFormat_) {
                return true;
            }
        }
    }
    return false;
}

bool AudioCaptureFilter::DoNegotiate(const CapabilitySet &outCaps)
{
    if (outCaps.empty()) {
        MEDIA_LOG_E("audio capture plugin must have out caps");
        return false;
    }
    for (const auto& outCap : outCaps) {
        if (!CheckSampleRate(outCap) || !CheckChannels(outCap) || !CheckSampleFormat(outCap)) {
            continue;
        }
        auto thisOut = std::make_shared<Plugin::Capability>();
        *thisOut = outCap;
        if (sampleFormatSpecified_) {
            thisOut->keys[Capability::Key::AUDIO_SAMPLE_FORMAT] = sampleFormat_;
        }
        if (sampleRateSpecified_) {
            thisOut->keys[Capability::Key::AUDIO_SAMPLE_RATE] = sampleRate_;
        }
        if (channelNumSpecified_) {
            thisOut->keys[Capability::Key::AUDIO_CHANNELS] = channelNum_;
        }
        if (bitRateSpecified_) {
            thisOut->keys[Capability::Key::MEDIA_BITRATE] = bitRate_;
        }
        if (outPorts_[0]->Negotiate(thisOut, capNegWithDownstream_)) {
            MEDIA_LOG_I("Negotiate success");
            return true;
        }
    }
    return false;
}

ErrorCode AudioCaptureFilter::FindPlugin()
{
    if (!inputTypeSpecified_) {
        MEDIA_LOG_E("Must set input type first");
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    PluginManager& pluginManager = PluginManager::Instance();
    std::set<std::string> nameList = pluginManager.ListPlugins(PluginType::SOURCE);
    for (const std::string& name : nameList) {
        std::shared_ptr<PluginInfo> info = pluginManager.GetPluginInfo(PluginType::SOURCE, name);
        MEDIA_LOG_I("name: %s, info->name: %s", name.c_str(), info->name.c_str());
        auto val = info->extra[PLUGIN_INFO_EXTRA_INPUT_TYPE];
        if (val.Type() == typeid(Plugin::SrcInputType)) {
            auto supportInputType = OHOS::Media::Plugin::AnyCast<Plugin::SrcInputType>(val);
            if (inputType_ == supportInputType && DoNegotiate(info->outCaps) &&
                CreatePlugin(info, name, pluginManager) == ErrorCode::SUCCESS) {
                MEDIA_LOG_I("CreatePlugin %s success", name_.c_str());
                return ErrorCode::SUCCESS;
            }
        }
    }
    MEDIA_LOG_I("Cannot find any plugin");
    return ErrorCode::ERROR_UNSUPPORTED_FORMAT;
}
} // namespace Pipeline
} // namespace Media
} // namespace OHOS
#endif