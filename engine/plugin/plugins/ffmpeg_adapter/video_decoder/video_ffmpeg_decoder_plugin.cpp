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

#define HST_LOG_TAG "Ffmpeg_Video_Decoder"

#include "video_ffmpeg_decoder_plugin.h"
#include <cstring>
#include <map>
#include <set>
#include "foundation/log.h"
#include "utils/constants.h"
#include "utils/memory_helper.h"
#include "plugin/common/plugin_buffer.h"
#include "plugin/common/plugin_video_tags.h"
#include "plugin/interface/codec_plugin.h"
#include "plugins/ffmpeg_adapter/utils/ffmpeg_utils.h"

namespace {
// register plugins
using namespace OHOS::Media::Plugin;
void UpdatePluginDefinition(const AVCodec* codec, CodecPluginDef& definition);

std::map<std::string, std::shared_ptr<const AVCodec>> codecMap;

const size_t BUFFER_QUEUE_SIZE = 6;

std::set<AVCodecID> supportedCodec = {AV_CODEC_ID_H264};

std::shared_ptr<CodecPlugin> VideoFfmpegDecoderCreator(const std::string& name)
{
    return std::make_shared<VideoFfmpegDecoderPlugin>(name);
}

Status RegisterVideoDecoderPlugins(const std::shared_ptr<Register>& reg)
{
    const AVCodec* codec = nullptr;
    void* iter = nullptr;
    MEDIA_LOG_I("registering video decoders");
    while ((codec = av_codec_iterate(&iter))) {
        if (!av_codec_is_decoder(codec) || codec->type != AVMEDIA_TYPE_VIDEO) {
            continue;
        }
        if (supportedCodec.find(codec->id) == supportedCodec.end()) {
            MEDIA_LOG_D("codec %s(%s) is not supported right now", codec->name, codec->long_name);
            continue;
        }
        CodecPluginDef definition;
        definition.name = "videodecoder_" + std::string(codec->name);
        definition.codecType = CodecType::VIDEO_DECODER;
        definition.rank = 100; // 100
        definition.creator = VideoFfmpegDecoderCreator;
        UpdatePluginDefinition(codec, definition);
        // do not delete the codec in the deleter
        codecMap[definition.name] = std::shared_ptr<AVCodec>(const_cast<AVCodec*>(codec), [](void* ptr) {});
        if (reg->AddPlugin(definition) != Status::OK) {
            MEDIA_LOG_W("register plugin %s(%s) failed", codec->name, codec->long_name);
        }
    }
    return Status::OK;
}

void UnRegisterVideoDecoderPlugins()
{
    codecMap.clear();
}

void UpdatePluginDefinition(const AVCodec* codec, CodecPluginDef& definition)
{
    Capability inputCaps("video/unknown");
    switch (codec->id) {
        case AV_CODEC_ID_H264:
            inputCaps.SetMime(OHOS::Media::MEDIA_MIME_VIDEO_AVC);
            break;
        default:
            MEDIA_LOG_I("codec is not supported right now");
            break;
    }
    definition.inCaps.push_back(inputCaps);

    Capability outputCaps(OHOS::Media::MEDIA_MIME_VIDEO_RAW);
    outputCaps.AppendDiscreteKeys<VideoPixelFormat>(
        Capability::Key::VIDEO_PIXEL_FORMAT,
        {VideoPixelFormat::YUV420P, VideoPixelFormat::NV12, VideoPixelFormat::NV21});
    MEDIA_LOG_E("Capability VIDEO_PIXEL_FORMAT: %u", Capability::Key::VIDEO_PIXEL_FORMAT);
    definition.outCaps.push_back(outputCaps);
}
} // namespace

PLUGIN_DEFINITION(FFmpegVideoDecoders, LicenseType::LGPL, RegisterVideoDecoderPlugins, UnRegisterVideoDecoderPlugins);

namespace OHOS {
namespace Media {
namespace Plugin {
namespace {
std::map<AVPixelFormat, VideoPixelFormat> g_pixelFormatMap = {
    {AV_PIX_FMT_YUV420P, VideoPixelFormat::YUV420P},     {AV_PIX_FMT_YUYV422, VideoPixelFormat::YUV420P},
    {AV_PIX_FMT_RGB24, VideoPixelFormat::RGB24},         {AV_PIX_FMT_BGR24, VideoPixelFormat::BGR24},
    {AV_PIX_FMT_YUV422P, VideoPixelFormat::YUV422P},     {AV_PIX_FMT_YUV444P, VideoPixelFormat::YUV444P},
    {AV_PIX_FMT_YUV410P, VideoPixelFormat::YUV410P},     {AV_PIX_FMT_YUV411P, VideoPixelFormat::YUV411P},
    {AV_PIX_FMT_GRAY8, VideoPixelFormat::GRAY8},         {AV_PIX_FMT_MONOWHITE, VideoPixelFormat::MONOWHITE},
    {AV_PIX_FMT_MONOBLACK, VideoPixelFormat::MONOBLACK}, {AV_PIX_FMT_PAL8, VideoPixelFormat::PAL8},
    {AV_PIX_FMT_YUVJ420P, VideoPixelFormat::YUVJ420P},   {AV_PIX_FMT_YUVJ422P, VideoPixelFormat::YUVJ422P},
    {AV_PIX_FMT_YUVJ444P, VideoPixelFormat::YUVJ444P},   {AV_PIX_FMT_NV12, VideoPixelFormat::NV12},
    {AV_PIX_FMT_NV21, VideoPixelFormat::NV21},
};
} // namespace

VideoFfmpegDecoderPlugin::VideoFfmpegDecoderPlugin(std::string name)
    : CodecPlugin(std::move(name)), outBufferQ_("vdecPluginQueue", BUFFER_QUEUE_SIZE)
{
}

Status VideoFfmpegDecoderPlugin::Init()
{
    OSAL::ScopedLock l(avMutex_);
    auto iter = codecMap.find(pluginName_);
    if (iter == codecMap.end()) {
        MEDIA_LOG_W("cannot find codec with name %s", pluginName_.c_str());
        return Status::ERROR_UNSUPPORTED_FORMAT;
    }
    avCodec_ = iter->second;
    cachedFrame_ = std::shared_ptr<AVFrame>(av_frame_alloc(), [](AVFrame* fp) { av_frame_free(&fp); });
    videoDecParams_[Tag::REQUIRED_OUT_BUFFER_CNT] = (uint32_t)BUFFER_QUEUE_SIZE;
    if (!decodeTask_) {
        decodeTask_ = std::make_shared<OHOS::Media::OSAL::Task>("videoFfmpegDecThread");
        decodeTask_->RegisterHandler([this] { ReceiveBuffer(); });
    }
    state_ = State::INITIALIZED;
    MEDIA_LOG_I("Init success");
    return Status::OK;
}

Status VideoFfmpegDecoderPlugin::Deinit()
{
    OSAL::ScopedLock l(avMutex_);
    avCodec_.reset();
    cachedFrame_.reset();
    ResetLocked();
    if (decodeTask_) {
        decodeTask_->Stop();
        decodeTask_.reset();
    }
    state_ = State::DESTROYED;
    return Status::OK;
}

Status VideoFfmpegDecoderPlugin::SetParameter(Tag tag, const ValueType& value)
{
    OSAL::ScopedLock l(avMutex_);
    videoDecParams_.insert(std::make_pair(tag, value));
    return Status::OK;
}

Status VideoFfmpegDecoderPlugin::GetParameter(Tag tag, ValueType& value)
{
    OSAL::ScopedLock l(avMutex_);
    auto res = videoDecParams_.find(tag);
    if (res != videoDecParams_.end()) {
        value = res->second;
        return Status::OK;
    }
    return Status::ERROR_INVALID_PARAMETER;
}

template <typename T>
void VideoFfmpegDecoderPlugin::FindInParameterMapThenAssignLocked(Tag tag, T& assign)
{
    auto iter = videoDecParams_.find(tag);
    if (iter != videoDecParams_.end() && typeid(T) == iter->second.Type()) {
        assign = Plugin::AnyCast<T>(iter->second);
    } else {
        MEDIA_LOG_W("parameter %d is not found or type mismatch", static_cast<int32_t>(tag));
    }
}

Status VideoFfmpegDecoderPlugin::CreateCodecContext()
{
    auto context = avcodec_alloc_context3(avCodec_.get());
    if (context == nullptr) {
        MEDIA_LOG_E("cannot allocate codec context");
        return Status::ERROR_UNKNOWN;
    }
    avCodecContext_ = std::shared_ptr<AVCodecContext>(context, [](AVCodecContext* ptr) {
        if (ptr != nullptr) {
            if (ptr->extradata) {
                av_free(ptr->extradata);
                ptr->extradata = nullptr;
            }
            avcodec_free_context(&ptr);
        }
    });
    MEDIA_LOG_I("Create ffmpeg codec context success");
    return Status::OK;
}

void VideoFfmpegDecoderPlugin::InitCodecContext()
{
    avCodecContext_->codec_type = AVMEDIA_TYPE_VIDEO;
    FindInParameterMapThenAssignLocked<int64_t>(Tag::MEDIA_BITRATE, avCodecContext_->bit_rate);
    FindInParameterMapThenAssignLocked<std::uint32_t>(Tag::VIDEO_WIDTH, width_);
    FindInParameterMapThenAssignLocked<std::uint32_t>(Tag::VIDEO_HEIGHT, height_);
    FindInParameterMapThenAssignLocked<Plugin::VideoPixelFormat>(Tag::VIDEO_PIXEL_FORMAT, pixelFormat_);
    MEDIA_LOG_D("bitRate: %" PRId64 ", width: %u, height: %u, pixelFormat: %u", avCodecContext_->bit_rate, width_,
                height_, pixelFormat_);
    SetCodecExtraData();
    // Reset coded_width/_height to prevent it being reused from last time when
    // the codec is opened again, causing a mismatch and possible segfault/corruption.
    avCodecContext_->coded_width = 0;
    avCodecContext_->coded_height = 0;
    avCodecContext_->workaround_bugs |= FF_BUG_AUTODETECT;
    avCodecContext_->err_recognition = 1;
}

void VideoFfmpegDecoderPlugin::DeinitCodecContext()
{
    if (avCodecContext_ == nullptr) {
        return;
    }
    if (avCodecContext_->extradata) {
        av_free(avCodecContext_->extradata);
        avCodecContext_->extradata = nullptr;
    }
    avCodecContext_->extradata_size = 0;
    avCodecContext_->opaque = nullptr;
    avCodecContext_->width = 0;
    avCodecContext_->height = 0;
    avCodecContext_->coded_width = 0;
    avCodecContext_->coded_height = 0;
    avCodecContext_->time_base.den = 0;
    avCodecContext_->time_base.num = 0;
    avCodecContext_->ticks_per_frame = 0;
    avCodecContext_->sample_aspect_ratio.num = 0;
    avCodecContext_->sample_aspect_ratio.den = 0;
    avCodecContext_->get_buffer2 = nullptr;
}

void VideoFfmpegDecoderPlugin::SetCodecExtraData()
{
    auto iter = videoDecParams_.find(Tag::MEDIA_CODEC_CONFIG);
    if (iter == videoDecParams_.end() || iter->second.Type() != typeid(std::vector<uint8_t>)) {
        return;
    }
    auto codecConfig = Plugin::AnyCast<std::vector<uint8_t>>(iter->second);
    int configSize = codecConfig.size();
    if (configSize > 0) {
        auto allocSize = AlignUp(configSize + AV_INPUT_BUFFER_PADDING_SIZE, 16); // 16
        avCodecContext_->extradata = static_cast<uint8_t*>(av_mallocz(allocSize));
        (void)memcpy_s(avCodecContext_->extradata, configSize, codecConfig.data(), configSize);
        avCodecContext_->extradata_size = configSize;
        MEDIA_LOG_I("Set Codec Extra Data success");
    }
}

Status VideoFfmpegDecoderPlugin::OpenCodecContext()
{
    AVCodec* vdec = avcodec_find_decoder(avCodecContext_->codec_id);
    if (vdec == nullptr) {
        MEDIA_LOG_E("Codec: %d is not found", static_cast<int32_t>(avCodecContext_->codec_id));
        DeinitCodecContext();
        return Status::ERROR_INVALID_PARAMETER;
    }
    auto res = avcodec_open2(avCodecContext_.get(), avCodec_.get(), nullptr);
    if (res != 0) {
        MEDIA_LOG_E("avcodec open error %s when start decoder ", AVStrError(res).c_str());
        DeinitCodecContext();
        return Status::ERROR_UNKNOWN;
    }
    MEDIA_LOG_I("Open ffmpeg codec context success");
    return Status::OK;
}

Status VideoFfmpegDecoderPlugin::CloseCodecContext()
{
    Status ret = Status::OK;
    if (avCodecContext_ != nullptr) {
        auto res = avcodec_close(avCodecContext_.get());
        if (res != 0) {
            DeinitCodecContext();
            MEDIA_LOG_E("avcodec close error %s when stop decoder", AVStrError(res).c_str());
            ret = Status::ERROR_UNKNOWN;
        }
        avCodecContext_.reset();
    }
    return ret;
}

Status VideoFfmpegDecoderPlugin::Prepare()
{
    {
        OSAL::ScopedLock l(avMutex_);
        if (state_ != State::INITIALIZED && state_ != State::PREPARED) {
            return Status::ERROR_WRONG_STATE;
        }
        if (CreateCodecContext() != Status::OK) {
            MEDIA_LOG_E("Create codec context fail");
            return Status::ERROR_UNKNOWN;
        }
        InitCodecContext();
#ifdef DUMP_RAW_DATA
        dumpData_.open("./vdec_out.dat", std::ios::out | std::ios::binary);
#endif
        state_ = State::PREPARED;
    }
    outBufferQ_.SetActive(true);
    MEDIA_LOG_I("Prepare success");
    return Status::OK;
}

Status VideoFfmpegDecoderPlugin::ResetLocked()
{
    videoDecParams_.clear();
    avCodecContext_.reset();
    outBufferQ_.Clear();
#ifdef DUMP_RAW_DATA
    dumpData_.close();
#endif
    state_ = State::INITIALIZED;
    return Status::OK;
}

Status VideoFfmpegDecoderPlugin::Reset()
{
    OSAL::ScopedLock l(avMutex_);
    return ResetLocked();
}

Status VideoFfmpegDecoderPlugin::Start()
{
    {
        OSAL::ScopedLock l(avMutex_);
        if (state_ != State::PREPARED) {
            return Status::ERROR_WRONG_STATE;
        }
        if (OpenCodecContext() != Status::OK) {
            MEDIA_LOG_E("Open codec context fail");
            return Status::ERROR_UNKNOWN;
        }
        state_ = State::RUNNING;
    }
    outBufferQ_.SetActive(true);
    decodeTask_->Start();
    MEDIA_LOG_I("Start success");
    return Status::OK;
}

Status VideoFfmpegDecoderPlugin::Stop()
{
    Status ret = Status::OK;
    {
        OSAL::ScopedLock l(avMutex_);
        ret = CloseCodecContext();
#ifdef DUMP_RAW_DATA
        dumpData_.close();
#endif
        state_ = State::INITIALIZED;
    }
    outBufferQ_.SetActive(false);
    decodeTask_->Stop();
    MEDIA_LOG_I("Stop success");
    return ret;
}

Status VideoFfmpegDecoderPlugin::QueueOutputBuffer(const std::shared_ptr<Buffer>& outputBuffer, int32_t timeoutMs)
{
    MEDIA_LOG_D("queue output buffer");
    outBufferQ_.Push(outputBuffer);
    return Status::OK;
}

Status VideoFfmpegDecoderPlugin::DequeueOutputBuffer(std::shared_ptr<Buffer>& outputBuffers, int32_t timeoutMs)
{
    (void)timeoutMs;
    return Status::OK;
}

Status VideoFfmpegDecoderPlugin::Flush()
{
    OSAL::ScopedLock l(avMutex_);
    if (avCodecContext_ != nullptr) {
        // flush avcodec buffers
    }
    return Status::OK;
}

Status VideoFfmpegDecoderPlugin::QueueInputBuffer(const std::shared_ptr<Buffer>& inputBuffer, int32_t timeoutMs)
{
    MEDIA_LOG_D("queue input buffer");
    if (inputBuffer->IsEmpty() && !(inputBuffer->flag & BUFFER_FLAG_EOS)) {
        MEDIA_LOG_E("decoder does not support fd buffer");
        return Status::ERROR_INVALID_DATA;
    }
    Status ret = Status::OK;
    {
        OSAL::ScopedLock l(avMutex_);
        ret = SendBufferLocked(inputBuffer);
    }
    NotifyInputBufferDone(inputBuffer);
    return ret;
}

Status VideoFfmpegDecoderPlugin::DequeueInputBuffer(std::shared_ptr<Buffer>& inputBuffer, int32_t timeoutMs)
{
    (void)timeoutMs;
    return Status::OK;
}

Status VideoFfmpegDecoderPlugin::SendBufferLocked(const std::shared_ptr<Buffer>& inputBuffer)
{
    if (state_ != State::RUNNING) {
        MEDIA_LOG_W("queue input buffer in wrong state");
        return Status::ERROR_WRONG_STATE;
    }
    AVPacket packet;
    size_t bufferLength = 0;
    bool eos = false;
    if (inputBuffer == nullptr || (inputBuffer->flag & BUFFER_FLAG_EOS) != 0) {
        // eos buffer
        eos = true;
    } else {
        auto inputMemory = inputBuffer->GetMemory();
        const uint8_t* ptr = inputMemory->GetReadOnlyData();
        bufferLength = inputMemory->GetSize();
        size_t bufferEnd = bufferLength;
        // pad to data if needed
        if ((bufferLength % AV_INPUT_BUFFER_PADDING_SIZE != 0) &&
            (bufferLength - bufferEnd + bufferLength % AV_INPUT_BUFFER_PADDING_SIZE < AV_INPUT_BUFFER_PADDING_SIZE)) {
            if (paddedBufferSize_ < bufferLength + AV_INPUT_BUFFER_PADDING_SIZE) {
                paddedBufferSize_ = bufferLength + AV_INPUT_BUFFER_PADDING_SIZE;
                paddedBuffer_.reserve(paddedBufferSize_);
                MEDIA_LOG_I("increase padded buffer size to %zu", paddedBufferSize_);
            }
            paddedBuffer_.assign(ptr, ptr + bufferLength);
            paddedBuffer_.insert(paddedBuffer_.end(), AV_INPUT_BUFFER_PADDING_SIZE, 0);
            ptr = paddedBuffer_.data();
        }
        av_init_packet(&packet);
        packet.data = const_cast<uint8_t*>(ptr);
        packet.size = static_cast<int32_t>(bufferLength);
        packet.pts = static_cast<int64_t>(inputBuffer->pts);
    }
    AVPacket* packetPtr = nullptr;
    if (!eos) {
        packetPtr = &packet;
    }
    auto ret = avcodec_send_packet(avCodecContext_.get(), packetPtr);
    if (ret < 0) {
        MEDIA_LOG_D("send buffer error %s", AVStrError(ret).c_str());
        return Status::ERROR_NO_MEMORY;
    }
    return Status::OK;
}

void VideoFfmpegDecoderPlugin::CheckResolutionChange()
{
    if ((width_ > 0) && (height_ > 0) && ((cachedFrame_->width != width_) || (cachedFrame_->height != height_))) {
        MEDIA_LOG_W("Demuxer's W&H&S: [%u %u] diff from FFMPEG's [%d %d]", width_, height_, cachedFrame_->width,
                    cachedFrame_->height);
        // need to reallocte output buffers
    }
}

#ifdef DUMP_RAW_DATA
void VideoFfmpegDecoderPlugin::DumpVideoRawOutData()
{
    if (cachedFrame_->format == AV_PIX_FMT_YUV420P) {
        if (cachedFrame_->data[0] != nullptr && cachedFrame_->linesize[0] != 0) {
            dumpData_.write((char*)cachedFrame_->data[0], cachedFrame_->linesize[0] * cachedFrame_->height);
        }
        if (cachedFrame_->data[1] != nullptr && cachedFrame_->linesize[1] != 0) {
            dumpData_.write((char*)cachedFrame_->data[1], cachedFrame_->linesize[1] * cachedFrame_->height / 2); // 2
        }
        if (cachedFrame_->data[2] != nullptr && cachedFrame_->linesize[2] != 0) {                                // 2
            dumpData_.write((char*)cachedFrame_->data[2], cachedFrame_->linesize[2] * cachedFrame_->height / 2); // 2
        }
    } else if (cachedFrame_->format == AV_PIX_FMT_NV12 || cachedFrame_->format == AV_PIX_FMT_NV21) {
        if (cachedFrame_->data[0] != nullptr && cachedFrame_->linesize[0] != 0) {
            dumpData_.write((char*)cachedFrame_->data[0], cachedFrame_->linesize[0] * cachedFrame_->height);
        }
        if (cachedFrame_->data[1] != nullptr && cachedFrame_->linesize[1] != 0) {
            dumpData_.write((char*)cachedFrame_->data[1], cachedFrame_->linesize[1] * cachedFrame_->height / 2); // 2
        }
    }
}
#endif

void VideoFfmpegDecoderPlugin::CalculateFrameSizes(size_t& ySize, size_t& uvSize, size_t& frameSize)
{
    ySize = static_cast<size_t>(cachedFrame_->linesize[0] * AlignUp(cachedFrame_->height, 16)); // 16
    // AV_PIX_FMT_YUV420P: linesize[0] = linesize[1] * 2, AV_PIX_FMT_NV12: linesize[0] = linesize[1]
    uvSize = static_cast<size_t>(cachedFrame_->linesize[1] * AlignUp(cachedFrame_->height, 16) / 2); // 2 16
    frameSize = 0;
    if (cachedFrame_->format == AV_PIX_FMT_YUV420P) {
        frameSize = ySize + (uvSize * 2); // 2
    } else if (cachedFrame_->format == AV_PIX_FMT_NV12 || cachedFrame_->format == AV_PIX_FMT_NV21) {
        frameSize = ySize + uvSize;
    }
}

Status VideoFfmpegDecoderPlugin::FillFrameBuffer(const std::shared_ptr<Buffer>& frameBuffer)
{
    MEDIA_LOG_D("receive one frame: %d, picture type: %d, pixel format: %d, packet size: %d", cachedFrame_->key_frame,
                static_cast<int32_t>(cachedFrame_->pict_type), static_cast<int32_t>(cachedFrame_->format),
                cachedFrame_->pkt_size);
    if (cachedFrame_->flags & AV_FRAME_FLAG_CORRUPT ||
        g_pixelFormatMap[static_cast<AVPixelFormat>(cachedFrame_->format)] != pixelFormat_) {
        MEDIA_LOG_W("format: %d unsupported, pixelFormat_: %u", cachedFrame_->format, pixelFormat_);
        return Status::ERROR_INVALID_DATA;
    }
    CheckResolutionChange();
#ifdef DUMP_RAW_DATA
    DumpVideoRawOutData();
#endif
    MEDIA_LOG_D("linesize: %d, %d, %d", cachedFrame_->linesize[0], cachedFrame_->linesize[1],
                cachedFrame_->linesize[2]); // 2
    auto bufferMeta = frameBuffer->GetBufferMeta();
    if (bufferMeta != nullptr && bufferMeta->GetType() == BufferMetaType::VIDEO) {
        std::shared_ptr<VideoBufferMeta> videoMeta = std::dynamic_pointer_cast<VideoBufferMeta>(bufferMeta);
        videoMeta->videoPixelFormat = g_pixelFormatMap[static_cast<AVPixelFormat>(cachedFrame_->format)];
        videoMeta->height = cachedFrame_->height;
        videoMeta->width = cachedFrame_->width;
        for (int i = 0; cachedFrame_->linesize[i] > 0; ++i) {
            videoMeta->stride.emplace_back(cachedFrame_->linesize[i]);
        }
        videoMeta->planes = videoMeta->stride.size();
    }
    size_t ySize = 0;
    size_t uvSize = 0;
    size_t frameSize = 0;
    CalculateFrameSizes(ySize, uvSize, frameSize);
    auto frameBufferMem = frameBuffer->GetMemory();
    if (frameBufferMem->GetCapacity() < frameSize) {
        MEDIA_LOG_W("output buffer size is not enough: real[%zu], need[%zu]", frameBufferMem->GetCapacity(), frameSize);
        return Status::ERROR_NO_MEMORY;
    }
    if (cachedFrame_->format == AV_PIX_FMT_YUV420P) {
        frameBufferMem->Write(cachedFrame_->data[0], ySize);
        frameBufferMem->Write(cachedFrame_->data[1], uvSize);
        frameBufferMem->Write(cachedFrame_->data[2], uvSize); // 2
    } else if ((cachedFrame_->format == AV_PIX_FMT_NV12) || (cachedFrame_->format == AV_PIX_FMT_NV21)) {
        frameBufferMem->Write(cachedFrame_->data[0], ySize);
        frameBufferMem->Write(cachedFrame_->data[1], uvSize);
    } else {
        MEDIA_LOG_E("Unsupported pixel format: %d", cachedFrame_->format);
        return Status::ERROR_UNSUPPORTED_FORMAT;
    }
    frameBuffer->pts = static_cast<uint64_t>(cachedFrame_->pts);
    return Status::OK;
}

Status VideoFfmpegDecoderPlugin::ReceiveBufferLocked(const std::shared_ptr<Buffer>& frameBuffer)
{
    if (state_ != State::RUNNING) {
        MEDIA_LOG_W("queue input buffer in wrong state");
        return Status::ERROR_WRONG_STATE;
    }
    Status status;
    auto ret = avcodec_receive_frame(avCodecContext_.get(), cachedFrame_.get());
    if (ret >= 0) {
        status = FillFrameBuffer(frameBuffer);
    } else if (ret == AVERROR_EOF) {
        MEDIA_LOG_I("eos received");
        frameBuffer->GetMemory()->Reset();
        avcodec_flush_buffers(avCodecContext_.get());
        status = Status::END_OF_STREAM;
    } else {
        MEDIA_LOG_D("video decoder receive error: %s", AVStrError(ret).c_str());
        status = Status::ERROR_TIMED_OUT;
    }
    av_frame_unref(cachedFrame_.get());
    return status;
}

void VideoFfmpegDecoderPlugin::ReceiveBuffer()
{
    std::shared_ptr<Buffer> frameBuffer = outBufferQ_.Pop();
    if (frameBuffer == nullptr || frameBuffer->IsEmpty() ||
        frameBuffer->GetBufferMeta()->GetType() != BufferMetaType::VIDEO) {
        MEDIA_LOG_W("cannot fetch valid buffer to output");
        return;
    }
    Status status;
    {
        OSAL::ScopedLock l(avMutex_);
        status = ReceiveBufferLocked(frameBuffer);
    }
    if (status == Status::OK || status == Status::END_OF_STREAM) {
        NotifyOutputBufferDone(frameBuffer);
    } else {
        outBufferQ_.Push(frameBuffer);
    }
}

void VideoFfmpegDecoderPlugin::NotifyInputBufferDone(const std::shared_ptr<Buffer>& input)
{
    auto ptr = dataCb_.lock();
    if (ptr != nullptr) {
        ptr->OnInputBufferDone(input);
    }
}

void VideoFfmpegDecoderPlugin::NotifyOutputBufferDone(const std::shared_ptr<Buffer>& output)
{
    auto ptr = dataCb_.lock();
    if (ptr != nullptr) {
        ptr->OnOutputBufferDone(output);
    }
}

std::shared_ptr<Allocator> VideoFfmpegDecoderPlugin::GetAllocator()
{
    return nullptr;
}
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif
