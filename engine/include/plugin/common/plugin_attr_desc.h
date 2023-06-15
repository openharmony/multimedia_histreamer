/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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

#ifndef HISTREAMER_PIPELINE_PLUGIN_CAP_DESC_H
#define HISTREAMER_PIPELINE_PLUGIN_CAP_DESC_H
#include <tuple>
#include "plugin/common/plugin_audio_tags.h"
#include "plugin/common/plugin_source_tags.h"
#include "plugin/common/plugin_video_tags.h"

namespace OHOS {
namespace Media {
namespace Plugin {
const ValueType g_emptyString = std::string();
const ValueType g_u8Def = (uint8_t)0;
const ValueType g_u32Def = (uint32_t)0;
const ValueType g_d32Def = (int32_t)0;
const ValueType g_d64Def = (int64_t)0;
const ValueType g_u64Def = (uint64_t)0;
const ValueType g_doubleDef = (double)0.0;
const ValueType g_seekableDef = Seekable::INVALID;
const ValueType g_srcInputTypedef = SrcInputType::UNKNOWN;
const ValueType g_unknown = nullptr;
const ValueType g_vecBufDef = std::vector<uint8_t>();
const ValueType g_mediaTypeDef = MediaType::AUDIO;
const ValueType g_channelLayoutDef = AudioChannelLayout::MONO;
const ValueType g_auSampleFmtDef = AudioSampleFormat::U8;
const ValueType g_aacProfileDef = AudioAacProfile::LC;
const ValueType g_aacStFmtDef = AudioAacStreamFormat::RAW;
const ValueType g_vdPixelFmtDef = VideoPixelFormat::UNKNOWN;
const ValueType g_vdBitStreamFmtDef = VideoBitStreamFormat::UNKNOWN;
const ValueType g_vdH264ProfileDef = VideoH264Profile::BASELINE;
const ValueType g_audioRenderInfoDef = AudioRenderInfo {};
const ValueType g_audioInterruptModeDef = AudioInterruptMode::SHARE_MODE;

// tuple is <tagName, default_val, typeName> default_val is used for type compare
const std::map<Tag, std::tuple<const char*, const ValueType&, const char*>> g_tagInfoMap = {
    {Tag::MIME, {"mime",                               g_emptyString,      "string"}},
    {Tag::TRACK_ID, {"track_id",                       g_u32Def,           "uint32_t"}},
    {Tag::REQUIRED_OUT_BUFFER_CNT, {"req_out_buf_cnt", g_u32Def,           "uint32_t"}},
    {Tag::BUFFER_ALLOCATOR, {"buf_allocator",          g_unknown,          "shared_ptr<Allocator>"}},
    {Tag::BUFFERING_SIZE, {"bufing_size",              g_u32Def,           "uint32_t"}},
    {Tag::WATERLINE_HIGH, {"waterline_h",              g_u32Def,           "uint32_t"}},
    {Tag::WATERLINE_LOW, {"waterline_l",               g_u32Def,           "uint32_t"}},
    {Tag::SRC_INPUT_TYPE, {"src_input_typ",            g_srcInputTypedef,  "SrcInputType"}},
    {Tag::MEDIA_TYPE, {"media_type",                   g_mediaTypeDef,      "MediaType"}},
    {Tag::MEDIA_TITLE, {"title",                       g_emptyString,      "string"}},
    {Tag::MEDIA_ARTIST, {"artist",                     g_emptyString,      "string"}},
    {Tag::MEDIA_LYRICIST, {"lyricist",                 g_emptyString,      "string"}},
    {Tag::MEDIA_ALBUM, {"album",                       g_emptyString,      "string"}},
    {Tag::MEDIA_ALBUM_ARTIST, {"album_artist",         g_emptyString,      "string"}},
    {Tag::MEDIA_DATE, {"date",                         g_emptyString,      "string"}},
    {Tag::MEDIA_COMMENT, {"comment",                   g_emptyString,      "string"}},
    {Tag::MEDIA_GENRE, {"genre",                       g_emptyString,      "string"}},
    {Tag::MEDIA_COPYRIGHT, {"copyright",               g_emptyString,      "string"}},
    {Tag::MEDIA_LANGUAGE, {"lang",                     g_emptyString,      "string"}},
    {Tag::MEDIA_DESCRIPTION, {"media_desc",            g_emptyString,      "string"}},
    {Tag::MEDIA_LYRICS, {"lyrics",                     g_emptyString,      "string"}},
    {Tag::MEDIA_DURATION, {"duration",                 g_d64Def,           "int64_t"}},
    {Tag::MEDIA_FILE_SIZE, {"file_size",               g_u64Def,           "uint64_t"}},
    {Tag::MEDIA_SEEKABLE, {"media_seekable",           g_seekableDef,      "Seekable"}},
    {Tag::MEDIA_BITRATE, {"bit_rate",                  g_d64Def,           "int64_t"}},
    {Tag::MEDIA_FILE_URI, {"file_uri",                 g_emptyString,      "string"}},
    {Tag::MEDIA_CODEC_CONFIG, {"codec_config",         g_vecBufDef,        "std::vector<uint8_t>"}},
    {Tag::MEDIA_POSITION, {"position",                 g_u64Def,           "uint64_t"}},
    {Tag::AUDIO_CHANNELS, {"channels",                 g_u32Def,           "uint32_t"}},
    {Tag::AUDIO_CHANNEL_LAYOUT, {"channel_layout",     g_channelLayoutDef, "AudioChannelLayout"}},
    {Tag::AUDIO_SAMPLE_RATE, {"sample_rate",           g_u32Def,           "uint32_t"}},
    {Tag::AUDIO_SAMPLE_FORMAT, {"sample_fmt",          g_auSampleFmtDef,   "AudioSampleFormat"}},
    {Tag::AUDIO_SAMPLE_PER_FRAME, {"sample_per_frame", g_u32Def,           "uint32_t"}},
    {Tag::AUDIO_OUTPUT_CHANNELS, {"output_channels",   g_u32Def,           "uint32_t"}},
    {Tag::AUDIO_OUTPUT_CHANNEL_LAYOUT, {"output_channel_layout", g_channelLayoutDef, "AudioChannelLayout"}},
    {Tag::AUDIO_MPEG_VERSION, {"ad_mpeg_ver",          g_u32Def,           "uint32_t"}},
    {Tag::AUDIO_MPEG_LAYER, {"ad_mpeg_layer",          g_u32Def,           "uint32_t"}},

    {Tag::AUDIO_AAC_PROFILE, {"aac_profile",           g_aacProfileDef,    "AudioAacProfile"}},
    {Tag::AUDIO_AAC_LEVEL, {"aac_level",               g_u32Def,           "uint32_t"}},
    {Tag::AUDIO_AAC_STREAM_FORMAT, {"aac_stm_fmt",     g_aacStFmtDef,      "AudioAacStreamFormat"}},
    {Tag::VIDEO_WIDTH, {"vd_w",                        g_u32Def,           "uint32_t"}},
    {Tag::VIDEO_HEIGHT, {"vd_h",                       g_u32Def,           "uint32_t"}},
    {Tag::VIDEO_PIXEL_FORMAT, {"pixel_fmt",            g_vdPixelFmtDef,    "VideoPixelFormat"}},
    {Tag::VIDEO_FRAME_RATE, {"frm_rate",               g_u32Def,           "uint32_t"}},
    {Tag::VIDEO_SURFACE, {"surface",                   g_unknown,          "Surface"}},
    {Tag::VIDEO_MAX_SURFACE_NUM, {"surface_num",       g_u32Def,           "uint32_t"}},
    {Tag::VIDEO_CAPTURE_RATE, {"capture_rate",         g_doubleDef,        "double"}},
    {Tag::VIDEO_BIT_STREAM_FORMAT, {"vd_bit_stream_fmt", g_vdBitStreamFmtDef, "VideoBitStreamFormat"}},
    {Tag::BITS_PER_CODED_SAMPLE, {"bits_per_coded_sample", g_u32Def,       "uint32_t"}},
    {Tag::MEDIA_START_TIME, {"med_start_time",         g_d64Def,           "int64_t"}},
    {Tag::VIDEO_H264_PROFILE, {"h264_profile",         g_vdH264ProfileDef, "VideoH264Profile"}},
    {Tag::VIDEO_H264_LEVEL, {"vd_level",               g_u32Def,           "uint32_t"}},
    {Tag::APP_TOKEN_ID, {"apptoken_id",                g_u32Def,           "uint32_t"}},
    {Tag::APP_UID, {"app_uid",                         g_d32Def,           "int32_t"}},
    {Tag::APP_PID, {"app_pid",                         g_d32Def,           "int32_t"}},
    {Tag::AUDIO_RENDER_INFO, {"audio_render_info",     g_audioRenderInfoDef, "AudioRenderInfo"}},
    {Tag::AUDIO_INTERRUPT_MODE, {"audio_interrupt_mode", g_audioInterruptModeDef, "AudioInterruptMode"}},
    {Tag::USER_FRAME_NUMBER, {"frame_number",          g_u32Def,            "uint32_t"}},
    {Tag::USER_TIME_SYNC_RESULT, {"time_sync_result",  g_emptyString,       "string"}},
    {Tag::USER_AV_SYNC_GROUP_INFO, {"av_sync_group_info",   g_emptyString,  "string"}},
    {Tag::USER_SHARED_MEMORY_FD, {"shared_memory_fd",  g_emptyString,       "string"}},
};

const std::map<AudioSampleFormat, const char*> g_auSampleFmtStrMap = {
    {AudioSampleFormat::S8, "S8"},
    {AudioSampleFormat::U8, "U8"},
    {AudioSampleFormat::S8P, "S8P"},
    {AudioSampleFormat::U8P, "U8P"},
    {AudioSampleFormat::S16, "S16"},
    {AudioSampleFormat::U16, "U16"},
    {AudioSampleFormat::S16P, "S16P"},
    {AudioSampleFormat::U16P, "U16P"},
    {AudioSampleFormat::S24, "S24"},
    {AudioSampleFormat::U24, "U24"},
    {AudioSampleFormat::S24P, "S24P"},
    {AudioSampleFormat::U24P, "U24P"},
    {AudioSampleFormat::S32, "S32"},
    {AudioSampleFormat::U32, "U32"},
    {AudioSampleFormat::S32P, "S32P"},
    {AudioSampleFormat::U32P, "U32P"},
    {AudioSampleFormat::S64, "S64"},
    {AudioSampleFormat::U64, "U64"},
    {AudioSampleFormat::S64P, "S64P"},
    {AudioSampleFormat::U64P, "U64P"},
    {AudioSampleFormat::F32, "F32"},
    {AudioSampleFormat::F32P, "F32P"},
    {AudioSampleFormat::F64, "F64"},
    {AudioSampleFormat::F64P, "F64P"},
};

const std::map<AudioChannelLayout, const char*> g_auChannelLayoutStrMap = {
    {AudioChannelLayout::UNKNOWN, "UNKNOWN"},
    {AudioChannelLayout::MONO, "MONO"},
    {AudioChannelLayout::STEREO, "STEREO"},
    {AudioChannelLayout::CH_2POINT1, "CH_2POINT1"},
    {AudioChannelLayout::CH_2_1, "CH_2_1"},
    {AudioChannelLayout::SURROUND, "SURROUND"},
    {AudioChannelLayout::CH_3POINT1, "CH_3POINT1"},
    {AudioChannelLayout::CH_4POINT0, "CH_4POINT0"},
    {AudioChannelLayout::CH_4POINT1, "CH_4POINT1"},
    {AudioChannelLayout::CH_2_2, "CH_2_2"},
    {AudioChannelLayout::QUAD, "QUAD"},
    {AudioChannelLayout::CH_5POINT0, "CH_5POINT0"},
    {AudioChannelLayout::CH_5POINT1, "CH_5POINT1"},
    {AudioChannelLayout::CH_5POINT0_BACK, "CH_5POINT0_BACK"},
    {AudioChannelLayout::CH_5POINT1_BACK, "CH_5POINT1_BACK"},
    {AudioChannelLayout::CH_6POINT0, "CH_6POINT0"},
    {AudioChannelLayout::CH_6POINT0_FRONT, "CH_6POINT0_FRONT"},
    {AudioChannelLayout::HEXAGONAL, "HEXAGONAL"},
    {AudioChannelLayout::CH_6POINT1, "CH_6POINT1"},
    {AudioChannelLayout::CH_6POINT1_BACK, "CH_6POINT1_BACK"},
    {AudioChannelLayout::CH_6POINT1_FRONT, "CH_6POINT1_FRONT"},
    {AudioChannelLayout::CH_7POINT0, "CH_7POINT0"},
    {AudioChannelLayout::CH_7POINT0_FRONT, "CH_7POINT0_FRONT"},
    {AudioChannelLayout::CH_7POINT1, "CH_7POINT1"},
    {AudioChannelLayout::CH_7POINT1_WIDE, "CH_7POINT1_WIDE"},
    {AudioChannelLayout::CH_7POINT1_WIDE_BACK, "CH_7POINT1_WIDE_BACK"},
#ifdef AVS3DA_SUPPORT
    {AudioChannelLayout::CH_5POINT1POINT2, "CH_5POINT1POINT2"},
    {AudioChannelLayout::CH_5POINT1POINT4, "CH_5POINT1POINT4"},
    {AudioChannelLayout::CH_7POINT1POINT2, "CH_7POINT1POINT2"},
    {AudioChannelLayout::CH_7POINT1POINT4, "CH_7POINT1POINT4"},
    {AudioChannelLayout::CH_9POINT1POINT4, "CH_9POINT1POINT4"},
    {AudioChannelLayout::CH_9POINT1POINT6, "CH_9POINT1POINT6"},
    {AudioChannelLayout::CH_10POINT2, "CH_10POINT2"},
    {AudioChannelLayout::CH_22POINT2, "CH_22POINT2"},
#endif
    {AudioChannelLayout::OCTAGONAL, "OCTAGONAL"},
    {AudioChannelLayout::HEXADECAGONAL, "HEXADECAGONAL"},
    {AudioChannelLayout::STEREO_DOWNMIX, "STEREO_DOWNMIX"},
};

const std::map<VideoPixelFormat, const char*> g_videoPixelFormatStrMap = {
    {VideoPixelFormat::UNKNOWN, "UNKNOWN"},
    {VideoPixelFormat::YUV410P, "YUV410P"},
    {VideoPixelFormat::YUV411P, "YUV411P"},
    {VideoPixelFormat::YUV420P, "YUV420P"},
    {VideoPixelFormat::NV12, "NV12"},
    {VideoPixelFormat::NV21, "NV21"},
    {VideoPixelFormat::YUYV422, "YUYV422"},
    {VideoPixelFormat::YUV422P, "YUV422P"},
    {VideoPixelFormat::YUV444P, "YUV444P"},
    {VideoPixelFormat::RGBA, "RGBA"},
    {VideoPixelFormat::ARGB, "ARGB"},
    {VideoPixelFormat::ABGR, "ABGR"},
    {VideoPixelFormat::BGRA, "BGRA"},
    {VideoPixelFormat::RGB24, "RGB24"},
    {VideoPixelFormat::BGR24, "BGR24"},
    {VideoPixelFormat::PAL8, "PAL8"},
    {VideoPixelFormat::GRAY8, "GRAY8"},
    {VideoPixelFormat::MONOWHITE, "MONOWHITE"},
    {VideoPixelFormat::MONOBLACK, "MONOBLACK"},
    {VideoPixelFormat::YUVJ420P, "YUVJ420P"},
    {VideoPixelFormat::YUVJ422P, "YUVJ422P"},
    {VideoPixelFormat::YUVJ444P, "YUVJ444P"},
};

const std::map<VideoBitStreamFormat, const char*> g_vdBitStreamFormatStrMap = {
    {VideoBitStreamFormat::UNKNOWN, "UNKNOWN"},
    {VideoBitStreamFormat::AVC1, "AVC1"},
    {VideoBitStreamFormat::HEVC, "HEVC"},
    {VideoBitStreamFormat::ANNEXB, "ANNEXB"},
};

const std::map<AudioAacProfile, const char*> g_auAacProfileNameStrMap = {
    {AudioAacProfile::NONE, "NONE"},
    {AudioAacProfile::MAIN, "MAIN"},
    {AudioAacProfile::LC, "LC"},
    {AudioAacProfile::SSR, "SSR"},
    {AudioAacProfile::LTP, "LTP"},
    {AudioAacProfile::HE, "HE"},
    {AudioAacProfile::SCALABLE, "SCALABLE"},
    {AudioAacProfile::ERLC, "ERLC"},
    {AudioAacProfile::ER_SCALABLE, "ER_SCALABLE"},
    {AudioAacProfile::LD, "LD"},
    {AudioAacProfile::HE_PS, "HE_PS"},
    {AudioAacProfile::ELD, "ELD"},
    {AudioAacProfile::XHE, "XHE"},
};

const std::map<AudioAacStreamFormat, const char*> g_auAacStreamFormatNameStrMap = {
    {AudioAacStreamFormat::MP2ADTS, "MP2ADTS"},
    {AudioAacStreamFormat::MP4ADTS, "MP4ADTS"},
    {AudioAacStreamFormat::MP4LOAS, "MP4LOAS"},
    {AudioAacStreamFormat::MP4LATM, "MP4LATM"},
    {AudioAacStreamFormat::ADIF, "ADIF"},
    {AudioAacStreamFormat::MP4FF, "MP4FF"},
    {AudioAacStreamFormat::RAW, "RAW"},
};

inline bool HasTagInfo(Tag tag)
{
    return g_tagInfoMap.count(tag) != 0;
}

inline const char* GetTagStrName(Tag tag)
{
    if (!HasTagInfo(tag)) {
        return "null";
    }
    return std::get<0>(g_tagInfoMap.at(tag));
}

inline const char* GetTagTypeStrName(Tag tag)
{
    if (!HasTagInfo(tag)) {
        return "null";
    }
    return std::get<2>(g_tagInfoMap.at(tag)); // secondary parameter 2
}

inline const Plugin::ValueType* GetTagDefValue(Tag tag)
{
    if (!HasTagInfo(tag)) {
        return nullptr;
    }
    return &std::get<1>(g_tagInfoMap.at(tag));
}

inline bool HasAudSampleFmtInfo(AudioSampleFormat fmt)
{
    return g_auSampleFmtStrMap.count(fmt) != 0;
}

inline const char* GetAudSampleFmtNameStr(AudioSampleFormat fmt)
{
    if (!HasAudSampleFmtInfo(fmt)) {
        return "null";
    }
    return g_auSampleFmtStrMap.at(fmt);
}

inline bool HasAudChanLyInfo(AudioChannelLayout layout)
{
    return g_auChannelLayoutStrMap.count(layout) != 0;
}

inline const char* GetAudChanLyNameStr(AudioChannelLayout layout)
{
    if (!HasAudChanLyInfo(layout)) {
        return "null";
    }
    return g_auChannelLayoutStrMap.at(layout);
}

inline bool HasVideoPixelFormatNameStr(VideoPixelFormat pixelFormat)
{
    return g_videoPixelFormatStrMap.count(pixelFormat) != 0;
}

inline const char* GetVideoPixelFormatNameStr(VideoPixelFormat pixelFormat)
{
    if (!HasVideoPixelFormatNameStr(pixelFormat)) {
        return "null";
    }
    return g_videoPixelFormatStrMap.at(pixelFormat);
}

inline bool HasAuAacProfileNameStr(AudioAacProfile aacProfile)
{
    return g_auAacProfileNameStrMap.count(aacProfile) != 0;
}

inline const char* GetAuAacProfileNameStr(AudioAacProfile aacProfile)
{
    if (!HasAuAacProfileNameStr(aacProfile)) {
        return "null";
    }
    return g_auAacProfileNameStrMap.at(aacProfile);
}

inline bool HasAacStreamFormatNameStr(AudioAacStreamFormat aacStreamFormat)
{
    return g_auAacStreamFormatNameStrMap.count(aacStreamFormat) != 0;
}

inline const char* GetAuAacStreamFormatNameStr(AudioAacStreamFormat aacStreamFormat)
{
    if (!HasAacStreamFormatNameStr(aacStreamFormat)) {
        return "null";
    }
    return g_auAacStreamFormatNameStrMap.at(aacStreamFormat);
}

inline const char* Tag2String(const Tag tag)
{
    auto mapIte = g_tagInfoMap.find(tag);
    if (mapIte == g_tagInfoMap.end()) {
        return "NULL";
    }
    return std::get<0>(mapIte->second);
}
} // Plugin
} // Media
} // OHOS
#endif // HISTREAMER_PIPELINE_PLUGIN_CAP_DESC_H
