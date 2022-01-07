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

#define HST_LOG_TAG "AudioEncoderFilter"

#include "audio_encoder_filter.h"
#include "osal/utils/util.h"
#include "factory/filter_factory.h"
#include "utils/steady_clock.h"

#define DEFAULT_OUT_BUFFER_POOL_SIZE 5
#define MAX_OUT_DECODED_DATA_SIZE_PER_FRAME 20 * 1024 // 20kB

namespace OHOS {
namespace Media {
namespace Pipeline {
static AutoRegisterFilter<AudioEncoderFilter> g_registerFilterHelper("builtin.recorder.audioencoder");

AudioEncoderFilter::AudioEncoderFilter(const std::string &name) : CodecFilterBase(name)
{
    filterType_ = FilterType::AUDIO_ENCODER;
    MEDIA_LOG_D("audio encoder ctor called");
}

AudioEncoderFilter::~AudioEncoderFilter()
{
    MEDIA_LOG_D("audio encoder dtor called");
    Release();
}

ErrorCode AudioEncoderFilter::Start()
{
    MEDIA_LOG_I("audio encoder start called");
    if (state_ != FilterState::READY && state_ != FilterState::PAUSED) {
        MEDIA_LOG_W("call encoder start() when state is not ready or working");
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    rb->SetActive(true);
    return FilterBase::Start();
}

ErrorCode AudioEncoderFilter::Prepare()
{
    MEDIA_LOG_I("audio encoder prepare called");
    if (state_ != FilterState::INITIALIZED) {
        MEDIA_LOG_W("encoder filter is not in init state");
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    auto err = FilterBase::Prepare();
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, "Audio Encoder prepare error because of filter base prepare error");
    return ErrorCode::SUCCESS;
}

ErrorCode AudioEncoderFilter::SetAudioEncoder(int32_t sourceId, Plugin::AudioFormat encoder)
{
    switch (encoder) {
        case Plugin::AudioFormat::AAC :
            mime_ = MEDIA_MIME_AUDIO_AAC;
            break;
        case Plugin::AudioFormat::MPEG :
            mime_ = MEDIA_MIME_AUDIO_MPEG;
            break;
        default:
            mime_ = MEDIA_MIME_AUDIO_AAC;
            break;
    }
    return ErrorCode::SUCCESS;
}

bool AudioEncoderFilter::Negotiate(const std::string& inPort,
                                   const std::shared_ptr<const Plugin::Capability>& upstreamCap,
                                   Capability& upstreamNegotiatedCap)
{
    PROFILE_BEGIN("Audio Encoder Negotiate begin");
    if (state_ != FilterState::PREPARING) {
        MEDIA_LOG_W("encoder filter is not in preparing when negotiate");
        return false;
    }

    auto targetOutPort = GetRouteOutPort(inPort);
    if (targetOutPort == nullptr) {
        MEDIA_LOG_E("encoder out port is not found");
        return false;
    }
    std::shared_ptr<Plugin::PluginInfo> selectedPluginInfo = nullptr;
    bool atLeastOutCapMatched = false;
    auto candidatePlugins = FindAvailablePlugins(*upstreamCap, Plugin::PluginType::CODEC);
    for (const auto& candidate : candidatePlugins) {
        if (candidate.first->outCaps.empty()) {
            MEDIA_LOG_E("encoder plugin must have out caps");
        }
        for (const auto& outCap : candidate.first->outCaps) { // each codec plugin should have at least one out cap
            if (outCap.mime != mime_) {
                continue;
            }
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
                MEDIA_LOG_I("choose plugin %s as working plugin", candidate.first->name.c_str());
                break;
            }
        }
        if (selectedPluginInfo != nullptr) {
            break;
        }
    }
    if (!atLeastOutCapMatched) {
        MEDIA_LOG_W("cannot find available encoder plugin");
        return false;
    }
    if (selectedPluginInfo == nullptr) {
        MEDIA_LOG_W("cannot find available downstream plugin");
        return false;
    }
    auto res = UpdateAndInitPluginByInfo<Plugin::Codec>(plugin_, pluginInfo_, selectedPluginInfo,
        [](const std::string& name)-> std::shared_ptr<Plugin::Codec> {
        return Plugin::PluginManager::Instance().CreateCodecPlugin(name);
    });
    PROFILE_END("audio encoder negotiate end");
    return res;
}

uint32_t AudioEncoderFilter::CalculateBufferSize(const std::shared_ptr<const Plugin::Meta> &meta)
{
    Plugin::ValueType value;
    if (plugin_->GetParameter(Plugin::Tag::AUDIO_SAMPLE_PER_FRAME, value) != Plugin::Status::OK ||
        value.Type() != typeid(uint32_t)) {
        MEDIA_LOG_E("Get samplePerFrame from plugin fail");
        return 0;
    }
    auto samplesPerFrame = Plugin::AnyCast<uint32_t>(value);
    uint32_t channels;
    if (!meta->GetUint32(Plugin::MetaID::AUDIO_CHANNELS, channels)) {
        return 0;
    }
    Plugin::AudioSampleFormat format;
    if (!meta->GetData<Plugin::AudioSampleFormat>(Plugin::MetaID::AUDIO_SAMPLE_FORMAT, format)) {
        return 0;
    }
    return GetBytesPerSample(format) * samplesPerFrame * channels;
}

bool AudioEncoderFilter::Configure(const std::string &inPort, const std::shared_ptr<const Plugin::Meta> &upstreamMeta)
{
    PROFILE_BEGIN("Audio encoder configure begin");
    if (plugin_ == nullptr || pluginInfo_ == nullptr) {
        MEDIA_LOG_E("cannot configure encoder when no plugin available");
        return false;
    }

//    auto thisMeta = std::make_shared<Plugin::Meta>();
//    if (!MergeMetaWithCapability(*upstreamMeta, capNegWithDownstream_, *thisMeta)) {
//        MEDIA_LOG_E("cannot configure encoder plugin since meta is not compatible with negotiated caps");
//        return false;
//    }
    auto targetOutPort = GetRouteOutPort(inPort);
    if (targetOutPort == nullptr) {
        MEDIA_LOG_E("encoder out port is not found");
        return false;
    }
    if (!targetOutPort->Configure(upstreamMeta)) {
        MEDIA_LOG_E("encoder filter downstream Configure failed");
        return false;
    }
    auto err = ConfigureToStartPluginLocked(upstreamMeta);
    if (err != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("encoder configure error");
        OnEvent({EVENT_ERROR, err});
        return false;
    }
    state_ = FilterState::READY;
    OnEvent({EVENT_READY});
    MEDIA_LOG_I("audio encoder send EVENT_READY");
    PROFILE_END("Audio encoder configure end");
    return true;
}

ErrorCode AudioEncoderFilter::ConfigureToStartPluginLocked(const std::shared_ptr<const Plugin::Meta>& meta)
{
    auto err = ConfigureWithMetaLocked(meta);
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, "configure encoder plugin error");
    err = TranslatePluginStatus(plugin_->Prepare());
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, "encoder prepare failed");
    err = TranslatePluginStatus(plugin_->Start());
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, "encoder start failed");

    uint32_t bufferCnt = 0;
    if (GetPluginParameterLocked(Tag::REQUIRED_OUT_BUFFER_CNT, bufferCnt) != ErrorCode::SUCCESS) {
        bufferCnt = DEFAULT_OUT_BUFFER_POOL_SIZE;
    }
    // 每次重新创建bufferPool
    outBufferPool_ = std::make_shared<BufferPool<AVBuffer>>(bufferCnt);
    frameSize_ = CalculateBufferSize(meta);
    if (frameSize_ == 0) {
        frameSize_ = MAX_OUT_DECODED_DATA_SIZE_PER_FRAME;
    }
    auto outAllocator = plugin_->GetAllocator();
    if (outAllocator == nullptr) {
        MEDIA_LOG_I("plugin doest not support out allocator, using framework allocator");
        outBufferPool_->Init(frameSize_);
    } else {
        MEDIA_LOG_I("using plugin output allocator");
        for (size_t cnt = 0; cnt < bufferCnt; cnt++) {
            auto buf = MemoryHelper::make_unique<AVBuffer>();
            buf->AllocMemory(outAllocator, frameSize_);
            outBufferPool_->Append(std::move(buf));
        }
    }
    rb = MemoryHelper::make_unique<Plugin::RingBuffer>(frameSize_ * 10); // 最大缓存10帧
    if (!rb) {
        MEDIA_LOG_E("create ring buffer fail");
        return ErrorCode::ERROR_NO_MEMORY;
    }
    rb->Init();
    cahceBuffer_ = std::make_shared<AVBuffer>(Plugin::BufferMetaType::AUDIO);
    auto bufferMem = cahceBuffer_->AllocMemory(NULL, frameSize_);
    if (!bufferMem) {
        MEDIA_LOG_E("alloc cache frame buffer memory fail");
        return ErrorCode::ERROR_NO_MEMORY;
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AudioEncoderFilter::PushData(const std::string &inPort, AVBufferPtr buffer, int64_t offset)
{
    const static int8_t maxRetryCnt = 3; // max retry times of handling one frame
    if (state_ != FilterState::READY && state_ != FilterState::PAUSED && state_ != FilterState::RUNNING) {
        MEDIA_LOG_W("pushing data to encoder when state is %d", static_cast<int>(state_.load()));
        return ErrorCode::ERROR_INVALID_OPERATION;
    }
    auto inputMemory = buffer->GetMemory();
    if (!inputMemory) {
        MEDIA_LOG_E("invalid buffer memory");
        return ErrorCode::ERROR_NO_MEMORY;
    }
    if (inputMemory->GetSize() > 0) {
        rb->WriteBuffer(const_cast<uint8_t *>(inputMemory->GetReadOnlyData()), inputMemory->GetSize());
    }
    auto totalSize = rb->GetSize();
    bool isEos = false;
    while ((totalSize >= frameSize_) || ((inputMemory->GetSize() == 0) && !isEos)) {
        cahceBuffer_->GetMemory()->Reset();
        cahceBuffer_->flag = 0;
        auto frmSize = (totalSize >= frameSize_) ? frameSize_ : totalSize;
        if (frmSize > 0) {
            if (rb->ReadBuffer(cahceBuffer_->GetMemory()->GetWritableData(frmSize), frmSize) != frmSize) {
                MEDIA_LOG_E("Read data from ring buffer fail");
                return ErrorCode::ERROR_UNKNOWN;
            }
        } else { // EOS
            cahceBuffer_->flag = BUFFER_FLAG_EOS;
            isEos = true;
        }
        ErrorCode handleFrameRes;
        int8_t retryCnt = 0;
        do {
            handleFrameRes = HandleFrame(cahceBuffer_);
            while (FinishFrame() == ErrorCode::SUCCESS) {
                MEDIA_LOG_D("finish frame");
            }
            retryCnt++;
            if (retryCnt >= maxRetryCnt) { // if retry cnt exceeds we will drop this frame
                break;
            }
            // if timed out or returns again we should try again
        } while (handleFrameRes == ErrorCode::ERROR_TIMED_OUT || handleFrameRes == ErrorCode::ERROR_AGAIN);
        totalSize -= frmSize;
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AudioEncoderFilter::Stop()
{
    MEDIA_LOG_I("AudioEncoderFilter stop start.");
    // 先改变底层状态 然后停掉上层线程 否则会产生死锁
    ErrorCode err ;
    if (plugin_ != nullptr) {
        err = TranslatePluginStatus(plugin_->Flush());
        RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, "encoder flush error");
        err = TranslatePluginStatus(plugin_->Stop());
        RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, "encoder stop error");
    }
    rb->SetActive(false);
    MEDIA_LOG_I("AudioEncoderFilter stop end.");
    return FilterBase::Stop();
}

ErrorCode AudioEncoderFilter::Release()
{
    if (plugin_) {
        plugin_->Stop();
        plugin_->Deinit();
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AudioEncoderFilter::HandleFrame(const std::shared_ptr<AVBuffer>& buffer)
{
    MEDIA_LOG_D("HandleFrame called");
    auto ret = TranslatePluginStatus(plugin_->QueueInputBuffer(buffer, 0));
    if (ret != ErrorCode::SUCCESS && ret != ErrorCode::ERROR_TIMED_OUT) {
        MEDIA_LOG_E("Queue input buffer to plugin fail: %d", ret);
    }
    return ret;
}

ErrorCode AudioEncoderFilter::FinishFrame()
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
        MEDIA_LOG_E("Queue out buffer to plugin fail: %d", to_underlying(ret));
        return ret;
    }
    std::shared_ptr<AVBuffer> pcmFrame = nullptr;
    auto status = plugin_->DequeueOutputBuffer(pcmFrame, 0);
    if (status != Plugin::Status::OK && status != Plugin::Status::END_OF_STREAM) {
        if (status != Plugin::Status::ERROR_NOT_ENOUGH_DATA) {
            MEDIA_LOG_E("Dequeue pcm frame from plugin fail: %d", static_cast<int32_t>(status));
        }
        return TranslatePluginStatus(status);
    }
    if (pcmFrame) {
        // push to port
        auto oPort = outPorts_[0];
        if (oPort->GetWorkMode() == WorkMode::PUSH) {
            oPort->PushData(pcmFrame, -1);
        } else {
            MEDIA_LOG_W("encoder out port works in pull mode");
        }
        pcmFrame.reset(); // 释放buffer 如果没有被缓存使其回到buffer pool 如果被sink缓存 则从buffer pool拿其他的buffer
    }
    MEDIA_LOG_D("end finish frame");
    return ErrorCode::SUCCESS;
}
} // OHOS
} // Media
} // Pipeline
#endif