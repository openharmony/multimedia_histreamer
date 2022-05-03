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
#define HST_LOG_TAG "FfmpegTrackMeta"
#include "ffmpeg_track_meta.h"
#include "foundation/log.h"
#include "plugins/ffmpeg_adapter/utils/aac_audio_config_parser.h"
#include "plugins/ffmpeg_adapter/utils/ffmpeg_utils.h"
#include "utils/constants.h"

#ifdef VIDEO_SUPPORT
#include "plugins/ffmpeg_adapter/utils/avc_config_data_parser.h"
#endif

namespace OHOS {
namespace Media {
namespace Plugin {
namespace Ffmpeg {
namespace {
using ConvertFunc = void (*)(const AVStream& avStream, const std::shared_ptr<AVFormatContext>& avFormatContext,
        const std::shared_ptr<AVCodecContext>& avCodecContext, TagMap& meta);

struct StreamConvertor {
    AVCodecID codecId;
    ConvertFunc convertor;
};

StreamConvertor g_streamConvertors[] = {{AV_CODEC_ID_PCM_S16LE, ConvertRawAudioStreamToMetaInfo},
                                        {AV_CODEC_ID_PCM_S16BE, ConvertRawAudioStreamToMetaInfo},
                                        {AV_CODEC_ID_PCM_U16LE, ConvertRawAudioStreamToMetaInfo},
                                        {AV_CODEC_ID_PCM_U16BE, ConvertRawAudioStreamToMetaInfo},
                                        {AV_CODEC_ID_PCM_F32LE, ConvertRawAudioStreamToMetaInfo},
                                        {AV_CODEC_ID_PCM_S8, ConvertRawAudioStreamToMetaInfo},
                                        {AV_CODEC_ID_PCM_U8, ConvertRawAudioStreamToMetaInfo},
                                        {AV_CODEC_ID_MP1, ConvertMP1StreamToMetaInfo},
                                        {AV_CODEC_ID_MP2, ConvertMP2StreamToMetaInfo},
                                        {AV_CODEC_ID_MP3, ConvertMP3StreamToMetaInfo},
                                        {AV_CODEC_ID_AAC, ConvertAACStreamToMetaInfo},
                                        {AV_CODEC_ID_AAC_LATM, ConvertAACLatmStreamToMetaInfo},
                                        {AV_CODEC_ID_VORBIS, ConvertVorbisStreamToMetaInfo},
                                        {AV_CODEC_ID_FLAC, ConvertFLACStreamToMetaInfo},
                                        {AV_CODEC_ID_APE, ConvertAPEStreamToMetaInfo},
#ifdef VIDEO_SUPPORT
                                        {AV_CODEC_ID_H264, ConvertAVCStreamToMetaInfo}
#endif
};

bool IsPcmStream(const AVStream& avStream)
{
    auto codecId = avStream.codecpar->codec_id;
    return codecId == AV_CODEC_ID_PCM_S16LE || codecId == AV_CODEC_ID_PCM_S16BE || codecId == AV_CODEC_ID_PCM_U16LE ||
           codecId == AV_CODEC_ID_PCM_U16BE || codecId == AV_CODEC_ID_PCM_S8 || codecId == AV_CODEC_ID_PCM_U8 ||
           codecId == AV_CODEC_ID_PCM_F32LE;
}

void ConvertCommonAudioStreamToMetaInfo(const AVStream& avStream,
                                        const std::shared_ptr<AVFormatContext>& avFormatContext,
                                        const std::shared_ptr<AVCodecContext>& avCodecContext, TagMap& meta)
{
    meta.insert({Tag::TRACK_ID, static_cast<uint32_t>(avStream.index)});
    if (avCodecContext->channels != -1) {
        meta.insert({Tag::AUDIO_SAMPLE_RATE, static_cast<uint32_t>(avCodecContext->sample_rate)});
        meta.insert({Tag::AUDIO_CHANNELS, static_cast<uint32_t>(avCodecContext->channels)});
        meta.insert({Tag::AUDIO_CHANNEL_LAYOUT,
                     ConvertChannelLayoutFromFFmpeg(avCodecContext->channels, avCodecContext->channel_layout)});
        // ffmpeg defaults to 1024 samples per frame for planar PCM in each buffer (one for each channel).
        uint32_t samplesPerFrame = 1024;
        if (!IsPcmStream(avStream) && avCodecContext->frame_size != 0) {
            samplesPerFrame = static_cast<uint32_t>(avCodecContext->frame_size);
        }
        meta.insert({Tag::AUDIO_SAMPLE_PER_FRAME, samplesPerFrame});
        meta.insert({Tag::AUDIO_SAMPLE_FORMAT, ConvFf2PSampleFmt(avCodecContext->sample_fmt)});
        int64_t bitRate = avCodecContext->bit_rate;
        if (!bitRate) {
            bitRate = avFormatContext->bit_rate;
        }
        meta.insert({Tag::MEDIA_BITRATE, bitRate});
    }
    if (avCodecContext->extradata_size > 0) {
        CodecConfig codecConfig;
        codecConfig.assign(avCodecContext->extradata, avCodecContext->extradata + avCodecContext->extradata_size);
        meta.insert({Tag::MEDIA_CODEC_CONFIG, std::move(codecConfig)});
    }
    meta.insert({Tag::BITS_PER_CODED_SAMPLE, static_cast<uint32_t>(avCodecContext->bits_per_coded_sample)});
}
} // namespace

void ConvertRawAudioStreamToMetaInfo(const AVStream& avStream, const std::shared_ptr<AVFormatContext>& avFormatContext,
                                     const std::shared_ptr<AVCodecContext>& avCodecContext, TagMap& meta)
{
    meta.insert({Tag::MIME, std::string(MEDIA_MIME_AUDIO_RAW)});
    ConvertCommonAudioStreamToMetaInfo(avStream, avFormatContext, avCodecContext, meta);
}

void ConvertMP1StreamToMetaInfo(const AVStream& avStream, const std::shared_ptr<AVFormatContext>& avFormatContext,
                                const std::shared_ptr<AVCodecContext>& avCodecContext, TagMap& meta)
{
    meta.insert({Tag::MIME, std::string(MEDIA_MIME_AUDIO_MPEG)});
    ConvertCommonAudioStreamToMetaInfo(avStream, avFormatContext, avCodecContext, meta);
    meta.insert({Tag::AUDIO_MPEG_VERSION, static_cast<uint32_t>(1)});
    meta.insert({Tag::AUDIO_MPEG_LAYER, static_cast<uint32_t>(1)});
}

void ConvertMP2StreamToMetaInfo(const AVStream& avStream, const std::shared_ptr<AVFormatContext>& avFormatContext,
                                const std::shared_ptr<AVCodecContext>& avCodecContext, TagMap& meta)
{
    meta.insert({Tag::MIME, std::string(MEDIA_MIME_AUDIO_MPEG)});
    ConvertCommonAudioStreamToMetaInfo(avStream, avFormatContext, avCodecContext, meta);
    meta.insert({Tag::AUDIO_MPEG_VERSION, static_cast<uint32_t>(1)});
    meta.insert({Tag::AUDIO_MPEG_LAYER, static_cast<uint32_t>(2)}); // 2
}

void ConvertMP3StreamToMetaInfo(const AVStream& avStream, const std::shared_ptr<AVFormatContext>& avFormatContext,
                                const std::shared_ptr<AVCodecContext>& avCodecContext, TagMap& meta)
{
    meta.insert({Tag::MIME, std::string(MEDIA_MIME_AUDIO_MPEG)});
    ConvertCommonAudioStreamToMetaInfo(avStream, avFormatContext, avCodecContext, meta);
    meta.insert({Tag::AUDIO_MPEG_VERSION, static_cast<uint32_t>(1)});
    meta.insert({Tag::AUDIO_MPEG_LAYER, static_cast<uint32_t>(3)}); // 3
}

void ConvertFLACStreamToMetaInfo(const AVStream& avStream, const std::shared_ptr<AVFormatContext>& avFormatContext,
                                 const std::shared_ptr<AVCodecContext>& avCodecContext, TagMap& meta)
{
    meta.insert({Tag::MIME, std::string(MEDIA_MIME_AUDIO_FLAC)});
    ConvertCommonAudioStreamToMetaInfo(avStream, avFormatContext, avCodecContext, meta);
}

void ConvertAPEStreamToMetaInfo(const AVStream& avStream, const std::shared_ptr<AVFormatContext>& avFormatContext,
                                const std::shared_ptr<AVCodecContext>& avCodecContext, TagMap& meta)
{
    meta.insert({Tag::MIME, std::string(MEDIA_MIME_AUDIO_APE)});
    ConvertCommonAudioStreamToMetaInfo(avStream, avFormatContext, avCodecContext, meta);
}

void ConvertVorbisStreamToMetaInfo(const AVStream& avStream, const std::shared_ptr<AVFormatContext>& avFormatContext,
                                   const std::shared_ptr<AVCodecContext>& avCodecContext, TagMap& meta)
{
    meta.insert({Tag::MIME, std::string(MEDIA_MIME_AUDIO_VORBIS)});
    ConvertCommonAudioStreamToMetaInfo(avStream, avFormatContext, avCodecContext, meta);
}

void ConvertAACStreamToMetaInfo(const AVStream& avStream, const std::shared_ptr<AVFormatContext>& avFormatContext,
                                const std::shared_ptr<AVCodecContext>& avCodecContext, TagMap& meta)
{
    meta.insert({Tag::MIME, std::string(MEDIA_MIME_AUDIO_AAC)});
    ConvertCommonAudioStreamToMetaInfo(avStream, avFormatContext, avCodecContext, meta);
    meta.insert({Tag::AUDIO_MPEG_VERSION, static_cast<uint32_t>(4)}); // 4
    meta.insert({Tag::AUDIO_AAC_PROFILE, AudioAacProfile::LC});
    if (avCodecContext->extradata_size > 0) {
        std::vector<uint8_t> codecConfig;
        codecConfig.assign(avCodecContext->extradata, avCodecContext->extradata + avCodecContext->extradata_size);
        meta.insert({Tag::MEDIA_CODEC_CONFIG, std::move(codecConfig)});
        AACAudioConfigParser parser(avCodecContext->extradata, avCodecContext->extradata_size);
        if (!parser.ParseConfigs()) {
            return;
        }
        meta.insert({Tag::AUDIO_AAC_LEVEL, parser.GetLevel()});
        auto profile = parser.GetProfile();
        if (profile != AudioAacProfile::NONE) {
            meta.insert({Tag::AUDIO_AAC_PROFILE, profile});
        }
    } else {
        meta.insert({Tag::AUDIO_AAC_STREAM_FORMAT, AudioAacStreamFormat::MP4ADTS});
    }
}

void ConvertAACLatmStreamToMetaInfo(const AVStream& avStream, const std::shared_ptr<AVFormatContext>& avFormatContext,
                                    const std::shared_ptr<AVCodecContext>& avCodecContext, TagMap& meta)
{
    meta.insert({Tag::MIME, std::string(MEDIA_MIME_AUDIO_AAC_LATM)});
    meta.insert({Tag::TRACK_ID, static_cast<uint32_t>(avStream.index)});
    if (avCodecContext->channels != -1) {
        meta.insert({Tag::AUDIO_SAMPLE_RATE, static_cast<uint32_t>(avCodecContext->sample_rate)});
        meta.insert({Tag::AUDIO_CHANNELS, static_cast<uint32_t>(avCodecContext->channels)});
        meta.insert({Tag::AUDIO_SAMPLE_FORMAT, ConvFf2PSampleFmt(avCodecContext->sample_fmt)});
        meta.insert({Tag::AUDIO_CHANNEL_LAYOUT,
                     ConvertChannelLayoutFromFFmpeg(avCodecContext->channels,  avCodecContext->channel_layout)});
        meta.insert({Tag::AUDIO_SAMPLE_PER_FRAME, static_cast<uint32_t>(avCodecContext->frame_size)});
        int64_t bitRate = avCodecContext->bit_rate;
        if (!bitRate) {
            bitRate = avFormatContext->bit_rate;
        }
        meta.insert({Tag::MEDIA_BITRATE, bitRate});
    }
    meta.insert({Tag::AUDIO_MPEG_VERSION, static_cast<uint32_t>(4)}); // 4
    meta.insert({Tag::AUDIO_AAC_STREAM_FORMAT, AudioAacStreamFormat::MP4LOAS});
    meta.insert({Tag::BITS_PER_CODED_SAMPLE, static_cast<uint32_t>(avCodecContext->bits_per_coded_sample)});
}

#ifdef VIDEO_SUPPORT
void ConvertAVCStreamToMetaInfo(const AVStream& avStream, const std::shared_ptr<AVFormatContext>& avFormatContext,
                                const std::shared_ptr<AVCodecContext>& avCodecContext, TagMap& meta)
{
    meta.insert({Tag::MIME, std::string(MEDIA_MIME_VIDEO_H264)});
    meta.insert({Tag::TRACK_ID, static_cast<uint32_t>(avStream.index)});
    int64_t bitRate = avCodecContext->bit_rate;
    if (!bitRate) {
        bitRate = avFormatContext->bit_rate;
    }
    meta.insert({Tag::MEDIA_BITRATE, bitRate});
    meta.insert({Tag::BITS_PER_CODED_SAMPLE, static_cast<uint32_t>(avCodecContext->bits_per_coded_sample)});
    meta.insert({Tag::VIDEO_WIDTH, static_cast<uint32_t>(avCodecContext->width)});
    meta.insert({Tag::VIDEO_HEIGHT, static_cast<uint32_t>(avCodecContext->height)});
    if (avCodecContext->extradata_size > 0) {
        AVCConfigDataParser parser(avCodecContext->extradata, avCodecContext->extradata_size);
        if (!parser.ParseConfigData()) {
            return;
        }
        std::shared_ptr<uint8_t> cfgData = nullptr;
        size_t cfgDataSize = 0;
        if (parser.GetNewConfigData(cfgData, cfgDataSize) && (cfgData != nullptr) && (cfgDataSize != 0)) {
            std::vector<uint8_t> codecConfig;
            codecConfig.assign(cfgData.get(), cfgData.get() + cfgDataSize);
            meta.insert({Tag::MEDIA_CODEC_CONFIG, std::move(codecConfig)});
        }
    }
}
#endif
void ConvertAVStreamToMetaInfo(const AVStream& avStream, const std::shared_ptr<AVFormatContext>& avFormatContext,
                               const std::shared_ptr<AVCodecContext>& avCodecContext, TagMap& meta)
{
    meta.clear();
    auto codecId = avStream.codecpar->codec_id;
    for (auto& streamConvertor : g_streamConvertors) {
        if (streamConvertor.codecId == codecId) {
            streamConvertor.convertor(avStream, avFormatContext, avCodecContext, meta);
            return;
        }
    }
    MEDIA_LOG_E("unsupported codec id: " PUBLIC_LOG_D32, codecId);
}
} // namespace Ffmpeg
} // namespace Plugin
} // namespace Media
} // namespace OHOS
