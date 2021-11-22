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

#define HST_LOG_TAG "AudioDecoderFilter"

#include "audio_decoder_filter.h"
#include "osal/utils/util.h"
#include "utils/constants.h"
#include "utils/memory_helper.h"
#include "factory/filter_factory.h"
#include "common/plugin_utils.h"
#include "plugin/common/plugin_audio_tags.h"
#include "utils/steady_clock.h"

namespace {
constexpr int32_t DEFAULT_OUT_BUFFER_POOL_SIZE = 5;
constexpr int32_t DEFAULT_IN_BUFFER_POOL_SIZE = 5;
constexpr int32_t MAX_OUT_DECODED_DATA_SIZE_PER_FRAME = 20 * 1024; // 20kB
constexpr int32_t AF_64BIT_BYTES = 8;
constexpr int32_t AF_32BIT_BYTES = 4;
constexpr int32_t AF_16BIT_BYTES = 2;
constexpr int32_t AF_8BIT_BYTES = 1;

int32_t CalculateBufferSize(const std::shared_ptr<const OHOS::Media::Plugin::Meta> &meta)
{
    using namespace OHOS::Media;
    int32_t samplesPerFrame;
    if (!meta->GetInt32(Plugin::MetaID::AUDIO_SAMPLE_PER_FRAME, samplesPerFrame)) {
        return 0;
    }
    int32_t channels;
    if (!meta->GetInt32(Plugin::MetaID::AUDIO_CHANNELS, channels)) {
        return 0;
    }
    int32_t bytesPerSample = 0;
    Plugin::AudioSampleFormat format;
    if (!meta->GetData<Plugin::AudioSampleFormat>(Plugin::MetaID::AUDIO_SAMPLE_FORMAT, format)) {
        return 0;
    }
    switch (format) {
        case Plugin::AudioSampleFormat::S64:
        case Plugin::AudioSampleFormat::S64P:
        case Plugin::AudioSampleFormat::U64:
        case Plugin::AudioSampleFormat::U64P:
        case Plugin::AudioSampleFormat::F64:
        case Plugin::AudioSampleFormat::F64P:
            bytesPerSample = AF_64BIT_BYTES;
            break;
        case Plugin::AudioSampleFormat::F32:
        case Plugin::AudioSampleFormat::F32P:
        case Plugin::AudioSampleFormat::S32:
        case Plugin::AudioSampleFormat::S32P:
        case Plugin::AudioSampleFormat::U32:
        case Plugin::AudioSampleFormat::U32P:
            bytesPerSample = AF_32BIT_BYTES;
            break;
        case Plugin::AudioSampleFormat::S16:
        case Plugin::AudioSampleFormat::S16P:
        case Plugin::AudioSampleFormat::U16:
        case Plugin::AudioSampleFormat::U16P:
            bytesPerSample = AF_16BIT_BYTES;
            break;
        case Plugin::AudioSampleFormat::S8:
        case Plugin::AudioSampleFormat::U8:
            bytesPerSample = AF_8BIT_BYTES;
            break;
        default:
            bytesPerSample = 0;
            break;
    }
    return bytesPerSample * samplesPerFrame * channels;
}
};

namespace OHOS {
namespace Media {
namespace Pipeline {
static AutoRegisterFilter<AudioDecoderFilter> g_registerFilterHelper("builtin.player.audiodecoder");

AudioDecoderFilter::AudioDecoderFilter(const std::string &name): DecoderFilterBase(name)
{
    filterType_ = FilterType::AUDIO_DECODER;
    MEDIA_LOG_D("audio decoder ctor called");
}

AudioDecoderFilter::~AudioDecoderFilter()
{
    MEDIA_LOG_D("audio decoder dtor called");
    Release();
}

ErrorCode AudioDecoderFilter::Start()
{
    MEDIA_LOG_I("audio decoder start called");
    if (state_ != FilterState::READY && state_ != FilterState::PAUSED) {
        MEDIA_LOG_W("call decoder start() when state is not ready or working");
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    return FilterBase::Start();
}

ErrorCode AudioDecoderFilter::Prepare()
{
    MEDIA_LOG_I("audio decoder prepare called");
    if (state_ != FilterState::INITIALIZED) {
        MEDIA_LOG_W("decoder filter is not in init state");
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    auto err = FilterBase::Prepare();
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, "Audio Decoder prepare error because of filter base prepare error");
    return ErrorCode::SUCCESS;
}

bool AudioDecoderFilter::Negotiate(const std::string& inPort,
                                   const std::shared_ptr<const Plugin::Capability>& upstreamCap,
                                   Capability& upstreamNegotiatedCap)
{
    PROFILE_BEGIN("Audio Decoder Negotiate begin");
    if (state_ != FilterState::PREPARING) {
        MEDIA_LOG_W("decoder filter is not in preparing when negotiate");
        return false;
    }

    auto targetOutPort = GetRouteOutPort(inPort);
    if (targetOutPort == nullptr) {
        MEDIA_LOG_E("decoder out port is not found");
        return false;
    }
    std::shared_ptr<Plugin::PluginInfo> selectedPluginInfo;
    bool atLeastOutCapMatched = false;
    auto candidatePlugins = FindAvailablePlugins(*upstreamCap, Plugin::PluginType::CODEC);
    for (const auto& candidate : candidatePlugins) {
        if (candidate.first->outCaps.empty()) {
            MEDIA_LOG_E("decoder plugin must have out caps");
        }
        for (const auto& outCap : candidate.first->outCaps) { // each codec plugin should have at least one out cap
            auto thisOut = std::make_shared<Plugin::Capability>();
            if (!MergeCapabilityKeys(*upstreamCap, outCap, *thisOut)) {
                MEDIA_LOG_W("one of out cap of plugin %s does not match with upstream capability",
                            candidate.first->name.c_str());
                continue;
            }
            atLeastOutCapMatched = true;
            thisOut->mime = outCap.mime;
            if (targetOutPort->Negotiate(thisOut, capNegWithDownstream_)) {
                capNegWithUpstream_ = candidate.second;
                selectedPluginInfo = candidate.first;
                MEDIA_LOG_I("choose plugin %s as working parameter", candidate.first->name.c_str());
                break;
            }
        }
    }
    if (!atLeastOutCapMatched) {
        MEDIA_LOG_W("cannot find available decoder plugin");
        return false;
    }
    if (selectedPluginInfo == nullptr) {
        MEDIA_LOG_W("cannot find available downstream plugin");
        return false;
    }
    PROFILE_END("Audio Decoder Negotiate end");
    return UpdateAndInitPluginByInfo(selectedPluginInfo);
}

bool AudioDecoderFilter::Configure(const std::string &inPort, const std::shared_ptr<const Plugin::Meta> &upstreamMeta)
{
    PROFILE_BEGIN("Audio decoder configure begin");
    if (plugin_ == nullptr || targetPluginInfo_ == nullptr) {
        MEDIA_LOG_E("cannot configure decoder when no plugin available");
        return false;
    }

    auto thisMeta = std::make_shared<Plugin::Meta>();
    if (!MergeMetaWithCapability(*upstreamMeta, capNegWithDownstream_, *thisMeta)) {
        MEDIA_LOG_E("cannot configure decoder plugin since meta is not compatible with negotiated caps");
    }
    auto targetOutPort = GetRouteOutPort(inPort);
    if (targetOutPort == nullptr) {
        MEDIA_LOG_E("decoder out port is not found");
        return false;
    }
    if (!targetOutPort->Configure(thisMeta)) {
        MEDIA_LOG_E("decoder filter downstream Configure failed");
        return false;
    }
    auto err = ConfigureToStartPluginLocked(thisMeta);
    if (err != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("decoder configure error");
        OnEvent({EVENT_ERROR, err});
        return false;
    }
    state_ = FilterState::READY;
    OnEvent({EVENT_READY});
    MEDIA_LOG_I("audio decoder send EVENT_READY");
    PROFILE_END("Audio decoder configure end");
    return true;
}

ErrorCode AudioDecoderFilter::ConfigureToStartPluginLocked(const std::shared_ptr<const Plugin::Meta>& meta)
{
    auto err = ConfigureWithMetaLocked(meta);
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, "configure decoder plugin error");

    uint32_t bufferCnt = 0;
    if (GetPluginParameterLocked(Tag::REQUIRED_OUT_BUFFER_CNT, bufferCnt) != ErrorCode::SUCCESS) {
        bufferCnt = DEFAULT_OUT_BUFFER_POOL_SIZE;
    }

    // 每次重新创建bufferPool
    outBufferPool_ = std::make_shared<BufferPool<AVBuffer>>(bufferCnt);

    int32_t bufferSize = CalculateBufferSize(meta);
    if (bufferSize == 0) {
        bufferSize = MAX_OUT_DECODED_DATA_SIZE_PER_FRAME;
    }
    auto outAllocator = plugin_->GetAllocator();
    if (outAllocator == nullptr) {
        MEDIA_LOG_I("plugin doest not support out allocator, using framework allocator");
        outBufferPool_->Init(bufferSize);
    } else {
        MEDIA_LOG_I("using plugin output allocator");
        for (size_t cnt = 0; cnt < bufferCnt; cnt++) {
            auto buf = MemoryHelper::make_unique<AVBuffer>();
            buf->AllocMemory(outAllocator, bufferSize);
            outBufferPool_->Append(std::move(buf));
        }
    }

    err = TranslatePluginStatus(plugin_->Prepare());
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, "decoder prepare failed");
    err = TranslatePluginStatus(plugin_->Start());
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, "decoder start failed");

    return ErrorCode::SUCCESS;
}

ErrorCode AudioDecoderFilter::PushData(const std::string &inPort, AVBufferPtr buffer)
{
    const static int8_t maxRetryCnt = 3; // max retry times of handling one frame
    if (state_ != FilterState::READY && state_ != FilterState::PAUSED && state_ != FilterState::RUNNING) {
        MEDIA_LOG_W("pushing data to decoder when state is %d", static_cast<int>(state_.load()));
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    if (isFlushing_) {
        MEDIA_LOG_I("decoder is flushing, discarding this data from port %s", inPort.c_str());
        return ErrorCode::SUCCESS;
    }
    ErrorCode handleFrameRes;
    int8_t retryCnt = 0;
    do {
        handleFrameRes = HandleFrame(buffer);
        while (FinishFrame() == ErrorCode::SUCCESS) {
            MEDIA_LOG_D("finish frame");
        }
        retryCnt++;
        if (retryCnt >= maxRetryCnt) { // if retry cnt exceeds we will drop this frame
            break;
        }
        // if timed out or returns again we should try again
    } while (handleFrameRes == ErrorCode::ERROR_TIMED_OUT || handleFrameRes == ErrorCode::ERROR_AGAIN);
    return ErrorCode::SUCCESS;
}

void AudioDecoderFilter::FlushStart()
{
    MEDIA_LOG_I("FlushStart entered.");
    isFlushing_ = true;
    if (plugin_ != nullptr) {
        auto err = TranslatePluginStatus(plugin_->Flush());
        if (err != ErrorCode::SUCCESS) {
            MEDIA_LOG_E("decoder plugin flush error");
        }
    }
    MEDIA_LOG_I("FlushStart exit.");
}

void AudioDecoderFilter::FlushEnd()
{
    MEDIA_LOG_I("FlushEnd entered");
    isFlushing_ = false;
}

ErrorCode AudioDecoderFilter::Stop()
{
    MEDIA_LOG_I("AudioDecoderFilter stop start.");
    // 先改变底层状态 然后停掉上层线程 否则会产生死锁
    ErrorCode err ;
    if (plugin_ != nullptr) {
        err = TranslatePluginStatus(plugin_->Flush());
        RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, "decoder flush error");
        err = TranslatePluginStatus(plugin_->Stop());
        RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, "decoder stop error");
    }
    MEDIA_LOG_I("AudioDecoderFilter stop end.");
    return FilterBase::Stop();
}

ErrorCode AudioDecoderFilter::Release()
{
    if (plugin_) {
        plugin_->Stop();
        plugin_->Deinit();
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AudioDecoderFilter::HandleFrame(const std::shared_ptr<AVBuffer>& buffer)
{
    MEDIA_LOG_D("HandleFrame called");
    auto ret = TranslatePluginStatus(plugin_->QueueInputBuffer(buffer, 0));
    if (ret != ErrorCode::SUCCESS && ret != ErrorCode::ERROR_TIMED_OUT) {
        MEDIA_LOG_E("Queue input buffer to plugin fail: %d", ret);
    }
    return ret;
}

ErrorCode AudioDecoderFilter::FinishFrame()
{
    MEDIA_LOG_D("begin finish frame");
    auto outBuffer = outBufferPool_->AllocateAppendBufferNonBlocking();
    if (outBuffer == nullptr) {
        MEDIA_LOG_E("Get out buffer from buffer pool fail");
        return ErrorCode::ERROR_NO_MEMORY;
    }
    outBuffer->Reset();
    auto ret = TranslatePluginStatus(plugin_->QueueOutputBuffer(outBuffer, 0));
    if (ret != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("Queue out buffer to plugin fail: %d", ret);
        return ret;
    }
    std::shared_ptr<AVBuffer> pcmFrame = nullptr;
    auto status = plugin_->DequeueOutputBuffer(pcmFrame, 0);
    if (status != Plugin::Status::OK && status != Plugin::Status::END_OF_STREAM) {
        if (status != Plugin::Status::ERROR_NOT_ENOUGH_DATA) {
            MEDIA_LOG_E("Dequeue pcm frame from plugin fail: %d", status);
        }
        return TranslatePluginStatus(status);
    }
    if (pcmFrame) {
        // push to port
        auto oPort = outPorts_[0];
        if (oPort->GetWorkMode() == WorkMode::PUSH) {
            oPort->PushData(pcmFrame);
        } else {
            MEDIA_LOG_W("decoder out port works in pull mode");
        }
        pcmFrame.reset(); // 释放buffer 如果没有被缓存使其回到buffer pool 如果被sink缓存 则从buffer pool拿其他的buffer
    }
    MEDIA_LOG_D("end finish frame");
    return ErrorCode::SUCCESS;
}
}
}
}
