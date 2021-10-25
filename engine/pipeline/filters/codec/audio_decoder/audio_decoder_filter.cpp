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

#define LOG_TAG "AudioDecoderFilter"

#include "audio_decoder_filter.h"
#include "osal/utils/util.h"
#include "utils/constants.h"
#include "utils/memory_helper.h"
#include "factory/filter_factory.h"
#include "common/plugin_utils.h"
#include "plugin/common/plugin_audio_tags.h"

namespace {
constexpr int32_t DEFAULT_OUT_BUFFER_POOL_SIZE = 5;
constexpr int32_t DEFAULT_IN_BUFFER_POOL_SIZE = 5;
constexpr int32_t MAX_OUT_DECODED_DATA_SIZE_PER_FRAME = 20 * 1024; // 20kB
constexpr int32_t AF_64BIT_BYTES = 8;
constexpr int32_t AF_32BIT_BYTES = 4;
constexpr int32_t AF_16BIT_BYTES = 2;
constexpr int32_t AF_8BIT_BYTES = 1;
constexpr int32_t RETRY_TIMES = 3;
constexpr int32_t RETRY_DELAY = 10; // 10ms

int32_t CalculateBufferSize(const std::shared_ptr<const OHOS::Media::Plugin::Meta> &meta)
{
    using namespace OHOS::Media;
    int32_t samplesPerFrame;
    if (!meta->GetInt32(Plugin::MetaID::AUDIO_SAMPLE_PRE_FRAME, samplesPerFrame)) {
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

class AudioDecoderFilter::DataCallbackImpl : public Plugin::DataCallbackHelper {
public:
    explicit DataCallbackImpl(AudioDecoderFilter& filter): decoderFilter_(filter){}

    ~DataCallbackImpl() override = default;

    void OnInputBufferDone(const std::shared_ptr<Plugin::Buffer> &input) override
    {
        decoderFilter_.OnInputBufferDone(input);
    }

    void OnOutputBufferDone(const std::shared_ptr<Plugin::Buffer> &output) override
    {
        decoderFilter_.OnOutputBufferDone(output);
    }

private:
    AudioDecoderFilter& decoderFilter_;
};

AudioDecoderFilter::AudioDecoderFilter(const std::string &name): DecoderFilterBase(name),
    dataCallback_(std::make_shared<DataCallbackImpl>(*this))
{
    MEDIA_LOG_D("audio decoder ctor called");
}

AudioDecoderFilter::~AudioDecoderFilter()
{
    MEDIA_LOG_D("audio decoder dtor called");
    Release();
    if (inBufferQ_) {
        inBufferQ_->SetActive(false);
    }
    if (outBufferQ_) {
        outBufferQ_->SetActive(false);
    }
}

ErrorCode AudioDecoderFilter::QueueAllBufferInPoolToPluginLocked()
{
    ErrorCode err = ErrorCode::SUCCESS;
    while (!outBufferPool_->Empty()) {
        auto ptr = outBufferPool_->AllocateBuffer();
        if (ptr == nullptr) {
            MEDIA_LOG_W("cannot allocate buffer in buffer pool");
            continue;
        }
        err = TranslatePluginStatus(plugin_->QueueOutputBuffer(ptr, -1));
        if (err != ErrorCode::SUCCESS) {
            MEDIA_LOG_E("queue output buffer error");
        }
    }
    return err;
}

ErrorCode AudioDecoderFilter::Start()
{
    MEDIA_LOG_D("audio decoder start called");
    if (state_ != FilterState::READY && state_ != FilterState::PAUSED) {
        MEDIA_LOG_W("call decoder start() when state is not ready or working");
        return ErrorCode::ERROR_STATE;
    }
    return FilterBase::Start();
}

ErrorCode AudioDecoderFilter::Prepare()
{
    if (state_ != FilterState::INITIALIZED) {
        MEDIA_LOG_W("decoder filter is not in init state");
        return ErrorCode::ERROR_STATE;
    }
    if (!outBufferQ_) {
        outBufferQ_ = std::make_shared<BlockingQueue<AVBufferPtr>>("adecOutBuffQueue",
                            DEFAULT_OUT_BUFFER_POOL_SIZE);
    } else {
        outBufferQ_->SetActive(true);
    }
    if (!pushTask_) {
        pushTask_ = std::make_shared<OHOS::Media::OSAL::Task>("adecPushThread");
        pushTask_->RegisterHandler([this] { FinishFrame(); });
    }
    if (drivingMode_ == ThreadDrivingMode::ASYNC) {
        if (!inBufferQ_) {
            inBufferQ_ = std::make_shared<BlockingQueue<AVBufferPtr>>("adecFilterInBufQue",
                DEFAULT_IN_BUFFER_POOL_SIZE);
        } else {
            inBufferQ_->SetActive(true);
        }
        if (!handleFrameTask_) {
            handleFrameTask_ = std::make_shared<OHOS::Media::OSAL::Task>("adecHandleFrameThread");
            handleFrameTask_->RegisterHandler([this] { HandleFrame(); });
        }
    } else {
        if (inBufferQ_) {
            inBufferQ_->SetActive(false);
            inBufferQ_.reset();
        }
        if (handleFrameTask_) {
            handleFrameTask_->Stop();
            handleFrameTask_.reset();
        }
    }
    auto err = FilterBase::Prepare();
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, "Audio Decoder prepare error because of filter base prepare error");
    return ErrorCode::SUCCESS;
}

bool AudioDecoderFilter::Negotiate(const std::string& inPort, const std::shared_ptr<const Plugin::Meta> &inMeta,
                                   CapabilitySet& outCaps)
{
    if (state_ != FilterState::PREPARING) {
        MEDIA_LOG_W("decoder filter is not in preparing when negotiate");
        return false;
    }
    auto creator = [] (const std::string& pluginName) {
        return Plugin::PluginManager::Instance().CreateCodecPlugin(pluginName);
    };
    ErrorCode err = FindPluginAndUpdate<Plugin::Codec>(inMeta, Plugin::PluginType::CODEC, plugin_,
        targetPluginInfo_, creator);

    RETURN_TARGET_ERR_MESSAGE_LOG_IF_FAIL(err, false, "cannot find matched plugin");
    outCaps = targetPluginInfo_->inCaps;
    auto targetOutPort = GetRouteOutPort(inPort);
    if (targetOutPort == nullptr) {
        MEDIA_LOG_E("decoder out port is not found");
        return false;
    }
    // todo how to decide pcm caps
    std::shared_ptr<Plugin::Meta> pcmMeta = std::make_shared<Plugin::Meta>();
    // shall we avoid copy
    pcmMeta->Update(*inMeta);
    pcmMeta->SetString(Plugin::MetaID::MIME, MEDIA_MIME_AUDIO_RAW);
    CapabilitySet sinkCaps;
    if (!targetOutPort->Negotiate(pcmMeta, sinkCaps)) {
        MEDIA_LOG_E("negotiate with sink failed");
        return false;
    }
    err = ConfigureToStartPluginLocked(inMeta);
    if (err != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("decoder configure error");
        OnEvent({EVENT_ERROR, err});
        return false;
    }

    if (drivingMode_ == ThreadDrivingMode::ASYNC && handleFrameTask_ != nullptr) {
        handleFrameTask_->Start();
    }
    pushTask_->Start();
    state_ = FilterState::READY;
    OnEvent({EVENT_READY});
    MEDIA_LOG_I("audio decoder send EVENT_READY");
    return true;
}

ErrorCode AudioDecoderFilter::ConfigureWithMetaLocked(const std::shared_ptr<const Plugin::Meta> &meta)
{
    uint32_t channels;
    if (meta->GetUint32(Plugin::MetaID::AUDIO_CHANNELS, channels)) {
        MEDIA_LOG_D("found audio channel meta");
        SetPluginParameterLocked(Tag::AUDIO_CHANNELS, channels);
    }
    uint32_t sampleRate;
    if (meta->GetUint32(Plugin::MetaID::AUDIO_SAMPLE_RATE, sampleRate)) {
        MEDIA_LOG_D("found audio sample rate meta");
        SetPluginParameterLocked(Tag::AUDIO_SAMPLE_RATE, sampleRate);
    }
    int64_t bitRate;
    if (meta->GetInt64(Plugin::MetaID::MEDIA_BITRATE, bitRate)) {
        MEDIA_LOG_D("found audio bit rate meta");
        SetPluginParameterLocked(Tag::MEDIA_BITRATE, bitRate);
    }
    auto audioFormat = Plugin::AudioSampleFormat::U8;
    if (meta->GetData<Plugin::AudioSampleFormat>(Plugin::MetaID::AUDIO_SAMPLE_FORMAT, audioFormat)) {
        SetPluginParameterLocked(Tag::AUDIO_SAMPLE_FORMAT, audioFormat);
    }
    std::vector<uint8_t> codecConfig;
    if (meta->GetData<std::vector<uint8_t>>(Plugin::MetaID::MEDIA_CODEC_CONFIG, codecConfig)) {
        SetPluginParameterLocked(Tag::MEDIA_CODEC_CONFIG, std::move(codecConfig));
    }
    return ErrorCode::SUCCESS;
}

ErrorCode AudioDecoderFilter::ConfigureToStartPluginLocked(const std::shared_ptr<const Plugin::Meta>& meta)
{
    auto err = TranslatePluginStatus(plugin_->SetDataCallback(dataCallback_));
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, "set decoder plugin callback failed");
    err = TranslatePluginStatus(plugin_->Init());
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, "decoder plugin init error");

    err = ConfigureWithMetaLocked(meta);
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
    err = QueueAllBufferInPoolToPluginLocked();
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, "queue out buffer to plugin failed");

    err = TranslatePluginStatus(plugin_->Start());
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, "decoder start failed");

    return ErrorCode::SUCCESS;
}

ErrorCode AudioDecoderFilter::PushData(const std::string &inPort, AVBufferPtr buffer)
{
    if (state_ != FilterState::READY && state_ != FilterState::PAUSED && state_ != FilterState::RUNNING) {
        MEDIA_LOG_W("pushing data to decoder when state is %d", static_cast<int>(state_.load()));
        return ErrorCode::ERROR_STATE;
    }
    if (isFlushing_) {
        MEDIA_LOG_I("decoder is flushing, discarding this data from port %s", inPort.c_str());
        return ErrorCode::SUCCESS;
    }
    // async
    if (drivingMode_ == ThreadDrivingMode::ASYNC) {
        inBufferQ_->Push(buffer);
        return ErrorCode::SUCCESS;
    }
    // sync
    HandleOneFrame(buffer);
    return ErrorCode::SUCCESS;
}

void AudioDecoderFilter::FlushStart()
{
    MEDIA_LOG_I("FlushStart entered.");
    isFlushing_ = true;
    if (inBufferQ_) {
        inBufferQ_->SetActive(false);
    }
    handleFrameTask_->PauseAsync();
    if (outBufferQ_) {
        outBufferQ_->SetActive(false);
    }
    pushTask_->PauseAsync();
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
    if (inBufferQ_) {
        inBufferQ_->SetActive(true);
    }
    handleFrameTask_->Start();
    if (outBufferQ_) {
        outBufferQ_->SetActive(true);
    }
    pushTask_->Start();
    if (plugin_ != nullptr) {
        QueueAllBufferInPoolToPluginLocked();
    }
}

ErrorCode AudioDecoderFilter::Stop()
{
    MEDIA_LOG_I("AudioDecoderFilter stop start.");
    // 先改变底层状态 然后停掉上层线程 否则会产生死锁
    auto err = TranslatePluginStatus(plugin_->Flush());
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, "decoder flush error");
    err = TranslatePluginStatus(plugin_->Stop());
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(err, "decoder stop error");
    outBufferQ_->SetActive(false);
    pushTask_->Pause();

    if (drivingMode_ == ThreadDrivingMode::ASYNC && handleFrameTask_ != nullptr) {
        inBufferQ_->SetActive(false);
        handleFrameTask_->Pause();
    }

    outBufferPool_.reset();
    MEDIA_LOG_I("AudioDecoderFilter stop end.");
    return FilterBase::Stop();
}

ErrorCode AudioDecoderFilter::Release()
{
    if (plugin_) {
        plugin_->Stop();
        plugin_->Deinit();
    }

    if (drivingMode_ == ThreadDrivingMode::ASYNC && handleFrameTask_ != nullptr) {
        handleFrameTask_->Stop();
        handleFrameTask_.reset();
    }

    if (inBufferQ_ != nullptr) {
        inBufferQ_->SetActive(false);
        inBufferQ_.reset();
    }
    // 先停止线程 然后释放bufferQ 如果顺序反过来 可能导致线程访问已经释放的锁
    if (pushTask_ != nullptr) {
        pushTask_->Stop();
        pushTask_.reset();
    }

    if (outBufferQ_ != nullptr) {
        outBufferQ_->SetActive(false);
        outBufferQ_.reset();
    }
    return ErrorCode::SUCCESS;
}

void AudioDecoderFilter::HandleFrame()
{
    MEDIA_LOG_D("HandleFrame called");
    auto oneBuffer = inBufferQ_->Pop();
    if (oneBuffer == nullptr) {
        MEDIA_LOG_W("decoder find nullptr in esBufferQ");
        return;
    }
    HandleOneFrame(oneBuffer);
    MEDIA_LOG_D("HandleFrame finished");
}

void AudioDecoderFilter::HandleOneFrame(const std::shared_ptr<AVBuffer>& data)
{
    MEDIA_LOG_D("HandleOneFrame called");
    Plugin::Status status = Plugin::Status::OK;
    int32_t retryCnt = 0;
    do {
        status = plugin_->QueueInputBuffer(data, 0);
        if (status != Plugin::Status::ERROR_TIMED_OUT) {
            break;
        }
        MEDIA_LOG_I("queue input buffer timeout, will retry");
        retryCnt++;
        if (retryCnt <= RETRY_TIMES) {
            OSAL::SleepFor(RETRY_DELAY);
        } else {
            break;
        }
    } while (true);
    if (status != Plugin::Status::OK) {
        MEDIA_LOG_W("queue input buffer with error %d, ignore this buffer", status);
    }
    MEDIA_LOG_D("HandleOneFrame finished");
}

void AudioDecoderFilter::FinishFrame()
{
    MEDIA_LOG_D("begin finish frame");
    // get buffer from plugin
    auto ptr = outBufferQ_->Pop();
    if (ptr) {
        // push to port
        auto oPort = outPorts_[0];
        if (oPort->GetWorkMode() == WorkMode::PUSH) {
            oPort->PushData(ptr);
        } else {
            MEDIA_LOG_W("decoder out port works in pull mode");
        }
        ptr.reset(); // 释放buffer 如果没有被缓存使其回到buffer pool 如果被sink缓存 则从buffer pool拿其他的buffer
        auto oPtr = outBufferPool_->AllocateBuffer();
        if (oPtr != nullptr) {
            oPtr->Reset();
            plugin_->QueueOutputBuffer(oPtr, -1);
        }
    }
    MEDIA_LOG_D("end finish frame");
}

void AudioDecoderFilter::OnInputBufferDone(const std::shared_ptr<AVBuffer> &buffer)
{
    // do nothing since we has no input buffer pool
}

void AudioDecoderFilter::OnOutputBufferDone(const std::shared_ptr<AVBuffer> &buffer)
{
    outBufferQ_->Push(buffer);
}
}
}
}
