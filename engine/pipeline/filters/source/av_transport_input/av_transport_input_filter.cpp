/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#define HST_LOG_TAG "AVInputFilter"
#include "pipeline/filters/source/av_transport_input/av_transport_input_filter.h"
#include "pipeline/filters/common/plugin_utils.h"
#include "foundation/log.h"
#include "pipeline/factory/filter_factory.h"
#include "plugin/common/plugin_attr_desc.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
using namespace Plugin;

static AutoRegisterFilter<AVInputFilter> g_registerFilterHelper("builtin.avtransport.avinput");

AVInputFilter::AVInputFilter(const std::string& name) : FilterBase(name), plugin_(nullptr), pluginInfo_(nullptr)
{
    MEDIA_LOG_I("ctor called");
    filterType_ = FilterType::AV_INPUT;
}

AVInputFilter::~AVInputFilter()
{
    MEDIA_LOG_I("dtor called");
    OSAL::ScopedLock lock(inputFilterMutex_);
    if (plugin_ != nullptr) {
        plugin_->Deinit();
    }
}

std::vector<WorkMode> AVInputFilter::GetWorkModes()
{
    return {WorkMode::PUSH};
}

ErrorCode AVInputFilter::SetParameter(int32_t key, const Any& value)
{
    Tag tag;
    if (!TranslateIntoParameter(key, tag)) {
        MEDIA_LOG_E("This key is invalid!");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    {
        OSAL::ScopedLock lock(inputFilterMutex_);
        paramsMap_[tag] = value;
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AVInputFilter::GetParameter(int32_t key, Any& value)
{
    Tag tag;
    if (!TranslateIntoParameter(key, tag)) {
        MEDIA_LOG_E("This key is invalid!");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    {
        OSAL::ScopedLock lock(inputFilterMutex_);
        value = paramsMap_[tag];
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AVInputFilter::Prepare()
{
    MEDIA_LOG_I("Prepare entered.");
    if (state_ != FilterState::INITIALIZED) {
        MEDIA_LOG_E("The current state is invalid");
        return ErrorCode::ERROR_INVALID_STATE;
    }
    state_ = FilterState::PREPARING;
    ErrorCode err = FindPlugin();
    if (err != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("Find plugin fail");
        state_ = FilterState::INITIALIZED;
        return err;
    }
    err = DoConfigure();
    if (err != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("Configure downStream fail");
        state_ = FilterState::INITIALIZED;
        return err;
    }
    err = PreparePlugin();
    if (err != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("Prepare plugin fail");
        state_ = FilterState::INITIALIZED;
        return err;
    }
    state_ = FilterState::READY;
    MEDIA_LOG_I("Prepare end.");
    return ErrorCode::SUCCESS;
}

ErrorCode AVInputFilter::Start()
{
    MEDIA_LOG_I("Start");
    OSAL::ScopedLock lock(inputFilterMutex_);
    if (state_ != FilterState::READY && state_ != FilterState::PAUSED) {
        MEDIA_LOG_E("The current state is invalid");
        return ErrorCode::ERROR_INVALID_STATE;
    }
    if (plugin_ == nullptr) {
        MEDIA_LOG_E("plugin is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (TranslatePluginStatus(plugin_->Start()) != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("The plugin start fail!");
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    state_ = FilterState::RUNNING;
    return ErrorCode::SUCCESS;
}

ErrorCode AVInputFilter::Stop()
{
    MEDIA_LOG_I("Stop");
    OSAL::ScopedLock lock(inputFilterMutex_);
    if (state_ != FilterState::RUNNING) {
        MEDIA_LOG_E("The current state is invalid");
        return ErrorCode::ERROR_INVALID_STATE;
    }
    if (plugin_ == nullptr) {
        MEDIA_LOG_E("plugin is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (TranslatePluginStatus(plugin_->Stop()) != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("The plugin stop fail!");
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    state_ = FilterState::READY;
    return ErrorCode::SUCCESS;
}

ErrorCode AVInputFilter::Pause()
{
    MEDIA_LOG_I("Pause");
    OSAL::ScopedLock lock(inputFilterMutex_);
    if (state_ != FilterState::RUNNING) {
        MEDIA_LOG_E("The current state is invalid");
        return ErrorCode::ERROR_INVALID_STATE;
    }
    if (plugin_ == nullptr) {
        MEDIA_LOG_E("plugin is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (TranslatePluginStatus(plugin_->Stop()) != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("The plugin stop fail!");
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    state_ = FilterState::PAUSED;
    return ErrorCode::SUCCESS;
}

ErrorCode AVInputFilter::Resume()
{
    MEDIA_LOG_I("Resume");
    return ErrorCode::SUCCESS;
}

void AVInputFilter::InitPorts()
{
    MEDIA_LOG_I("InitPorts");
    auto outPort = std::make_shared<OutPort>(this);
    {
        OSAL::ScopedLock lock(inputFilterMutex_);
        outPorts_.push_back(outPort);
    }
}

ErrorCode AVInputFilter::FindPlugin()
{
    OSAL::ScopedLock lock(inputFilterMutex_);
    std::string mime;
    if (paramsMap_.find(Tag::MIME) == paramsMap_.end() ||
        !paramsMap_[Tag::MIME].SameTypeWith(typeid(std::string))) {
        MEDIA_LOG_E("Must set mime correctly first");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    mime = Plugin::AnyCast<std::string>(paramsMap_[Tag::MIME]);
    auto nameList = PluginManager::Instance().ListPlugins(PluginType::AVTRANS_INPUT);
    for (const std::string& name : nameList) {
        auto info = PluginManager::Instance().GetPluginInfo(PluginType::AVTRANS_INPUT, name);
        if (mime != info->outCaps[0].mime) {
            continue;
        }
        if (DoNegotiate(info->outCaps) && CreatePlugin(info) == ErrorCode::SUCCESS) {
            MEDIA_LOG_I("CreatePlugin " PUBLIC_LOG_S " success", name_.c_str());
            return ErrorCode::SUCCESS;
        }
    }
    MEDIA_LOG_I("Cannot find any plugin");
    return ErrorCode::ERROR_UNSUPPORTED_FORMAT;
}

bool AVInputFilter::DoNegotiate(const CapabilitySet& outCaps)
{
    MEDIA_LOG_I("DoNegotiate start");
    if (outCaps.empty()) {
        MEDIA_LOG_E("Input plugin must have out caps");
        return false;
    }
    for (const auto& outCap : outCaps) {
        auto thisOutCap = std::make_shared<Capability>(outCap);
        MEDIA_LOG_I("thisOutCap " PUBLIC_LOG_S, thisOutCap->mime.c_str());
        Meta upstreamParams;
        Meta downstreamParams;
        if (outPorts_.size() == 0 || outPorts_[0] == nullptr) {
            MEDIA_LOG_E("outPorts is empty or invalid!");
            return false;
        }
        if (outPorts_[0]->Negotiate(thisOutCap, capNegWithDownstream_, upstreamParams, downstreamParams)) {
            MEDIA_LOG_I("Negotiate success");
            return true;
        }
    }
    MEDIA_LOG_I("DoNegotiate end");
    return false;
}

ErrorCode AVInputFilter::CreatePlugin(const std::shared_ptr<PluginInfo>& selectedInfo)
{
    MEDIA_LOG_I("CreatePlugin");
    if (selectedInfo == nullptr || selectedInfo->name.empty()) {
        MEDIA_LOG_E("selectedInfo is nullptr or pluginName is invalid!");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    if ((plugin_ != nullptr) && (pluginInfo_ != nullptr)) {
        if (selectedInfo->name == pluginInfo_->name && TranslatePluginStatus(plugin_->Reset()) == ErrorCode::SUCCESS) {
            MEDIA_LOG_I("Reuse last plugin: " PUBLIC_LOG_S, selectedInfo->name.c_str());
            return ErrorCode::SUCCESS;
        }
        if (TranslatePluginStatus(plugin_->Deinit()) != ErrorCode::SUCCESS) {
            MEDIA_LOG_E("Deinit last plugin: " PUBLIC_LOG_S " error", pluginInfo_->name.c_str());
        }
    }
    plugin_ = PluginManager::Instance().CreateAvTransInputPlugin(selectedInfo->name);
    if (plugin_ == nullptr) {
        MEDIA_LOG_E("PluginManager CreatePlugin " PUBLIC_LOG_S " fail", selectedInfo->name.c_str());
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    pluginInfo_ = selectedInfo;
    MEDIA_LOG_I("Create new plugin: " PUBLIC_LOG_S " success", pluginInfo_->name.c_str());
    return ErrorCode::SUCCESS;
}

ErrorCode AVInputFilter::DoConfigure()
{
    Plugin::Meta emptyMeta;
    Plugin::Meta targetMeta;
    if (MergeMeta(emptyMeta, targetMeta) != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("Merge Meta fail!");
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    if (ConfigMeta(targetMeta) != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("Config Meta fail!");
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    if (ConfigDownStream(targetMeta) != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("Config DownStream fail!");
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    auto err = InitPlugin();
    if (err != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("Init plugin fail");
        state_ = FilterState::INITIALIZED;
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    err = ConfigPlugin();
    if (err != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("Configure plugin fail");
        state_ = FilterState::INITIALIZED;
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AVInputFilter::MergeMeta(const Plugin::Meta& meta, Plugin::Meta& targetMeta)
{
    OSAL::ScopedLock lock(inputFilterMutex_);
    if (!MergeMetaWithCapability(meta, capNegWithDownstream_, targetMeta)) {
        MEDIA_LOG_E("cannot find available capability of plugin " PUBLIC_LOG_S, pluginInfo_->name.c_str());
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AVInputFilter::ConfigMeta(Plugin::Meta& meta)
{
    MEDIA_LOG_I("ConfigMeta start!");
    OSAL::ScopedLock lock(inputFilterMutex_);
    if (paramsMap_.find(Tag::MEDIA_TYPE) == paramsMap_.end() ||
        !paramsMap_[Tag::MEDIA_TYPE].SameTypeWith(typeid(Plugin::MediaType))) {
        MEDIA_LOG_E("MEDIA_TYPE in ParamsMap is not exist!");
        return ErrorCode::ERROR_NOT_EXISTED;
    }
    auto mediaType = Plugin::AnyCast<Plugin::MediaType>(paramsMap_[Tag::MEDIA_TYPE]);
    if (mediaType == MediaType::VIDEO) {
        ConfigVideoMeta(meta);
    } else {
        ConfigAudioMeta(meta);
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AVInputFilter::ConfigVideoMeta(Plugin::Meta& meta)
{
    MEDIA_LOG_I("ConfigVideoMeta start!");
    if (paramsMap_.find(Tag::VIDEO_WIDTH) != paramsMap_.end() &&
        paramsMap_[Tag::VIDEO_WIDTH].SameTypeWith(typeid(int))) {
        uint32_t width = Plugin::AnyCast<int>(paramsMap_[Tag::VIDEO_WIDTH]);
        MEDIA_LOG_I("ConfigVideoMeta: VIDEO_WIDTH is " PUBLIC_LOG_U32, width);
        meta.Set<Plugin::Tag::VIDEO_WIDTH>(width);
    }
    if (paramsMap_.find(Tag::VIDEO_HEIGHT) != paramsMap_.end() &&
        paramsMap_[Tag::VIDEO_HEIGHT].SameTypeWith(typeid(int))) {
        uint32_t height = Plugin::AnyCast<int>(paramsMap_[Tag::VIDEO_HEIGHT]);
        MEDIA_LOG_I("ConfigVideoMeta: VIDEO_HEIGHT is " PUBLIC_LOG_U32, height);
        meta.Set<Plugin::Tag::VIDEO_HEIGHT>(height);
    }
    if (paramsMap_.find(Tag::MEDIA_BITRATE) != paramsMap_.end() &&
        paramsMap_[Tag::MEDIA_BITRATE].SameTypeWith(typeid(int))) {
        int64_t mediaBitRate = Plugin::AnyCast<int>(paramsMap_[Tag::MEDIA_BITRATE]);
        MEDIA_LOG_I("ConfigVideoMeta: MEDIA_BITRATE is " PUBLIC_LOG_D64, mediaBitRate);
        meta.Set<Plugin::Tag::MEDIA_BITRATE>(mediaBitRate);
    }
    if (paramsMap_.find(Tag::VIDEO_FRAME_RATE) != paramsMap_.end() &&
        paramsMap_[Tag::VIDEO_FRAME_RATE].SameTypeWith(typeid(int))) {
        uint32_t videoFrameRate = Plugin::AnyCast<int>(paramsMap_[Tag::VIDEO_FRAME_RATE]);
        MEDIA_LOG_I("ConfigVideoMeta: VIDEO_FRAME_RATE is " PUBLIC_LOG_U32, videoFrameRate);
        meta.Set<Plugin::Tag::VIDEO_FRAME_RATE>(videoFrameRate);
    }
    if (paramsMap_.find(Tag::VIDEO_BIT_STREAM_FORMAT) != paramsMap_.end() &&
        paramsMap_[Tag::VIDEO_BIT_STREAM_FORMAT].SameTypeWith(typeid(VideoBitStreamFormat))) {
        auto videoBitStreamFormat = Plugin::AnyCast<VideoBitStreamFormat>(paramsMap_[Tag::VIDEO_BIT_STREAM_FORMAT]);
        MEDIA_LOG_I("ConfigVideoMeta: VIDEO_BIT_STREAM_FORMAT is " PUBLIC_LOG_U32, videoBitStreamFormat);
        meta.Set<Plugin::Tag::VIDEO_BIT_STREAM_FORMAT>(std::vector<VideoBitStreamFormat>{videoBitStreamFormat});
    }
    if (paramsMap_.find(Tag::VIDEO_PIXEL_FORMAT) != paramsMap_.end() &&
        paramsMap_[Tag::VIDEO_PIXEL_FORMAT].SameTypeWith(typeid(VideoPixelFormat))) {
        auto videoPixelFormat = Plugin::AnyCast<VideoPixelFormat>(paramsMap_[Tag::VIDEO_PIXEL_FORMAT]);
        MEDIA_LOG_I("ConfigVideoMeta: VIDEO_PIXEL_FORMAT is " PUBLIC_LOG_U32, videoPixelFormat);
        meta.Set<Plugin::Tag::VIDEO_PIXEL_FORMAT>(videoPixelFormat);
    }
    return ErrorCode::SUCCESS;
}

OHOS::Media::Plugin::AudioChannelLayout AVInputFilter::TransAudioChannelLayout(int layoutPtr)
{
    const static std::pair<int, OHOS::Media::Plugin::AudioChannelLayout> mapArray[] = {
        {1, OHOS::Media::Plugin::AudioChannelLayout::MONO},
        {2, OHOS::Media::Plugin::AudioChannelLayout::STEREO},
    };
    for (const auto& item : mapArray) {
        if (item.first == layoutPtr) {
            return item.second;
        }
    }
    return OHOS::Media::Plugin::AudioChannelLayout::UNKNOWN;
}

OHOS::Media::Plugin::AudioSampleFormat AVInputFilter::TransAudioSampleFormat(int sampleFormat)
{
    const static std::pair<int, OHOS::Media::Plugin::AudioSampleFormat> mapArray[] = {
        {0, OHOS::Media::Plugin::AudioSampleFormat::U8},
        {1, OHOS::Media::Plugin::AudioSampleFormat::S16},
        {2, OHOS::Media::Plugin::AudioSampleFormat::S24},
        {3, OHOS::Media::Plugin::AudioSampleFormat::S32},
        {4, OHOS::Media::Plugin::AudioSampleFormat::F32P},
        {-1, OHOS::Media::Plugin::AudioSampleFormat::NONE},
    };
    for (const auto& item : mapArray) {
        if (item.first == sampleFormat) {
            return item.second;
        }
    }
    return OHOS::Media::Plugin::AudioSampleFormat::NONE;
}

ErrorCode AVInputFilter::ConfigAudioMeta(Plugin::Meta& meta)
{
    MEDIA_LOG_I("ConfigAudioMeta start");
    if (paramsMap_.find(Tag::AUDIO_CHANNELS) != paramsMap_.end() &&
        paramsMap_[Tag::AUDIO_CHANNELS].SameTypeWith(typeid(int))) {
        uint32_t audioChannel = Plugin::AnyCast<int>(paramsMap_[Tag::AUDIO_CHANNELS]);
        MEDIA_LOG_I("ConfigAudioMeta: AUDIO_CHANNELS is " PUBLIC_LOG_U32, audioChannel);
        meta.Set<Plugin::Tag::AUDIO_CHANNELS>(audioChannel);
    }
    if (paramsMap_.find(Tag::AUDIO_SAMPLE_RATE) != paramsMap_.end() &&
        paramsMap_[Tag::AUDIO_SAMPLE_RATE].SameTypeWith(typeid(int))) {
        uint32_t sampleRate = Plugin::AnyCast<int>(paramsMap_[Tag::AUDIO_SAMPLE_RATE]);
        MEDIA_LOG_I("ConfigAudioMeta: AUDIO_SAMPLE_RATE is " PUBLIC_LOG_U32, sampleRate);
        meta.Set<Plugin::Tag::AUDIO_SAMPLE_RATE>(sampleRate);
    }
    if (paramsMap_.find(Tag::MEDIA_BITRATE) != paramsMap_.end() &&
        paramsMap_[Tag::MEDIA_BITRATE].SameTypeWith(typeid(int))) {
        int64_t mediaBitRate = Plugin::AnyCast<int>(paramsMap_[Tag::MEDIA_BITRATE]);
        MEDIA_LOG_I("ConfigAudioMeta: MEDIA_BITRATE is " PUBLIC_LOG_D64, mediaBitRate);
        meta.Set<Plugin::Tag::MEDIA_BITRATE>(mediaBitRate);
    }
    if (paramsMap_.find(Tag::AUDIO_SAMPLE_FORMAT) != paramsMap_.end() &&
        paramsMap_[Tag::AUDIO_SAMPLE_FORMAT].SameTypeWith(typeid(int))) {
        auto audioSampleFmtPtr = Plugin::AnyCast<int>(paramsMap_[Tag::AUDIO_SAMPLE_FORMAT]);
        MEDIA_LOG_I("ConfigAudioMeta: AUDIO_SAMPLE_FORMAT is " PUBLIC_LOG_U32, audioSampleFmtPtr);
        meta.Set<Plugin::Tag::AUDIO_SAMPLE_FORMAT>(TransAudioSampleFormat(audioSampleFmtPtr));
    }
    if (paramsMap_.find(Tag::AUDIO_CHANNEL_LAYOUT) != paramsMap_.end() &&
        paramsMap_[Tag::AUDIO_CHANNEL_LAYOUT].SameTypeWith(typeid(int))) {
        auto layoutPtr = Plugin::AnyCast<int>(paramsMap_[Tag::AUDIO_CHANNEL_LAYOUT]);
        MEDIA_LOG_I("ConfigAudioMeta: AUDIO_CHANNEL_LAYOUT is " PUBLIC_LOG_U32, layoutPtr);
        meta.Set<Plugin::Tag::AUDIO_CHANNEL_LAYOUT>(TransAudioChannelLayout(layoutPtr));
    }
    if (paramsMap_.find(Tag::AUDIO_SAMPLE_PER_FRAME) != paramsMap_.end() &&
        paramsMap_[Tag::AUDIO_SAMPLE_PER_FRAME].SameTypeWith(typeid(int))) {
        uint32_t samplePerFrame = Plugin::AnyCast<int>(paramsMap_[Tag::AUDIO_SAMPLE_PER_FRAME]);
        MEDIA_LOG_I("ConfigAudioMeta: AUDIO_SAMPLE_PER_FRAME is " PUBLIC_LOG_U32, samplePerFrame);
        meta.Set<Plugin::Tag::AUDIO_SAMPLE_PER_FRAME>(samplePerFrame);
    }
    if (paramsMap_.find(Tag::AUDIO_AAC_LEVEL) != paramsMap_.end() &&
        paramsMap_[Tag::AUDIO_AAC_LEVEL].SameTypeWith(typeid(int))) {
        uint32_t aacLevel = Plugin::AnyCast<int>(paramsMap_[Tag::AUDIO_AAC_LEVEL]);
        MEDIA_LOG_I("ConfigAudioMeta: AUDIO_AAC_LEVEL is " PUBLIC_LOG_U32, aacLevel);
        meta.Set<Plugin::Tag::AUDIO_AAC_LEVEL>(aacLevel);
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AVInputFilter::ConfigDownStream(const Plugin::Meta& meta)
{
    Meta upstreamParams;
    Meta downstreamParams;
    OSAL::ScopedLock lock(inputFilterMutex_);
    if (outPorts_.size() == 0 || outPorts_[0] == nullptr) {
        MEDIA_LOG_E("outPorts is empty or invalid!");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    if (!outPorts_[0]->Configure(std::make_shared<Plugin::Meta>(meta), upstreamParams, downstreamParams)) {
        MEDIA_LOG_E("Configure downstream fail");
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AVInputFilter::InitPlugin()
{
    MEDIA_LOG_I("InitPlugin");
    OSAL::ScopedLock lock(inputFilterMutex_);
    if (plugin_ == nullptr) {
        MEDIA_LOG_E("plugin is nullptr!");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    return TranslatePluginStatus(plugin_->Init());
}

ErrorCode AVInputFilter::ConfigPlugin()
{
    MEDIA_LOG_I("Configure");
    ErrorCode err = SetPluginParams();
    if (err != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("Set Plugin fail!");
        return err;
    }
    err = SetEventCallBack();
    if (err != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("Plugin SetEventCallBack fail!");
        return err;
    }
    err = SetDataCallBack();
    if (err != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("Plugin SetDataCallBack fail!");
        return err;
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AVInputFilter::SetPluginParams()
{
    OSAL::ScopedLock lock(inputFilterMutex_);
    if (plugin_ == nullptr) {
        MEDIA_LOG_E("plugin is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (paramsMap_.find(Tag::MEDIA_DESCRIPTION) != paramsMap_.end()) {
        plugin_->SetParameter(Tag::MEDIA_DESCRIPTION, paramsMap_[Tag::MEDIA_DESCRIPTION]);
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AVInputFilter::PreparePlugin()
{
    OSAL::ScopedLock lock(inputFilterMutex_);
    if (plugin_ == nullptr) {
        MEDIA_LOG_E("plugin is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return TranslatePluginStatus(plugin_->Prepare());
}

ErrorCode AVInputFilter::PushData(const std::string& inPort, const AVBufferPtr& buffer, int64_t offset)
{
    OSAL::ScopedLock lock(inputFilterMutex_);
    if (name_.compare(inPort) != 0) {
        MEDIA_LOG_E("FilterName is not targetName!");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    if (buffer == nullptr || plugin_ == nullptr) {
        MEDIA_LOG_E("buffer or plugin is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    MEDIA_LOG_I("PushData to plugin");
    if (TranslatePluginStatus(plugin_->PushData(inPort, buffer, offset)) != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("PushData to plugin fail!");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    MEDIA_LOG_I("PushData from plugin");
    if (outPorts_.size() == 0 || outPorts_[0] == nullptr) {
        MEDIA_LOG_E("outPorts is empty or invalid!");
        return ErrorCode::ERROR_INVALID_PARAMETER_VALUE;
    }
    MEDIA_LOG_I("PushData to next filter plugin!");
    outPorts_[0]->PushData(buffer, 0);
    return ErrorCode::SUCCESS;
}

ErrorCode AVInputFilter::SetEventCallBack()
{
    OSAL::ScopedLock lock(inputFilterMutex_);
    if (plugin_ == nullptr) {
        MEDIA_LOG_E("plugin is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER ;
    }
    return TranslatePluginStatus(plugin_->SetCallback(this));
}

ErrorCode AVInputFilter::SetDataCallBack()
{
    OSAL::ScopedLock lock(inputFilterMutex_);
    if (plugin_ == nullptr) {
        MEDIA_LOG_E("plugin is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return TranslatePluginStatus(plugin_->SetDataCallback(std::bind(&AVInputFilter::OnDataCallback, this,
        std::placeholders::_1)));
}

void AVInputFilter::OnDataCallback(std::shared_ptr<Plugin::Buffer> buffer)
{
    OSAL::ScopedLock lock(inputFilterMutex_);
    if (buffer == nullptr) {
        MEDIA_LOG_E("buffer is nullptr!");
        return;
    }
    if (outPorts_.size() == 0 || outPorts_[0] == nullptr) {
        MEDIA_LOG_E("outPorts is invalid!");
        return;
    }
    outPorts_[0]->PushData(buffer, 0);
}
} // namespace Pipeline
} // namespace Media
} // namespace OHOS