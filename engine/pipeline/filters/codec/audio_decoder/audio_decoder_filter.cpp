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
#include "common/plugin_utils.h"
#include "foundation/cpp_ext/memory_ext.h"
#include "factory/filter_factory.h"
#include "filters/common/dump_buffer.h"
#include "plugin/common/plugin_audio_tags.h"
#include "plugin/core/plugin_manager.h"
#include "utils/steady_clock.h"
#include "utils/constants.h"

namespace {
constexpr int32_t DEFAULT_OUT_BUFFER_POOL_SIZE = 5;
constexpr int32_t MAX_OUT_DECODED_DATA_SIZE_PER_FRAME = 20 * 1024; // 20kB
constexpr int32_t MAX_SAMPLE_PER_FRAME = 10240; // 10240 set max samples per frame

uint32_t CalculateBufferSize(const std::shared_ptr<const OHOS::Media::Plugin::Meta> &meta)
{
    using namespace OHOS::Media;
    uint32_t samplesPerFrame;
    if (!meta->GetUint32(Plugin::MetaID::AUDIO_SAMPLE_PER_FRAME, samplesPerFrame)) {
        return 0;
    }
    uint32_t channels;
    if (!meta->GetUint32(Plugin::MetaID::AUDIO_CHANNELS, channels)) {
        return 0;
    }
    Plugin::AudioSampleFormat format;
    if (!meta->GetData<Plugin::AudioSampleFormat>(Plugin::MetaID::AUDIO_SAMPLE_FORMAT, format)) {
        return 0;
    }
    return Pipeline::GetBytesPerSample(format) * samplesPerFrame * channels;
}
};

namespace OHOS {
namespace Media {
namespace Pipeline {
static AutoRegisterFilter<AudioDecoderFilter> g_registerFilterHelper("builtin.player.audiodecoder");

AudioDecoderFilter::AudioDecoderFilter(const std::string &name): CodecFilterBase(name)
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

bool AudioDecoderFilter::Negotiate(const std::string& inPort,
                                   const std::shared_ptr<const Plugin::Capability>& upstreamCap,
                                   Plugin::Capability& negotiatedCap,
                                   const Plugin::TagMap& upstreamParams,
                                   Plugin::TagMap& downstreamParams)
{
    PROFILE_BEGIN("Audio Decoder Negotiate begin");
    FALSE_RETURN_V_MSG_E(state_ == FilterState::PREPARING, false, "filter is not preparing when negotiate");
    auto targetOutPort = GetRouteOutPort(inPort);
    FALSE_RETURN_V_MSG_E(targetOutPort != nullptr, false, "decoder out port is not found");
    std::shared_ptr<Plugin::PluginInfo> selectedPluginInfo = nullptr;
    bool atLeastOutCapMatched = false;
    auto candidatePlugins = FindAvailablePlugins(*upstreamCap, Plugin::PluginType::CODEC);
    for (const auto& candidate : candidatePlugins) {
        FALSE_LOG_MSG(!candidate.first->outCaps.empty(), "decoder plugin must have out caps");
        for (const auto& outCap : candidate.first->outCaps) { // each codec plugin should have at least one out cap
            if (outCap.keys.count(Capability::Key::AUDIO_SAMPLE_FORMAT) == 0) {
                MEDIA_LOG_W("decoder plugin must specify sample format in out caps");
                continue;
            }
            auto thisOut = std::make_shared<Plugin::Capability>();
            if (!MergeCapabilityKeys(*upstreamCap, outCap, *thisOut)) {
                MEDIA_LOG_I("one cap of plugin " PUBLIC_LOG_S " mismatch upstream cap", candidate.first->name.c_str());
                continue;
            }
            atLeastOutCapMatched = true;
            thisOut->mime = outCap.mime;
            if (targetOutPort->Negotiate(thisOut, capNegWithDownstream_, upstreamParams, downstreamParams)) {
                capNegWithUpstream_ = candidate.second;
                selectedPluginInfo = candidate.first;
                MEDIA_LOG_I("use plugin " PUBLIC_LOG_S, candidate.first->name.c_str());
                MEDIA_LOG_I("neg upstream cap " PUBLIC_LOG_S, Capability2String(capNegWithUpstream_).c_str());
                MEDIA_LOG_I("neg downstream cap " PUBLIC_LOG_S, Capability2String(capNegWithDownstream_).c_str());
                break;
            }
        }
        if (selectedPluginInfo != nullptr) {
            break;
        }
    }
    FALSE_RETURN_V_MSG_E(atLeastOutCapMatched && selectedPluginInfo != nullptr, false,
        "can't find available decoder plugin with " PUBLIC_LOG_S, Capability2String(*upstreamCap).c_str());
    auto res = UpdateAndInitPluginByInfo<Plugin::Codec>(plugin_, pluginInfo_, selectedPluginInfo,
        [](const std::string& name)-> std::shared_ptr<Plugin::Codec> {
                return Plugin::PluginManager::Instance().CreateCodecPlugin(name);
        });
    plugin_->SetCallback(this);
    plugin_->SetDataCallback(this);
    PROFILE_END("Audio Decoder Negotiate end");
    return res;
}

bool AudioDecoderFilter::Configure(const std::string &inPort, const std::shared_ptr<const Plugin::Meta> &upstreamMeta)
{
    PROFILE_BEGIN("Audio decoder configure begin");
    MEDIA_LOG_I("receive upstream meta " PUBLIC_LOG_S, Meta2String(*upstreamMeta).c_str());
    FALSE_RETURN_V_MSG_E(plugin_ != nullptr && pluginInfo_ != nullptr, false,
                         "can't configure decoder when no plugin available");
    auto thisMeta = std::make_shared<Plugin::Meta>();
    FALSE_RETURN_V_MSG_E(MergeMetaWithCapability(*upstreamMeta, capNegWithDownstream_, *thisMeta), false,
                         "can't configure decoder plugin since meta is not compatible with negotiated caps");
    uint32_t samplesPerFrame = 0;
    if (GetPluginParameterLocked(Tag::AUDIO_SAMPLE_PER_FRAME, samplesPerFrame) != ErrorCode::SUCCESS) {
        MEDIA_LOG_W("Can't acquire samples per frame from decoder plugin: " PUBLIC_LOG_S,  pluginInfo_->name.c_str());
        samplesPerFrame = MAX_SAMPLE_PER_FRAME;
    }
    (void) thisMeta->SetUint32(Plugin::MetaID::AUDIO_SAMPLE_PER_FRAME, samplesPerFrame);
    auto targetOutPort = GetRouteOutPort(inPort);
    FALSE_RETURN_V_MSG_E(targetOutPort != nullptr, false, "encoder out port is not found");
    FALSE_RETURN_V_MSG_E(targetOutPort->Configure(thisMeta), false, "fail to configure downstream");
    auto err = ConfigureToStartPluginLocked(thisMeta);
    if (err != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("decoder configure error");
        OnEvent({name_, EventType::EVENT_ERROR, err});
        return false;
    }
    state_ = FilterState::READY;
    OnEvent({name_, EventType::EVENT_READY, err});
    MEDIA_LOG_I("audio decoder send EVENT_READY");
    PROFILE_END("Audio decoder configure end");
    return true;
}

ErrorCode AudioDecoderFilter::ConfigureToStartPluginLocked(const std::shared_ptr<const Plugin::Meta>& meta)
{
    auto err = ConfigPluginWithMeta(*plugin_, *meta);
    FAIL_RETURN_MSG(err, "configure decoder plugin error");

    uint32_t bufferCnt = 0;
    if (GetPluginParameterLocked(Tag::REQUIRED_OUT_BUFFER_CNT, bufferCnt) != ErrorCode::SUCCESS) {
        bufferCnt = DEFAULT_OUT_BUFFER_POOL_SIZE;
    }

    // 每次重新创建bufferPool
    outBufferPool_ = std::make_shared<BufferPool<AVBuffer>>(bufferCnt);

    auto bufferSize = CalculateBufferSize(meta);
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
            auto buf = CppExt::make_unique<AVBuffer>();
            buf->AllocMemory(outAllocator, bufferSize);
            outBufferPool_->Append(std::move(buf));
        }
    }

    err = TranslatePluginStatus(plugin_->Prepare());
    FAIL_RETURN_MSG(err, "decoder prepare failed");
    err = TranslatePluginStatus(plugin_->Start());
    FAIL_RETURN_MSG(err, "decoder start failed");

    return ErrorCode::SUCCESS;
}

ErrorCode AudioDecoderFilter::PushData(const std::string &inPort, const AVBufferPtr& buffer, int64_t offset)
{
    const static int8_t maxRetryCnt = 3; // max retry times of handling one frame
    if (state_ != FilterState::READY && state_ != FilterState::PAUSED && state_ != FilterState::RUNNING) {
        MEDIA_LOG_W("pushing data to decoder when state is " PUBLIC_LOG "d", static_cast<int>(state_.load()));
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    if (isFlushing_) {
        MEDIA_LOG_I("decoder is flushing, discarding this data from port " PUBLIC_LOG "s", inPort.c_str());
        return ErrorCode::SUCCESS;
    }

    DUMP_BUFFER2FILE("decoder_input.data", buffer);

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
        FAIL_RETURN_MSG(err, "decoder flush error");
        err = TranslatePluginStatus(plugin_->Stop());
        FAIL_RETURN_MSG(err, "decoder stop error");
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
        MEDIA_LOG_E("Queue input buffer to plugin fail: " PUBLIC_LOG "d", CppExt::to_underlying(ret));
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
    auto status = plugin_->QueueOutputBuffer(outBuffer, 0);
    if (status != Plugin::Status::OK && status != Plugin::Status::END_OF_STREAM) {
        if (status != Plugin::Status::ERROR_NOT_ENOUGH_DATA) {
            MEDIA_LOG_E("Queue output buffer to plugin fail: " PUBLIC_LOG_D32, static_cast<int32_t>((status)));
        }
    }
    MEDIA_LOG_D("end finish frame");
    return TranslatePluginStatus(status);
}

void AudioDecoderFilter::OnInputBufferDone(const std::shared_ptr<Plugin::Buffer>& input)
{
    MEDIA_LOG_D("AudioDecoderFilter::OnInputBufferDone");
}

void AudioDecoderFilter::OnOutputBufferDone(const std::shared_ptr<Plugin::Buffer>& output)
{
    MEDIA_LOG_D("begin");
    FALSE_RETURN(output != nullptr);

    // push to port
    auto oPort = outPorts_[0];
    if (oPort->GetWorkMode() == WorkMode::PUSH) {
        DUMP_BUFFER2FILE("decoder_output.data", output);
       oPort->PushData(output, -1);
    } else {
       MEDIA_LOG_W("decoder out port works in pull mode");
    }

    // 释放buffer 如果没有被缓存使其回到buffer pool 如果被sink缓存 则从buffer pool拿其他的buffer
    std::const_pointer_cast<Plugin::Buffer>(output).reset();
    MEDIA_LOG_D("end");
}
} // Pipeline
} // Media
} // OHOS
