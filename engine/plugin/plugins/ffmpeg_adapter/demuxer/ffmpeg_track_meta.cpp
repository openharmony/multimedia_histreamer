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

#include "ffmpeg_track_meta.h"
#include "foundation/log.h"
#include "utils/constants.h"
#include "utils/type_define.h"
#include "plugins/ffmpeg_adapter/utils/aac_audio_config_parser.h"
#include "plugins/ffmpeg_adapter/utils/ffmpeg_utils.h"
#ifdef VIDEO_SUPPORT
#include "plugins/ffmpeg_adapter/utils/avc_config_data_parser.h"
#endif

namespace OHOS {
namespace Media {
namespace Plugin {
namespace {
using ConvertFunc = void (*)(const AVStream& avStream, const std::shared_ptr<AVCodecContext>& context, TagMap& meta);

struct StreamConvertor {
    AVCodecID codecId;
    ConvertFunc convertor;
};

StreamConvertor g_streamConvertors[] = {{AV_CODEC_ID_PCM_S16LE, ConvertRawAudioStreamToMetaInfo},
                                        {AV_CODEC_ID_PCM_S16BE, ConvertRawAudioStreamToMetaInfo},
                                        {AV_CODEC_ID_PCM_U16LE, ConvertRawAudioStreamToMetaInfo},
                                        {AV_CODEC_ID_PCM_U16BE, ConvertRawAudioStreamToMetaInfo},
                                        {AV_CODEC_ID_PCM_S8, ConvertRawAudioStreamToMetaInfo},
                                        {AV_CODEC_ID_PCM_U8, ConvertRawAudioStreamToMetaInfo},
                                        {AV_CODEC_ID_MP1, ConvertMP1StreamToMetaInfo},
                                        {AV_CODEC_ID_MP2, ConvertMP2StreamToMetaInfo},
                                        {AV_CODEC_ID_MP3, ConvertMP3StreamToMetaInfo},
                                        {AV_CODEC_ID_AAC, ConvertAACStreamToMetaInfo},
                                        {AV_CODEC_ID_AAC_LATM, ConvertAACLatmStreamToMetaInfo},
#ifdef VIDEO_SUPPORT
                                        {AV_CODEC_ID_H264, ConvertAVCStreamToMetaInfo}
#endif
};

bool IsPcmStream(const AVStream& avStream)
{
    auto codecId = avStream.codecpar->codec_id;
    return codecId == AV_CODEC_ID_PCM_S16LE || codecId == AV_CODEC_ID_PCM_S16BE || codecId == AV_CODEC_ID_PCM_U16LE ||
           codecId == AV_CODEC_ID_PCM_U16BE || codecId == AV_CODEC_ID_PCM_S8 || codecId == AV_CODEC_ID_PCM_U8;
}

void ConvertCommonAudioStreamToMetaInfo(const AVStream& avStream, const std::shared_ptr<AVCodecContext>& context,
                                        TagMap& meta)
{
    meta.insert({Tag::TRACK_ID, static_cast<uint32_t>(avStream.index)});
    if (context->channels != -1) {
        meta.insert({Tag::AUDIO_SAMPLE_RATE, static_cast<uint32_t>(context->sample_rate)});
        meta.insert({Tag::AUDIO_CHANNELS, static_cast<uint32_t>(context->channels)});
        meta.insert(
            {Tag::AUDIO_CHANNEL_LAYOUT, ConvertChannelLayoutFromFFmpeg(context->channels, context->channel_layout)});
        // ffmpeg defaults to 1024 samples per frame for planar PCM in each buffer (one for each channel).
        uint32_t samplesPerFrame = 1024;
        if (!IsPcmStream(avStream)) {
            samplesPerFrame = static_cast<uint32_t>(context->frame_size);
        }
        meta.insert({Tag::AUDIO_SAMPLE_PER_FRAME, samplesPerFrame});
        meta.insert({Tag::AUDIO_SAMPLE_FORMAT, Trans2Format(context->sample_fmt)});
        meta.insert({Tag::MEDIA_BITRATE, static_cast<int64_t>(context->bit_rate)});
    }
}
} // namespace

void ConvertRawAudioStreamToMetaInfo(const AVStream& avStream, const std::shared_ptr<AVCodecContext>& context,
                                     TagMap& meta)
{
    meta.insert({Tag::MIME, std::string(MEDIA_MIME_AUDIO_RAW)});
    ConvertCommonAudioStreamToMetaInfo(avStream, context, meta);
}

void ConvertMP1StreamToMetaInfo(const AVStream& avStream, const std::shared_ptr<AVCodecContext>& context, TagMap& meta)
{
    meta.insert({Tag::MIME, std::string(MEDIA_MIME_AUDIO_MPEG)});
    ConvertCommonAudioStreamToMetaInfo(avStream, context, meta);
    meta.insert({Tag::AUDIO_MPEG_VERSION, static_cast<uint32_t>(1)});
    meta.insert({Tag::AUDIO_MPEG_LAYER, static_cast<uint32_t>(1)});
}

void ConvertMP2StreamToMetaInfo(const AVStream& avStream, const std::shared_ptr<AVCodecContext>& context, TagMap& meta)
{
    meta.insert({Tag::MIME, std::string(MEDIA_MIME_AUDIO_MPEG)});
    ConvertCommonAudioStreamToMetaInfo(avStream, context, meta);
    meta.insert({Tag::AUDIO_MPEG_VERSION, static_cast<uint32_t>(1)});
    meta.insert({Tag::AUDIO_MPEG_LAYER, static_cast<uint32_t>(2)}); // 2
}

void ConvertMP3StreamToMetaInfo(const AVStream& avStream, const std::shared_ptr<AVCodecContext>& context, TagMap& meta)
{
    meta.insert({Tag::MIME, std::string(MEDIA_MIME_AUDIO_MPEG)});
    ConvertCommonAudioStreamToMetaInfo(avStream, context, meta);
    meta.insert({Tag::AUDIO_MPEG_VERSION, static_cast<uint32_t>(1)});
    meta.insert({Tag::AUDIO_MPEG_LAYER, static_cast<uint32_t>(3)}); // 3
}

void ConvertAACStreamToMetaInfo(const AVStream& avStream, const std::shared_ptr<AVCodecContext>& context, TagMap& meta)
{
    meta.insert({Tag::MIME, std::string(MEDIA_MIME_AUDIO_AAC)});
    ConvertCommonAudioStreamToMetaInfo(avStream, context, meta);
    meta.insert({Tag::AUDIO_MPEG_VERSION, static_cast<uint32_t>(4)}); // 4
    meta.insert({Tag::AUDIO_AAC_PROFILE, AudioAacProfile::LC});
    if (context->extradata_size > 0) {
        std::vector<uint8_t> codecConfig;
        codecConfig.assign(context->extradata, context->extradata + context->extradata_size);
        meta.insert({Tag::MEDIA_CODEC_CONFIG, std::move(codecConfig)});
        AACAudioConfigParser parser(context->extradata, context->extradata_size);
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

void ConvertAACLatmStreamToMetaInfo(const AVStream& avStream, const std::shared_ptr<AVCodecContext>& context,
                                    TagMap& meta)
{
    meta.insert({Tag::MIME, std::string(MEDIA_MIME_AUDIO_AAC_LATM)});
    meta.insert({Tag::TRACK_ID, static_cast<uint32_t>(avStream.index)});
    if (context->channels != -1) {
        meta.insert({Tag::AUDIO_SAMPLE_RATE, static_cast<uint32_t>(context->sample_rate)});
        meta.insert({Tag::AUDIO_CHANNELS, static_cast<uint32_t>(context->channels)});
        meta.insert({Tag::AUDIO_SAMPLE_FORMAT, Trans2Format(context->sample_fmt)});
        meta.insert(
            {Tag::AUDIO_CHANNEL_LAYOUT, ConvertChannelLayoutFromFFmpeg(context->channels, context->channel_layout)});
        meta.insert({Tag::AUDIO_SAMPLE_PER_FRAME, static_cast<uint32_t>(context->frame_size)});
        meta.insert({Tag::MEDIA_BITRATE, static_cast<int64_t>(context->bit_rate)});
    }
    meta.insert({Tag::AUDIO_MPEG_VERSION, static_cast<uint32_t>(4)}); // 4
    meta.insert({Tag::AUDIO_AAC_STREAM_FORMAT, AudioAacStreamFormat::MP4LOAS});
}

#ifdef VIDEO_SUPPORT
void ConvertAVCStreamToMetaInfo(const AVStream& avStream, const std::shared_ptr<AVCodecContext>& context, TagMap& meta)
{
    meta.insert({Tag::MIME, std::string(MEDIA_MIME_VIDEO_AVC)});
    meta.insert({Tag::TRACK_ID, static_cast<uint32_t>(avStream.index)});
    meta.insert({Tag::MEDIA_BITRATE, context->bit_rate});
    meta.insert({Tag::VIDEO_WIDTH, static_cast<uint32_t>(context->width)});
    meta.insert({Tag::VIDEO_HEIGHT, static_cast<uint32_t>(context->height)});
    if (context->extradata_size > 0) {
        AVCConfigDataParser parser(context->extradata, context->extradata_size);
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

void ConvertAVStreamToMetaInfo(const AVStream& avStream, const std::shared_ptr<AVCodecContext>& context, TagMap& meta)
{
    meta.clear();
    auto codecId = avStream.codecpar->codec_id;
    for (auto& streamConvertor : g_streamConvertors) {
        if (streamConvertor.codecId == codecId) {
            streamConvertor.convertor(avStream, context, meta);
            return;
        }
    }
    MEDIA_LOG_E("unsupported codec id: %d", codecId);
}
} // namespace Plugin
} // namespace Media
} // namespace OHOS
