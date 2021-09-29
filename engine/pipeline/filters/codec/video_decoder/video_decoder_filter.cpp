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

#ifdef VIDEO_SUPPORT

#define LOG_TAG "VideoDecoderFilter"

#include "video_decoder_filter.h"
#include "utils/util.h"
#include "foundation/constants.h"
#include "factory/filter_factory.h"
#include "plugin/common/plugin_video_tags.h"
#include "foundation/memory_helper.h"
#include "foundation/log.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
const uint32_t DEFAULT_IN_BUFFER_POOL_SIZE = 200;
const uint32_t DEFAULT_OUT_BUFFER_POOL_SIZE = 8;
const float VIDEO_PIX_DEPTH = 1.5;
static uint32_t VIDEO_ALIGN_SIZE = 16;

template <typename T, typename U>
static constexpr T AlignUp(T num, U alignment)
{
    return (alignment > 0) ? ((num + static_cast<T>(alignment) - 1) & (~(static_cast<T>(alignment) - 1))) : num;
}

static AutoRegisterFilter<VideoDecoderFilter> g_registerFilterHelper("builtin.player.videodecoder");

class VideoDecoderFilter::DataCallbackImpl : public Plugin::DataCallbackHelper {
public:
    explicit DataCallbackImpl(VideoDecoderFilter& filter): decFilter_(filter)
    {
    }

    ~DataCallbackImpl() override = default;

    void OnInputBufferDone(const std::shared_ptr<Plugin::Buffer> &input) override
    {
        decFilter_.OnInputBufferDone(input);
    }

    void OnOutputBufferDone(const std::shared_ptr<Plugin::Buffer> &output) override
    {
        decFilter_.OnOutputBufferDone(output);
    }

private:
    VideoDecoderFilter& decFilter_;
};

VideoDecoderFilter::VideoDecoderFilter(const std::string &name): DecoderFilterBase(name),
    dataCallback_(std::make_shared<DataCallbackImpl>(*this))
{
    MEDIA_LOG_I("video decoder ctor called");
    vdecFormat_.width = 0;
    vdecFormat_.height = 0;
    vdecFormat_.bitRate = -1;
}

VideoDecoderFilter::~VideoDecoderFilter()
{
    MEDIA_LOG_I("video decoder dtor called");
    if (plugin_) {
        plugin_->Stop();
        plugin_->Deinit();
    }
    if (drivingMode_ == ThreadDrivingMode::ASYNC && handleFrameTask_) {
        handleFrameTask_->Stop();
        handleFrameTask_.reset();
    }
    if (inBufQue_) {
        inBufQue_->SetActive(false);
        inBufQue_.reset();
    }
    if (pushTask_) {
        pushTask_->Stop();
        pushTask_.reset();
    }
    if (outBufQue_) {
        outBufQue_->SetActive(false);
        outBufQue_.reset();
    }
}

ErrorCode VideoDecoderFilter::Start()
{
    MEDIA_LOG_D("video decoder start called");
    if (state_ != FilterState::READY && state_ != FilterState::PAUSED) {
        MEDIA_LOG_W("call decoder start() when state_ is not ready or working");
        return ERROR_STATE;
    }
    return FilterBase::Start();
}

ErrorCode VideoDecoderFilter::Prepare()
{
    MEDIA_LOG_D("video decoder prepare called");
    if (state_ != FilterState::INITIALIZED) {
        MEDIA_LOG_W("decoder filter is not in init state_");
        return ERROR_STATE;
    }
    if (!outBufQue_) {
        outBufQue_ = std::make_shared<BlockingQueue<AVBufferPtr>>("vdecFilterOutBufQue",
                                                                  DEFAULT_OUT_BUFFER_POOL_SIZE);
    } else {
        outBufQue_->SetActive(true);
    }
    if (!pushTask_) {
        pushTask_ = std::make_shared<OHOS::Media::OSAL::Task>("vdecPushThread");
        pushTask_->RegisterHandler([this] { FinishFrame(); });
    }
    if (!inBufQue_) {
        inBufQue_ = std::make_shared<BlockingQueue<AVBufferPtr>>("vdecFilterInBufQue",
                                                                 DEFAULT_IN_BUFFER_POOL_SIZE);
    } else {
        inBufQue_->SetActive(true);
    }
    if (drivingMode_ == ThreadDrivingMode::ASYNC && !handleFrameTask_) {
        handleFrameTask_ = std::make_shared<OHOS::Media::OSAL::Task>("decHandleFrameThread");
        handleFrameTask_->RegisterHandler([this] { HandleFrame(); });
    }
    return FilterBase::Prepare();
}

bool VideoDecoderFilter::Negotiate(const std::string& inPort, const std::shared_ptr<const Meta> &inMeta,
                                   CapabilitySet& outCaps)
{
    if (state_ != FilterState::PREPARING) {
        MEDIA_LOG_W("decoder filter is not in preparing when negotiate");
        return false;
    }

    MEDIA_LOG_D("video decoder negotiate called");
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
    if (Configure(inMeta) != ErrorCode::SUCCESS) {
        MEDIA_LOG_E("decoder configure error");
        Event event {
            .type = EVENT_ERROR,
            .param = err,
        };
        OnEvent(event);
        return false;
    }
    std::shared_ptr<Meta> videoMeta = std::make_shared<Meta>();
    videoMeta->Update(*inMeta);
    videoMeta->SetString(Media::Plugin::MetaID::MIME, MEDIA_MIME_VIDEO_RAW);
    videoMeta->SetUint32(Media::Plugin::MetaID::VIDEO_PIXEL_FORMAT, vdecFormat_.format);
    CapabilitySet sinkCaps;
    if (!targetOutPort->Negotiate(videoMeta, sinkCaps)) {
        MEDIA_LOG_E("negotiate with sink failed");
        return false;
    }
    return true;
}

ErrorCode VideoDecoderFilter::AllocateOutputBuffers()
{
    uint32_t bufferCnt = 0;
    if (GetPluginParameterLocked(Tag::REQUIRED_OUT_BUFFER_CNT, bufferCnt) != ErrorCode::SUCCESS) {
        bufferCnt = DEFAULT_OUT_BUFFER_POOL_SIZE;
    }
    outBufPool_ = std::make_shared<BufferPool<AVBuffer>>(bufferCnt);
    // YUV420: size = stride * height * 1.5
    uint32_t bufferSize = 0;
    uint32_t stride = AlignUp(vdecFormat_.width, VIDEO_ALIGN_SIZE);
    if ((vdecFormat_.format == static_cast<uint32_t>(Plugin::VideoPixelFormat::YUV420P)) ||
        (vdecFormat_.format == static_cast<uint32_t>(Plugin::VideoPixelFormat::NV21)) ||
        (vdecFormat_.format == static_cast<uint32_t>(Plugin::VideoPixelFormat::NV12))) {
        bufferSize = static_cast<uint32_t>(AlignUp(stride, VIDEO_ALIGN_SIZE) *
                        AlignUp(vdecFormat_.height, VIDEO_ALIGN_SIZE) * VIDEO_PIX_DEPTH);
        MEDIA_LOG_D("Output buffer size: %u", bufferSize);
    } else {
        // need to check video sink support and calc buffer size
        MEDIA_LOG_E("Unsupported video pixel format: %d", vdecFormat_.format);
        return ErrorCode::UNIMPLEMENT;
    }
    auto outAllocator = plugin_->GetAllocator(); // zero copy need change to use sink allocator
    if (outAllocator == nullptr) {
        MEDIA_LOG_I("plugin doest not support out allocator, using framework allocator");
        outBufPool_->Init(bufferSize, Plugin::BufferMetaType::VIDEO);
    } else {
        MEDIA_LOG_I("using plugin output allocator");
        for (size_t cnt = 0; cnt < bufferCnt; cnt++) {
            auto buf = MemoryHelper::make_unique<AVBuffer>(Plugin::BufferMetaType::VIDEO);
            buf->AllocMemory(outAllocator, bufferSize);
            outBufPool_->Append(std::move(buf));
        }
    }
    return ErrorCode::SUCCESS;
}

ErrorCode VideoDecoderFilter::SetVideoDecoderFormat(const std::shared_ptr<const Meta>& meta)
{
    vdecFormat_.format = static_cast<uint32_t>(Plugin::VideoPixelFormat::NV12);
    if (!meta->GetString(MetaID::MIME, vdecFormat_.mime)) {
        return ErrorCode::INVALID_PARAM_VALUE;
    }
    if (!meta->GetUint32(MetaID::VIDEO_WIDTH, vdecFormat_.width)) {
        return ErrorCode::INVALID_PARAM_VALUE;
    }
    if (!meta->GetUint32(MetaID::VIDEO_HEIGHT, vdecFormat_.height)) {
        return ErrorCode::INVALID_PARAM_VALUE;
    }
    if (!meta->GetInt64(MetaID::MEDIA_BITRATE, vdecFormat_.bitRate)) {
        MEDIA_LOG_D("Do not have codec bit rate");
    }
    // Optional: codec extra data
    if (!meta->GetData<std::vector<uint8_t>>(MetaID::MEDIA_CODEC_CONFIG, vdecFormat_.codecConfig)) {
        MEDIA_LOG_D("Do not have codec extra data");
    }
    return ErrorCode::SUCCESS;
}

ErrorCode VideoDecoderFilter::ConfigurePluginParams()
{
    if (SetPluginParameterLocked(Tag::MIME, vdecFormat_.mime) != ErrorCode::SUCCESS) {
        MEDIA_LOG_W("Set mime to plugin fail");
        return ErrorCode::UNKNOWN_ERROR;
    }
    if (SetPluginParameterLocked(Tag::VIDEO_WIDTH, vdecFormat_.width) != ErrorCode::SUCCESS) {
        MEDIA_LOG_W("Set width to plugin fail");
        return ErrorCode::UNKNOWN_ERROR;
    }
    if (SetPluginParameterLocked(Tag::VIDEO_HEIGHT, vdecFormat_.height) != ErrorCode::SUCCESS) {
        MEDIA_LOG_W("Set height to plugin fail");
        return ErrorCode::UNKNOWN_ERROR;
    }
    if (SetPluginParameterLocked(Tag::VIDEO_PIXEL_FORMAT, vdecFormat_.format) != ErrorCode::SUCCESS) {
        MEDIA_LOG_W("Set pixel format to plugin fail");
        return ErrorCode::UNKNOWN_ERROR;
    }
    if (vdecFormat_.bitRate != -1) {
        if (SetPluginParameterLocked(Tag::MEDIA_BITRATE, vdecFormat_.bitRate) != ErrorCode::SUCCESS) {
            MEDIA_LOG_W("Set bitrate to plugin fail");
        }
    }
    // Optional: codec extra data
    if (vdecFormat_.codecConfig.size() > 0) {
        if (SetPluginParameterLocked(Tag::MEDIA_CODEC_CONFIG, std::move(vdecFormat_.codecConfig)) !=
            ErrorCode::SUCCESS) {
            MEDIA_LOG_W("Set bitrate to plugin fail");
        }
    }
    MEDIA_LOG_D("ConfigurePluginParams success, mime: %s, width: %u, height: %u, format: %u, bitRate: %u",
                vdecFormat_.mime.c_str(), vdecFormat_.width, vdecFormat_.height, vdecFormat_.format,
                vdecFormat_.bitRate);
    return ErrorCode::SUCCESS;
}

ErrorCode VideoDecoderFilter::ConfigurePluginOutputBuffers()
{
    ErrorCode err = ErrorCode::SUCCESS;
    while (!outBufPool_->Empty()) {
        auto ptr = outBufPool_->AllocateBuffer();
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

ErrorCode VideoDecoderFilter::InitPlugin()
{
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(TranslatePluginStatus(plugin_->SetDataCallback(dataCallback_)),
                                   "Set plugin callback fail");
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(TranslatePluginStatus(plugin_->Init()), "Init plugin error");
    return ErrorCode::SUCCESS;
}

ErrorCode VideoDecoderFilter::ConfigurePlugin()
{
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(ConfigurePluginParams(), "Configure plugin params error");
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(ConfigurePluginOutputBuffers(), "Configure plugin output buffers error");
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(TranslatePluginStatus(plugin_->Prepare()), "Prepare plugin fail");
    return TranslatePluginStatus(plugin_->Start());
}

ErrorCode VideoDecoderFilter::Configure(const std::shared_ptr<const Meta>& meta)
{
    MEDIA_LOG_D("video decoder configure called");
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(InitPlugin(), "Init plugin fail");
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(SetVideoDecoderFormat(meta), "Set video decoder format fail");
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(AllocateOutputBuffers(), "Alloc output buffers fail");
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(ConfigurePlugin(), "Config plugin fail");
    if (drivingMode_ == ThreadDrivingMode::ASYNC && handleFrameTask_) {
        handleFrameTask_->Start();
    }
    if (pushTask_) {
        pushTask_->Start();
    }
    state_ = FilterState::READY;
    Event event {
        .type = EVENT_READY,
    };
    OnEvent(event);
    MEDIA_LOG_I("video decoder send EVENT_READY");
    return ErrorCode::SUCCESS;
}

ErrorCode VideoDecoderFilter::PushData(const std::string& inPort, AVBufferPtr buffer)
{
    if (state_ != FilterState::READY && state_ != FilterState::PAUSED && state_ != FilterState::RUNNING) {
        MEDIA_LOG_W("pushing data to decoder when state_ is %d", static_cast<int>(state_.load()));
        return ErrorCode::ERROR_STATE;
    }
    if (isFlushing_) {
        MEDIA_LOG_I("decoder is flushing, discarding this data from port %s", inPort.c_str());
        return ErrorCode::SUCCESS;
    }
    if (drivingMode_ == ThreadDrivingMode::ASYNC) {
        inBufQue_->Push(buffer);
        return ErrorCode::SUCCESS;
    }
    HandleOneFrame(buffer);
    return ErrorCode::SUCCESS;
}

void VideoDecoderFilter::FlushStart()
{
    MEDIA_LOG_I("FlushStart entered");
    isFlushing_ = true;
    if (inBufQue_) {
        inBufQue_->SetActive(false);
    }
    if (drivingMode_ == ThreadDrivingMode::ASYNC && handleFrameTask_) {
        handleFrameTask_->PauseAsync();
    }
    if (outBufQue_) {
        outBufQue_->SetActive(false);
    }
    if (pushTask_) {
        pushTask_->PauseAsync();
    }
    if (plugin_ != nullptr) {
        auto err = TranslatePluginStatus(plugin_->Flush());
        if (err != SUCCESS) {
            MEDIA_LOG_E("decoder plugin flush error");
        }
    }
}

void VideoDecoderFilter::FlushEnd()
{
    MEDIA_LOG_I("FlushEnd entered");
    isFlushing_ = false;
    if (inBufQue_) {
        inBufQue_->SetActive(true);
    }
    if (drivingMode_ == ThreadDrivingMode::ASYNC && handleFrameTask_) {
        handleFrameTask_->Start();
    }
    if (outBufQue_) {
        outBufQue_->SetActive(true);
    }
    if (pushTask_) {
        pushTask_->Start();
    }
    if (plugin_) {
        ConfigurePluginOutputBuffers();
    }
}

ErrorCode VideoDecoderFilter::Stop()
{
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(TranslatePluginStatus(plugin_->Flush()), "Flush plugin fail");
    RETURN_ERR_MESSAGE_LOG_IF_FAIL(TranslatePluginStatus(plugin_->Stop()), "Stop plugin fail");
    outBufQue_->SetActive(false);
    pushTask_->Pause();
    inBufQue_->SetActive(false);
    if (drivingMode_ == ThreadDrivingMode::ASYNC && handleFrameTask_) {
        handleFrameTask_->Pause();
    }
    outBufPool_.reset();
    MEDIA_LOG_I("Stop success");
    return FilterBase::Stop();
}

void VideoDecoderFilter::HandleFrame()
{
    MEDIA_LOG_D("HandleFrame called");
    auto oneBuffer = inBufQue_->Pop();
    if (oneBuffer == nullptr) {
        MEDIA_LOG_W("decoder find nullptr in esBufferQ");
        return;
    }
    HandleOneFrame(oneBuffer);
}

void VideoDecoderFilter::HandleOneFrame(const std::shared_ptr<AVBuffer>& data)
{
    MEDIA_LOG_D("HandleOneFrame called");
    Plugin::Status ret;
    do {
        ret = plugin_->QueueInputBuffer(data, -1);
        if (ret == Plugin::Status::OK) {
            break;
        }
        MEDIA_LOG_D("Send data to plugin error: %d", ret);
        OSAL::SleepFor(10);
    } while (1);
}

void VideoDecoderFilter::FinishFrame()
{
    MEDIA_LOG_D("begin finish frame");
    auto ptr = outBufQue_->Pop();
    if (ptr) {
        auto oPort = outPorts_[0];
        if (oPort->GetWorkMode() == WorkMode::PUSH) {
            oPort->PushData(ptr);
        } else {
            MEDIA_LOG_W("decoder out port works in pull mode");
        }
        ptr.reset();
        auto oPtr = outBufPool_->AllocateBuffer();
        if (oPtr != nullptr) {
            oPtr->Reset();
            plugin_->QueueOutputBuffer(oPtr, 0);
        }
    }
    MEDIA_LOG_D("end finish frame");
}

void VideoDecoderFilter::OnInputBufferDone(const std::shared_ptr<AVBuffer> &buffer)
{
    // do nothing since we has no input buffer pool
}

void VideoDecoderFilter::OnOutputBufferDone(const std::shared_ptr<AVBuffer> &buffer)
{
    outBufQue_->Push(buffer);
}
}
}
}
#endif
