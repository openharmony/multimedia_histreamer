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

#define HST_LOG_TAG "Ffmpeg_Au_Encoder"

#include "audio_ffmpeg_encoder_plugin.h"
#include <cstring>
#include <map>
#include <set>
#include "utils/memory_helper.h"
#include "plugins/ffmpeg_adapter/utils/ffmpeg_utils.h"

namespace {
// register plugins
using namespace OHOS::Media::Plugin;
void UpdatePluginDefinition(const AVCodec* codec, CodecPluginDef& definition);

std::map<std::string, std::shared_ptr<const AVCodec>> codecMap;

const size_t BUFFER_QUEUE_SIZE = 6;

std::set<AVCodecID> g_supportedCodec = {AV_CODEC_ID_AAC, AV_CODEC_ID_AAC_LATM};

std::shared_ptr<CodecPlugin> AuFfmpegEncoderCreator(const std::string& name)
{
    return std::make_shared<AudioFfmpegEncoderPlugin>(name);
}

Status RegisterAudioEncoderPlugins(const std::shared_ptr<Register>& reg)
{
    const AVCodec* codec = nullptr;
    void* ite = nullptr;
    MEDIA_LOG_I("registering audio encoders");
    while ((codec = av_codec_iterate(&ite))) {
        if (!av_codec_is_encoder(codec) || codec->type != AVMEDIA_TYPE_AUDIO) {
            continue;
        }
        if (g_supportedCodec.find(codec->id) == g_supportedCodec.end()) {
            MEDIA_LOG_W("codec %s(%s) is not supported right now", codec->name, codec->long_name);
            continue;
        }
        CodecPluginDef definition;
        definition.name = "ffmpegAuEnc_" + std::string(codec->name);
        definition.codecType = CodecType::AUDIO_ENCODER;
        definition.rank = 100; // 100
        definition.creator = AuFfmpegEncoderCreator;
        UpdatePluginDefinition(codec, definition);
        // do not delete the codec in the deleter
        codecMap[definition.name] = std::shared_ptr<AVCodec>(const_cast<AVCodec*>(codec), [](void* ptr) {});
        if (reg->AddPlugin(definition) != Status::OK) {
            MEDIA_LOG_W("register plugin %s(%s) failed", codec->name, codec->long_name);
        }
    }
    return Status::OK;
}

void UnRegisterAudioEncoderPlugin()
{
    codecMap.clear();
}

std::map<AVSampleFormat, AudioSampleFormat> g_formatMap = {
    {AV_SAMPLE_FMT_U8, AudioSampleFormat::U8},   {AV_SAMPLE_FMT_U8P, AudioSampleFormat::U8P},
    {AV_SAMPLE_FMT_S16, AudioSampleFormat::S16}, {AV_SAMPLE_FMT_S16P, AudioSampleFormat::S16P},
    {AV_SAMPLE_FMT_S32, AudioSampleFormat::S32}, {AV_SAMPLE_FMT_S32P, AudioSampleFormat::S32P},
    {AV_SAMPLE_FMT_FLT, AudioSampleFormat::F32}, {AV_SAMPLE_FMT_FLTP, AudioSampleFormat::F32P},
    {AV_SAMPLE_FMT_DBL, AudioSampleFormat::F64}, {AV_SAMPLE_FMT_DBLP, AudioSampleFormat::F64P},
};

std::map<AudioSampleFormat, AVSampleFormat> g_reverseFormatMap = {
    {AudioSampleFormat::U8, AV_SAMPLE_FMT_U8},   {AudioSampleFormat::U8P, AV_SAMPLE_FMT_U8P},
    {AudioSampleFormat::S16, AV_SAMPLE_FMT_S16}, {AudioSampleFormat::S16P, AV_SAMPLE_FMT_S16P},
    {AudioSampleFormat::S32, AV_SAMPLE_FMT_S32}, {AudioSampleFormat::S32P, AV_SAMPLE_FMT_S32P},
    {AudioSampleFormat::F32, AV_SAMPLE_FMT_FLT}, {AudioSampleFormat::F32P, AV_SAMPLE_FMT_FLTP},
    {AudioSampleFormat::F64, AV_SAMPLE_FMT_DBL}, {AudioSampleFormat::F64P, AV_SAMPLE_FMT_DBLP},
};

void UpdatePluginDefinition(const AVCodec* codec, CodecPluginDef& definition)
{
    Capability inputCaps(OHOS::Media::MEDIA_MIME_AUDIO_RAW);
    size_t index = 0;
    if (codec->sample_fmts != nullptr) {
        // todo we should always consider transfer sample fmt to supported format
    }
    if (codec->supported_samplerates != nullptr) {
        DiscreteCapability<uint32_t> values;
        for (index = 0; codec->supported_samplerates[index] != 0; ++index) {
            values.push_back(codec->supported_samplerates[index]);
        }
        if (index) {
            inputCaps.AppendDiscreteKeys(Capability::Key::AUDIO_SAMPLE_RATE, values);
        }
    }
    definition.inCaps.push_back(inputCaps);

    Capability outputCaps;
    switch (codec->id) {
        case AV_CODEC_ID_AAC:
            outputCaps.SetMime(OHOS::Media::MEDIA_MIME_AUDIO_AAC)
                .AppendFixedKey<uint32_t>(Capability::Key::AUDIO_MPEG_VERSION, 4)  // 4
                .AppendFixedKey<AudioAacProfile>(Capability::Key::AUDIO_AAC_PROFILE, AudioAacProfile::LC)
                .AppendFixedKey<AudioAacStreamFormat>(Capability::Key::AUDIO_AAC_STREAM_FORMAT,
                                                      AudioAacStreamFormat::MP4ADTS);
            break;
        case AV_CODEC_ID_AAC_LATM:
            outputCaps.SetMime(OHOS::Media::MEDIA_MIME_AUDIO_AAC_LATM)
                .AppendFixedKey<uint32_t>(Capability::Key::AUDIO_MPEG_VERSION, 4)  // 4
                .AppendFixedKey<AudioAacStreamFormat>(Capability::Key::AUDIO_AAC_STREAM_FORMAT,
                                                      AudioAacStreamFormat::MP4LOAS);
            break;
        default:
            MEDIA_LOG_I("codec is not supported right now");
    }
    definition.outCaps.push_back(outputCaps);
}
} // namespace
PLUGIN_DEFINITION(FFmpegAudioEncoders, LicenseType::LGPL, RegisterAudioEncoderPlugins, UnRegisterAudioEncoderPlugin);

namespace OHOS {
namespace Media {
namespace Plugin {
AudioFfmpegEncoderPlugin::AudioFfmpegEncoderPlugin(std::string name) : CodecPlugin(std::move(name)), prev_pts_(0)
{
}

AudioFfmpegEncoderPlugin::~AudioFfmpegEncoderPlugin()
{
    OSAL::ScopedLock lock(avMutex_);
    OSAL::ScopedLock lock1(parameterMutex_);
    DeInitLocked();
}

Status AudioFfmpegEncoderPlugin::Init()
{
    auto ite = codecMap.find(pluginName_);
    if (ite == codecMap.end()) {
        MEDIA_LOG_W("cannot find codec with name %s", pluginName_.c_str());
        return Status::ERROR_UNSUPPORTED_FORMAT;
    }
    OSAL::ScopedLock lock(avMutex_);
    avCodec_ = ite->second;
    cachedFrame_ = av_frame_alloc();
    OSAL::ScopedLock lock1(parameterMutex_);
    audioParameter_[Tag::REQUIRED_OUT_BUFFER_CNT] = (uint32_t)BUFFER_QUEUE_SIZE;
    return Status::OK;
}

Status AudioFfmpegEncoderPlugin::Deinit()
{
    OSAL::ScopedLock lock(avMutex_);
    OSAL::ScopedLock lock1(parameterMutex_);
    return DeInitLocked();
}

Status AudioFfmpegEncoderPlugin::DeInitLocked()
{
    avCodec_.reset();
    av_frame_free(&cachedFrame_);
    ResetLocked();
    return Status::OK;
}

Status AudioFfmpegEncoderPlugin::SetParameter(Tag tag, const ValueType& value)
{
    OSAL::ScopedLock lock(parameterMutex_);
    audioParameter_.insert(std::make_pair(tag, value));
    return Status::OK;
}

Status AudioFfmpegEncoderPlugin::GetParameter(Tag tag, ValueType& value)
{
    OSAL::ScopedLock lock(parameterMutex_);
    auto res = audioParameter_.find(tag);
    if (res != audioParameter_.end()) {
        value = res->second;
        return Status::OK;
    }
    return Status::ERROR_INVALID_PARAMETER;
}

template <typename T>
bool AudioFfmpegEncoderPlugin::FindInParameterMapThenAssignLocked(Tag tag, T& assign)
{
    auto ite = audioParameter_.find(tag);
    if (ite != audioParameter_.end() && typeid(T) == ite->second.Type()) {
        assign = Plugin::AnyCast<T>(ite->second);
        return true;
    } else {
        MEDIA_LOG_W("parameter %d is not found or type mismatch", static_cast<int32_t>(tag));
        return false;
    }
}

void AudioFfmpegEncoderPlugin::ConfigCodecLocked()
{
    uint32_t tmp = 0;
    if (FindInParameterMapThenAssignLocked<uint32_t>(Tag::AUDIO_CHANNELS, tmp)) {
        avCodecContext_->channels = tmp;
    }
    if (FindInParameterMapThenAssignLocked<uint32_t>(Tag::AUDIO_SAMPLE_RATE, tmp)) {
        avCodecContext_->sample_rate = tmp; // unused for constant quantizer encoding
    }
    int64_t bitRate = 0;
    if (FindInParameterMapThenAssignLocked<int64_t>(Tag::MEDIA_BITRATE, bitRate)) {
        avCodecContext_->bit_rate = bitRate;
    }
    AudioSampleFormat audioSampleFormat = AudioSampleFormat::S16;
    if (FindInParameterMapThenAssignLocked(Tag::AUDIO_SAMPLE_FORMAT, audioSampleFormat) &&
        g_reverseFormatMap.count(audioSampleFormat) != 0) {
        avCodecContext_->sample_fmt = g_reverseFormatMap.find(audioSampleFormat)->second;
    }
    AudioChannelLayout audioChannelLayout = AudioChannelLayout::STEREO;
    if (FindInParameterMapThenAssignLocked(Tag::AUDIO_CHANNEL_LAYOUT, audioChannelLayout)) {
        avCodecContext_->channel_layout = ConvertChannelLayoutToFFmpeg(audioChannelLayout);
    }
}

Status AudioFfmpegEncoderPlugin::Prepare()
{
    {
        OSAL::ScopedLock lock(avMutex_);
        if (avCodec_ == nullptr) {
            return Status::ERROR_WRONG_STATE;
        }
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
        {
            OSAL::ScopedLock lock1(parameterMutex_);
            ConfigCodecLocked();
        }

        if (!avCodecContext_->time_base.den) {
            avCodecContext_->time_base.den = avCodecContext_->sample_rate;
            avCodecContext_->time_base.num = 1;
            avCodecContext_->ticks_per_frame = 1;
        }

        avCodecContext_->workaround_bugs =
            static_cast<uint32_t>(avCodecContext_->workaround_bugs) | static_cast<uint32_t>(FF_BUG_AUTODETECT);
    }
    return Status::OK;
}

Status AudioFfmpegEncoderPlugin::ResetLocked()
{
    audioParameter_.clear();
    avCodecContext_.reset();
    return Status::OK;
}

Status AudioFfmpegEncoderPlugin::Reset()
{
    OSAL::ScopedLock lock(avMutex_);
    OSAL::ScopedLock lock1(parameterMutex_);
    return ResetLocked();
}

void AudioFfmpegEncoderPlugin::InitCacheFrame()
{
    if (!cachedFrame_) {
        cachedFrame_ = av_frame_alloc();
    }
    cachedFrame_->format = avCodecContext_->sample_fmt;
    cachedFrame_->sample_rate = avCodecContext_->sample_rate;
    cachedFrame_->channels = avCodecContext_->channels;
    cachedFrame_->channel_layout = avCodecContext_->channel_layout;
}

bool AudioFfmpegEncoderPlugin::CheckReformat()
{
    if (avCodec_ == nullptr || avCodecContext_ == nullptr) {
        return false;
    }
    for (size_t index = 0; avCodec_->sample_fmts[index] != AV_SAMPLE_FMT_NONE; ++index) {
        if (avCodec_->sample_fmts[index] == avCodecContext_->sample_fmt) {
            return false;
        }
    }
    return true;
}

Status AudioFfmpegEncoderPlugin::Start()
{
    {
        OSAL::ScopedLock lock(avMutex_);
        if (avCodecContext_ == nullptr) {
            return Status::ERROR_WRONG_STATE;
        }
        needReformat_ = CheckReformat();
        if (needReformat_) {
            sourceFmt_ = avCodecContext_->sample_fmt;
            // always use the first fmt
            avCodecContext_->sample_fmt = avCodec_->sample_fmts[0];
            SwrContext* swrContext = swr_alloc();
            if (swrContext == nullptr) {
                MEDIA_LOG_E("cannot allocate swr context");
                return Status::ERROR_NO_MEMORY;
            }
            swrContext = swr_alloc_set_opts(swrContext, AV_CH_LAYOUT_STEREO, avCodecContext_->sample_fmt,
                avCodecContext_->sample_rate, AV_CH_LAYOUT_STEREO, sourceFmt_, avCodecContext_->sample_rate,
                0, nullptr);
            if (swr_init(swrContext) != 0) {
                MEDIA_LOG_E("swr init error");
                return Status::ERROR_UNKNOWN;
            }
            swrCtx_ = std::shared_ptr<SwrContext>(swrContext, [](SwrContext* ptr) {
                if (ptr) {
                    swr_free(&ptr);
                }
            });
        }
        auto res = avcodec_open2(avCodecContext_.get(), avCodec_.get(), nullptr);
        if (res != 0) {
            MEDIA_LOG_E("avcodec open error %s when start encoder ", AVStrError(res).c_str());
            return Status::ERROR_UNKNOWN;
        }
        if (avCodecContext_->frame_size <= 0) {
            MEDIA_LOG_E("frame_size unknown");
            return Status::ERROR_UNKNOWN;
        }
        if (needReformat_) {
            resampleCache_.reserve(avCodecContext_->frame_size * avCodecContext_->channels);
        }
        SetParameter(Tag::AUDIO_SAMPLE_PER_FRAME, static_cast<uint32_t>(avCodecContext_->frame_size));
        InitCacheFrame();
    }
    return Status::OK;
}

Status AudioFfmpegEncoderPlugin::Stop()
{
    Status ret = Status::OK;
    {
        OSAL::ScopedLock lock(avMutex_);
        if (avCodecContext_ != nullptr) {
            auto res = avcodec_close(avCodecContext_.get());
            if (res != 0) {
                MEDIA_LOG_E("avcodec close error %s when stop encoder", AVStrError(res).c_str());
                ret = Status::ERROR_UNKNOWN;
            }
            avCodecContext_.reset();
        }
        if (outBuffer_) {
            outBuffer_.reset();
        }
    }
    return ret;
}

Status AudioFfmpegEncoderPlugin::Flush()
{
    MEDIA_LOG_I("Flush entered.");
    OSAL::ScopedLock lock(avMutex_);
    if (avCodecContext_ != nullptr) {
        avcodec_flush_buffers(avCodecContext_.get());
    }
    MEDIA_LOG_I("Flush exit.");
    return Status::OK;
}

Status AudioFfmpegEncoderPlugin::QueueInputBuffer(const std::shared_ptr<Buffer>& inputBuffer, int32_t timeoutMs)
{
    MEDIA_LOG_D("queue input buffer");
    (void)timeoutMs;
    if (inputBuffer->IsEmpty() && !(inputBuffer->flag & BUFFER_FLAG_EOS)) {
        MEDIA_LOG_E("encoder does not support fd buffer");
        return Status::ERROR_INVALID_DATA;
    }
    Status ret = Status::OK;
    {
        OSAL::ScopedLock lock(avMutex_);
        if (avCodecContext_ == nullptr) {
            return Status::ERROR_WRONG_STATE;
        }
        ret = SendBufferLocked(inputBuffer);
    }
    return ret;
}

Status AudioFfmpegEncoderPlugin::DequeueInputBuffer(std::shared_ptr<Buffer>& inputBuffer, int32_t timeoutMs)
{
    MEDIA_LOG_D("dequeue input buffer");
    (void)timeoutMs;
    return Status::OK;
}

Status AudioFfmpegEncoderPlugin::QueueOutputBuffer(const std::shared_ptr<Buffer>& outputBuffer, int32_t timeoutMs)
{
    MEDIA_LOG_D("queue output buffer");
    (void)timeoutMs;
    if (!outputBuffer) {
        return Status::ERROR_INVALID_PARAMETER;
    }
    outBuffer_ = outputBuffer;
    return Status::OK;
}

Status AudioFfmpegEncoderPlugin::DequeueOutputBuffer(std::shared_ptr<Buffer>& outputBuffers, int32_t timeoutMs)
{
    MEDIA_LOG_D("dequeue output buffer");
    (void)timeoutMs;
    Status status = ReceiveBuffer();
    outputBuffers.reset();
    if (status == Status::OK || status == Status::END_OF_STREAM) {
        outputBuffers = outBuffer_;
    }
    outBuffer_.reset();
    return status;
}

Status AudioFfmpegEncoderPlugin::SendBufferLocked(const std::shared_ptr<Buffer>& inputBuffer)
{
    size_t bufferLength = 0;
    bool eos = false;
    if (inputBuffer == nullptr || (inputBuffer->flag & BUFFER_FLAG_EOS) != 0) {
        // eos buffer
        eos = true;
    } else {
        auto inputMemory = inputBuffer->GetMemory();
        if (inputMemory->GetSize() != static_cast<size_t>(avCodecContext_->frame_size)) {
            // need more data
            MEDIA_LOG_W("Not enough data, input: %zu, frameSize: %d",
                        inputMemory->GetSize(), avCodecContext_->frame_size);
            return Status::ERROR_NOT_ENOUGH_DATA;
        }
        uint8_t* sampleData = nullptr;
        if (needReformat_) {
            std::vector<const uint8_t *> input(avCodecContext_->channels);
            input[0] = static_cast<const uint8_t *>(inputMemory->GetReadOnlyData());
            if (av_sample_fmt_is_planar(sourceFmt_)) {
                size_t planeSize = inputMemory->GetSize() / avCodecContext_->channels;
                for (auto i = 1; i < avCodecContext_->channels; ++i) {
                    input[i] = input[i - 1] + planeSize;
                }
            }
            sampleData = resampleCache_.data();
            swr_convert(swrCtx_.get(), &sampleData, resampleCache_.capacity(), input.data(), avCodecContext_->frame_size);
        } else {
            sampleData = const_cast<uint8_t*>(inputMemory->GetReadOnlyData());
        }
        bool isPlanar = av_sample_fmt_is_planar(avCodecContext_->sample_fmt);
        if (isPlanar && avCodecContext_->channels > 1) {
            for (size_t idx = 0; idx < avCodecContext_->channels; idx++) {
                cachedFrame_->data[idx] = sampleData + avCodecContext_->frame_size * idx;
                cachedFrame_->extended_data[idx] = cachedFrame_->data[idx];
                cachedFrame_->linesize[idx] = avCodecContext_->frame_size;
            }
        } else {
            cachedFrame_->data[0] = sampleData;
            cachedFrame_->extended_data = cachedFrame_->data;
            cachedFrame_->linesize[0] = bufferLength;
            cachedFrame_->nb_samples = bufferLength / avCodecContext_->frame_size; // need to check
        }
    }
    AVFrame* inputFrame = nullptr;
    if (!eos) {
        inputFrame = cachedFrame_;
    }
    auto ret = avcodec_send_frame(avCodecContext_.get(), inputFrame);
    if (!eos && inputFrame) {
        av_frame_unref(inputFrame);
    }
    if (ret == 0) {
        return Status::OK;
    } else if (ret == AVERROR_EOF) {
        return Status::END_OF_STREAM;
    } else if (ret == AVERROR(EAGAIN)) {
        return Status::ERROR_AGAIN;
    } else {
        MEDIA_LOG_E("send buffer error %s", AVStrError(ret).c_str());
        return Status::ERROR_UNKNOWN;
    }
}

Status AudioFfmpegEncoderPlugin::ReceiveFrameSucc(const std::shared_ptr<Buffer>& ioInfo,
                                                  std::shared_ptr<AVPacket> packet)
{
    auto ioInfoMem = ioInfo->GetMemory();
    if (ioInfoMem->GetCapacity() < packet->size) {
        MEDIA_LOG_W("output buffer size is not enough");
        return Status::ERROR_NO_MEMORY;
    }
    ioInfoMem->Write(packet->data, packet->size);
    // how get perfect pts with upstream pts ?
    ioInfo->pts = (UINT64_MAX - prev_pts_ < packet->duration) ?
                  (packet->duration - (UINT64_MAX - prev_pts_)) :
                  (prev_pts_ + static_cast<uint64_t>(packet->duration));
    prev_pts_ = ioInfo->pts;
    return Status::OK;
}

Status AudioFfmpegEncoderPlugin::ReceiveBufferLocked(const std::shared_ptr<Buffer>& ioInfo)
{
    Status status;
    std::shared_ptr<AVPacket> packet = std::make_shared<AVPacket>();
    auto ret = avcodec_receive_packet(avCodecContext_.get(), packet.get());
    if (ret >= 0) {
        MEDIA_LOG_D("receive one frame");
        status = ReceiveFrameSucc(ioInfo, packet);
    } else if (ret == AVERROR_EOF) {
        MEDIA_LOG_I("eos received");
        ioInfo->GetMemory()->Reset();
        ioInfo->flag = BUFFER_FLAG_EOS;
        status = Status::END_OF_STREAM;
    } else if (ret == AVERROR(EAGAIN)) {
        status = Status::ERROR_NOT_ENOUGH_DATA;
    } else {
        MEDIA_LOG_E("audio encoder receive error: %s", AVStrError(ret).c_str());
        status = Status::ERROR_UNKNOWN;
    }
    av_frame_unref(cachedFrame_);
    return status;
}

Status AudioFfmpegEncoderPlugin::ReceiveBuffer()
{
    std::shared_ptr<Buffer> ioInfo = outBuffer_;
    if ((ioInfo == nullptr) || ioInfo->IsEmpty() ||
        (ioInfo->GetBufferMeta()->GetType() != BufferMetaType::AUDIO)) {
        MEDIA_LOG_W("cannot fetch valid buffer to output");
        return Status::ERROR_NO_MEMORY;
    }
    Status status;
    {
        OSAL::ScopedLock l(avMutex_);
        if (avCodecContext_ == nullptr) {
            return Status::ERROR_WRONG_STATE;
        }
        status = ReceiveBufferLocked(ioInfo);
    }
    return status;
}

std::shared_ptr<Allocator> AudioFfmpegEncoderPlugin::GetAllocator()
{
    return nullptr;
}
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif