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

#define HST_LOG_TAG "Ffmpeg_Au_Decoder"

#include "audio_ffmpeg_decoder_plugin.h"
#include <cstring>
#include <map>
#include <set>
#include "utils/constants.h"
#include "utils/memory_helper.h"
#include "plugin/common/plugin_audio_tags.h"
#include "plugin/common/plugin_buffer.h"
#include "plugin/interface/codec_plugin.h"
#include "plugins/ffmpeg_adapter/utils/ffmpeg_utils.h"

namespace {
// register plugins
using namespace OHOS::Media::Plugin;
void UpdatePluginDefinition(const AVCodec* codec, CodecPluginDef& definition);

std::map<std::string, std::shared_ptr<const AVCodec>> codecMap;

const size_t BUFFER_QUEUE_SIZE = 6;

std::set<AVCodecID> g_supportedCodec = {AV_CODEC_ID_MP3, AV_CODEC_ID_FLAC, AV_CODEC_ID_AAC, AV_CODEC_ID_AAC_LATM};

std::shared_ptr<CodecPlugin> AuFfmpegDecoderCreator(const std::string& name)
{
    return std::make_shared<AudioFfmpegDecoderPlugin>(name);
}

Status RegisterAudioDecoderPlugins(const std::shared_ptr<Register>& reg)
{
    const AVCodec* codec = nullptr;
    void* ite = nullptr;
    MEDIA_LOG_I("registering audio decoders");
    while ((codec = av_codec_iterate(&ite))) {
        if (!av_codec_is_decoder(codec) || codec->type != AVMEDIA_TYPE_AUDIO) {
            continue;
        }
        if (g_supportedCodec.find(codec->id) == g_supportedCodec.end()) {
            MEDIA_LOG_W("codec %s(%s) is not supported right now", codec->name, codec->long_name);
            continue;
        }
        CodecPluginDef definition;
        definition.name = "audecoder_" + std::string(codec->name);
        definition.codecType = CodecType::AUDIO_DECODER;
        definition.rank = 100; // 100
        definition.creator = AuFfmpegDecoderCreator;
        UpdatePluginDefinition(codec, definition);
        // do not delete the codec in the deleter
        codecMap[definition.name] = std::shared_ptr<AVCodec>(const_cast<AVCodec*>(codec), [](void* ptr) {});
        if (reg->AddPlugin(definition) != Status::OK) {
            MEDIA_LOG_W("register plugin %s(%s) failed", codec->name, codec->long_name);
        }
    }
    return Status::OK;
}

void UnRegisterAudioDecoderPlugin()
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
    Capability cap("audio/unknown");
    switch (codec->id) {
        case AV_CODEC_ID_MP3:
            cap.SetMime(OHOS::Media::MEDIA_MIME_AUDIO_MPEG)
                .AppendFixedKey<uint32_t>(Capability::Key::AUDIO_MPEG_VERSION, 1)
                .AppendIntervalKey<uint32_t>(Capability::Key::AUDIO_MPEG_LAYER, 1, 3); // 3
            break;
        case AV_CODEC_ID_FLAC:
            cap.SetMime(OHOS::Media::MEDIA_MIME_AUDIO_FLAC);
            break;
        case AV_CODEC_ID_AAC:
            cap.SetMime(OHOS::Media::MEDIA_MIME_AUDIO_AAC);
            break;
        case AV_CODEC_ID_AAC_LATM:
            cap.SetMime(OHOS::Media::MEDIA_MIME_AUDIO_AAC_LATM);
            break;
        default:
            MEDIA_LOG_I("codec is not supported right now");
    }

    size_t index = 0;
    if (codec->supported_samplerates != nullptr) {
        DiscreteCapability<uint32_t> values;
        for (index = 0; codec->supported_samplerates[index] != 0; ++index) {
            values.push_back(codec->supported_samplerates[index]);
        }
        if (index) {
            cap.AppendDiscreteKeys(Capability::Key::AUDIO_SAMPLE_RATE, values);
        }
    }

    if (codec->channel_layouts != nullptr) {
        DiscreteCapability<AudioChannelLayout> values;
        for (index = 0; codec->channel_layouts[index] != 0; ++index) {
            values.push_back(AudioChannelLayout(codec->channel_layouts[index]));
        }
        if (index) {
            cap.AppendDiscreteKeys<AudioChannelLayout>(Capability::Key::AUDIO_CHANNEL_LAYOUT, values);
        }
    }

    if (codec->sample_fmts != nullptr) {
        DiscreteCapability<AudioSampleFormat> values;
        for (index = 0; codec->sample_fmts[index] != AV_SAMPLE_FMT_NONE; ++index) {
            values.push_back(g_formatMap[codec->sample_fmts[index]]);
        }
        if (index) {
            cap.AppendDiscreteKeys<AudioSampleFormat>(Capability::Key::AUDIO_SAMPLE_FORMAT, values);
        }
    }
    definition.inCaps.push_back(cap);
    definition.outCaps.push_back(Capability(OHOS::Media::MEDIA_MIME_AUDIO_RAW));
}
uint32_t GetWidth(AVSampleFormat sampleFormat)
{
    switch (sampleFormat) {
        case AV_SAMPLE_FMT_U8P:
        case AV_SAMPLE_FMT_U8:
            return 1;
        case AV_SAMPLE_FMT_S16P:
        case AV_SAMPLE_FMT_S16:
            return 2; // 2
        case AV_SAMPLE_FMT_S32P:
        case AV_SAMPLE_FMT_S32:
        case AV_SAMPLE_FMT_FLTP:
        case AV_SAMPLE_FMT_FLT:
            return 4; // 4
        case AV_SAMPLE_FMT_DBLP:
        case AV_SAMPLE_FMT_DBL:
        case AV_SAMPLE_FMT_S64P:
        case AV_SAMPLE_FMT_S64:
            return 8; // 8
        default:
            MEDIA_LOG_W("not supported right now");
            return 1;
    }
}
} // namespace
PLUGIN_DEFINITION(FFmpegAudioDecoders, LicenseType::LGPL, RegisterAudioDecoderPlugins, UnRegisterAudioDecoderPlugin);

namespace OHOS {
namespace Media {
namespace Plugin {
AudioFfmpegDecoderPlugin::AudioFfmpegDecoderPlugin(std::string name)
    : CodecPlugin(std::move(name)) {}

AudioFfmpegDecoderPlugin::~AudioFfmpegDecoderPlugin()
{
    OSAL::ScopedLock lock(avMutex_);
    OSAL::ScopedLock lock1(parameterMutex_);
    DeInitLocked();
}
Status AudioFfmpegDecoderPlugin::Init()
{
    auto ite = codecMap.find(pluginName_);
    if (ite == codecMap.end()) {
        MEDIA_LOG_W("cannot find codec with name %s", pluginName_.c_str());
        return Status::ERROR_UNSUPPORTED_FORMAT;
    }
    OSAL::ScopedLock lock(avMutex_);
    avCodec_ = ite->second;
    cachedFrame_ = std::shared_ptr<AVFrame>(av_frame_alloc(), [](AVFrame* fp) { av_frame_free(&fp); });
    OSAL::ScopedLock lock1(parameterMutex_);
    audioParameter_[Tag::REQUIRED_OUT_BUFFER_CNT] = (uint32_t)BUFFER_QUEUE_SIZE;
    return Status::OK;
}

Status AudioFfmpegDecoderPlugin::Deinit()
{
    OSAL::ScopedLock lock(avMutex_);
    OSAL::ScopedLock lock1(parameterMutex_);
    return DeInitLocked();
}

Status AudioFfmpegDecoderPlugin::DeInitLocked()
{
    avCodec_.reset();
    cachedFrame_.reset();
    ResetLocked();
    return Status::OK;
}

Status AudioFfmpegDecoderPlugin::SetParameter(Tag tag, const ValueType& value)
{
    OSAL::ScopedLock lock(parameterMutex_);
    audioParameter_.insert(std::make_pair(tag, value));
    return Status::OK;
}

Status AudioFfmpegDecoderPlugin::GetParameter(Tag tag, ValueType& value)
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
bool AudioFfmpegDecoderPlugin::FindInParameterMapThenAssignLocked(Tag tag, T& assign)
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

Status AudioFfmpegDecoderPlugin::Prepare()
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
        uint32_t tmp = 0;
        {
            OSAL::ScopedLock lock1(parameterMutex_);
            if (FindInParameterMapThenAssignLocked<uint32_t>(Tag::AUDIO_CHANNELS, tmp)) {
                avCodecContext_->channels = tmp;
            }
            if (FindInParameterMapThenAssignLocked<uint32_t>(Tag::AUDIO_SAMPLE_RATE, tmp)) {
                avCodecContext_->sample_rate = tmp;
            }
            int64_t bitRate = 0;
            if (FindInParameterMapThenAssignLocked<int64_t>(Tag::MEDIA_BITRATE, bitRate)) {
                avCodecContext_->bit_rate = bitRate;
            }
            AudioSampleFormat audioSampleFormat = AudioSampleFormat::S8;
            if (FindInParameterMapThenAssignLocked(Tag::AUDIO_SAMPLE_FORMAT, audioSampleFormat)) {
                auto ite = g_reverseFormatMap.find(audioSampleFormat);
                if (ite != g_reverseFormatMap.end()) {
                    avCodecContext_->sample_fmt = ite->second;
                }
            }
            InitCodecContextExtraDataLocked();
        }

        avCodecContext_->workaround_bugs =
            static_cast<uint32_t>(avCodecContext_->workaround_bugs) | static_cast<uint32_t>(FF_BUG_AUTODETECT);
        avCodecContext_->err_recognition = 1;
    }
    return Status::OK;
}

void AudioFfmpegDecoderPlugin::InitCodecContextExtraDataLocked()
{
    if (!avCodecContext_) {
        return;
    }
    auto it = audioParameter_.find(Tag::MEDIA_CODEC_CONFIG);
    if (it == audioParameter_.end() || it->second.Type() != typeid(std::vector<uint8_t>)) {
        return;
    }
    auto codecConfig = Plugin::AnyCast<std::vector<uint8_t>>(it->second);
    int configSize = codecConfig.size();
    if (configSize > 0) {
        auto allocSize = AlignUp(configSize + AV_INPUT_BUFFER_PADDING_SIZE, 16); // 16
        avCodecContext_->extradata = static_cast<uint8_t*>(av_mallocz(allocSize));
        if (memcpy_s(avCodecContext_->extradata, configSize, codecConfig.data(), configSize) != EOK) {
            MEDIA_LOG_E("init codec context extradata error");
            // todo should report error to fwk using callback
            return;
        }
        avCodecContext_->extradata_size = configSize;
    }
}

Status AudioFfmpegDecoderPlugin::ResetLocked()
{
    audioParameter_.clear();
    avCodecContext_.reset();
    return Status::OK;
}

Status AudioFfmpegDecoderPlugin::Reset()
{
    OSAL::ScopedLock lock(avMutex_);
    OSAL::ScopedLock lock1(parameterMutex_);
    return ResetLocked();
}

Status AudioFfmpegDecoderPlugin::Start()
{
    {
        OSAL::ScopedLock lock(avMutex_);
        if (avCodecContext_ == nullptr) {
            return Status::ERROR_WRONG_STATE;
        }
        auto res = avcodec_open2(avCodecContext_.get(), avCodec_.get(), nullptr);
        if (res != 0) {
            MEDIA_LOG_E("avcodec open error %s when start decoder ", AVStrError(res).c_str());
            return Status::ERROR_UNKNOWN;
        }
    }
    return Status::OK;
}

Status AudioFfmpegDecoderPlugin::Stop()
{
    Status ret = Status::OK;
    {
        OSAL::ScopedLock lock(avMutex_);
        if (avCodecContext_ != nullptr) {
            auto res = avcodec_close(avCodecContext_.get());
            if (res != 0) {
                MEDIA_LOG_E("avcodec close error %s when stop decoder", AVStrError(res).c_str());
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

Status AudioFfmpegDecoderPlugin::Flush()
{
    MEDIA_LOG_I("Flush entered.");
    OSAL::ScopedLock lock(avMutex_);
    if (avCodecContext_ != nullptr) {
        avcodec_flush_buffers(avCodecContext_.get());
    }
    MEDIA_LOG_I("Flush exit.");
    return Status::OK;
}

Status AudioFfmpegDecoderPlugin::QueueInputBuffer(const std::shared_ptr<Buffer>& inputBuffer, int32_t timeoutMs)
{
    MEDIA_LOG_D("queue input buffer");
    (void)timeoutMs;
    if (inputBuffer->IsEmpty() && !(inputBuffer->flag & BUFFER_FLAG_EOS)) {
        MEDIA_LOG_E("decoder does not support fd buffer");
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

Status AudioFfmpegDecoderPlugin::DequeueInputBuffer(std::shared_ptr<Buffer>& inputBuffer, int32_t timeoutMs)
{
    // dummy
    return Status::OK;
}

Status AudioFfmpegDecoderPlugin::QueueOutputBuffer(const std::shared_ptr<Buffer>& outputBuffers, int32_t timeoutMs)
{
    MEDIA_LOG_I("queue out put");
    (void)timeoutMs;
    if (!outputBuffers) {
        return Status::ERROR_INVALID_PARAMETER;
    }
    outBuffer_ = outputBuffers;
    return Status::OK;
}

Status AudioFfmpegDecoderPlugin::DequeueOutputBuffer(std::shared_ptr<Buffer>& outputBuffers, int32_t timeoutMs)
{
    MEDIA_LOG_I("dequeue out put");
    (void)timeoutMs;
    Status status;
    do {
        status = ReceiveBuffer();
    } while (status == Status::ERROR_NOT_ENOUGH_DATA);
    if (status == Status::OK || status == Status::END_OF_STREAM) {
        outputBuffers = outBuffer_;
    }
    outBuffer_.reset();
    return status;
}

Status AudioFfmpegDecoderPlugin::SendBufferLocked(const std::shared_ptr<Buffer>& inputBuffer)
{
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
        // pad to data if needed
        if (bufferLength % AV_INPUT_BUFFER_PADDING_SIZE != 0) {
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
        packet.size = bufferLength;
        packet.pts = inputBuffer->pts;
    }
    AVPacket* packetPtr = nullptr;
    if (!eos) {
        packetPtr = &packet;
    }
    auto ret = avcodec_send_packet(avCodecContext_.get(), packetPtr);
    if (ret < 0) {
        MEDIA_LOG_E("send buffer error %s", AVStrError(ret).c_str());
        return Status::ERROR_TIMED_OUT;
    }
    return Status::OK;
}

Status AudioFfmpegDecoderPlugin::ReceiveFrameSucc(const std::shared_ptr<Buffer>& ioInfo)
{
    int32_t channels = cachedFrame_->channels;
    int32_t samples = cachedFrame_->nb_samples;
    auto sampleFormat = static_cast<AVSampleFormat>(cachedFrame_->format);
    int32_t bytePerSample = GetWidth(sampleFormat);
    int32_t outputSize = samples * bytePerSample * channels;
    auto ioInfoMem = ioInfo->GetMemory();
    if (ioInfoMem->GetCapacity() < outputSize) {
        MEDIA_LOG_W("output buffer size is not enough");
        return Status::ERROR_NO_MEMORY;
    }
    if (av_sample_fmt_is_planar(avCodecContext_->sample_fmt)) {
        int32_t planarSize = outputSize / channels;
        for (size_t idx = 0; idx < channels; idx++) {
            ioInfoMem->Write(cachedFrame_->extended_data[idx], planarSize);
        }
    } else {
        ioInfoMem->Write(cachedFrame_->data[0], outputSize);
    }
    ioInfo->pts = static_cast<uint64_t>(cachedFrame_->pts);
    return Status::OK;
}

Status AudioFfmpegDecoderPlugin::ReceiveBufferLocked(const std::shared_ptr<Buffer>& ioInfo)
{
    Status status;
    auto ret = avcodec_receive_frame(avCodecContext_.get(), cachedFrame_.get());
    if (ret >= 0) {
        MEDIA_LOG_D("receive one frame");
        status = ReceiveFrameSucc(ioInfo);
    } else if (ret == AVERROR_EOF) {
        MEDIA_LOG_I("eos received");
        ioInfo->GetMemory()->Reset();
        ioInfo->flag = BUFFER_FLAG_EOS;
        status = Status::END_OF_STREAM;
    } else {
        MEDIA_LOG_I("audio decoder receive error: %s", AVStrError(ret).c_str());
        status = Status::ERROR_NOT_ENOUGH_DATA;
    }
    av_frame_unref(cachedFrame_.get());
    return status;
}

Status AudioFfmpegDecoderPlugin::ReceiveBuffer(void)
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

std::shared_ptr<Allocator> AudioFfmpegDecoderPlugin::GetAllocator()
{
    return nullptr;
}
} // namespace Plugin
} // namespace Media
} // namespace OHOS
