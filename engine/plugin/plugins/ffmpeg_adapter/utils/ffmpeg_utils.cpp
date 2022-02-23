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

#include "ffmpeg_utils.h"

#include <algorithm>

#include "foundation/log.h"
#include "libavutil/channel_layout.h"
#include "plugin/common/plugin_audio_tags.h"
#include "plugin/common/plugin_time.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace Ffmpeg {
// Internal definitions
namespace {
// Histreamer channel layout to ffmpeg channel layout
std::map<AudioChannelLayout, uint64_t> g_toFFMPEGChannelLayout = {
    {AudioChannelLayout::MONO, AV_CH_LAYOUT_MONO},
    {AudioChannelLayout::STEREO, AV_CH_LAYOUT_STEREO},
    {AudioChannelLayout::CH_2POINT1, AV_CH_LAYOUT_2POINT1},
    {AudioChannelLayout::CH_2_1, AV_CH_LAYOUT_2_1},
    {AudioChannelLayout::SURROUND, AV_CH_LAYOUT_SURROUND},
    {AudioChannelLayout::CH_3POINT1, AV_CH_LAYOUT_3POINT1},
    {AudioChannelLayout::CH_4POINT0, AV_CH_LAYOUT_4POINT0},
    {AudioChannelLayout::CH_4POINT1, AV_CH_LAYOUT_4POINT1},
    {AudioChannelLayout::CH_2_2, AV_CH_LAYOUT_2_2},
    {AudioChannelLayout::QUAD, AV_CH_LAYOUT_QUAD},
    {AudioChannelLayout::CH_5POINT0, AV_CH_LAYOUT_5POINT0},
    {AudioChannelLayout::CH_5POINT1, AV_CH_LAYOUT_5POINT1},
    {AudioChannelLayout::CH_5POINT0_BACK, AV_CH_LAYOUT_5POINT0_BACK},
    {AudioChannelLayout::CH_5POINT1_BACK, AV_CH_LAYOUT_5POINT1_BACK},
    {AudioChannelLayout::CH_6POINT0, AV_CH_LAYOUT_6POINT0},
    {AudioChannelLayout::CH_6POINT0_FRONT, AV_CH_LAYOUT_6POINT0_FRONT},
    {AudioChannelLayout::HEXAGONAL, AV_CH_LAYOUT_HEXAGONAL},
    {AudioChannelLayout::CH_6POINT1, AV_CH_LAYOUT_6POINT1},
    {AudioChannelLayout::CH_6POINT1_BACK, AV_CH_LAYOUT_6POINT1_BACK},
    {AudioChannelLayout::CH_6POINT1_FRONT, AV_CH_LAYOUT_6POINT1_FRONT},
    {AudioChannelLayout::CH_7POINT0, AV_CH_LAYOUT_7POINT0},
    {AudioChannelLayout::CH_7POINT0_FRONT, AV_CH_LAYOUT_7POINT0_FRONT},
    {AudioChannelLayout::CH_7POINT1, AV_CH_LAYOUT_7POINT1},
    {AudioChannelLayout::CH_7POINT1_WIDE, AV_CH_LAYOUT_7POINT1_WIDE},
    {AudioChannelLayout::CH_7POINT1_WIDE_BACK, AV_CH_LAYOUT_7POINT1_WIDE_BACK},
    {AudioChannelLayout::OCTAGONAL, AV_CH_LAYOUT_OCTAGONAL},
    {AudioChannelLayout::HEXADECAGONAL, AV_CH_LAYOUT_HEXADECAGONAL},
    {AudioChannelLayout::STEREO_DOWNMIX, AV_CH_LAYOUT_STEREO_DOWNMIX},
};

// ffmpeg channel layout to histreamer channel layout
std::map<uint64_t, AudioChannelMasks> g_fromFFMPEGChannelLayout = {
    {AV_CH_FRONT_LEFT, AudioChannelMasks::FRONT_LEFT},
    {AV_CH_FRONT_RIGHT, AudioChannelMasks::FRONT_RIGHT},
    {AV_CH_FRONT_CENTER, AudioChannelMasks::FRONT_CENTER},
    {AV_CH_LOW_FREQUENCY, AudioChannelMasks::LOW_FREQUENCY},
    {AV_CH_BACK_LEFT, AudioChannelMasks::BACK_LEFT},
    {AV_CH_BACK_RIGHT, AudioChannelMasks::BACK_RIGHT},
    {AV_CH_FRONT_LEFT_OF_CENTER, AudioChannelMasks::FRONT_LEFT_OF_CENTER},
    {AV_CH_FRONT_RIGHT_OF_CENTER, AudioChannelMasks::FRONT_RIGHT_OF_CENTER},
    {AV_CH_BACK_CENTER, AudioChannelMasks::BACK_CENTER},
    {AV_CH_SIDE_LEFT, AudioChannelMasks::SIDE_LEFT},
    {AV_CH_SIDE_RIGHT, AudioChannelMasks::SIDE_RIGHT},
    {AV_CH_TOP_CENTER, AudioChannelMasks::TOP_CENTER},
    {AV_CH_TOP_FRONT_LEFT, AudioChannelMasks::TOP_FRONT_LEFT},
    {AV_CH_TOP_FRONT_CENTER, AudioChannelMasks::TOP_FRONT_CENTER},
    {AV_CH_TOP_FRONT_RIGHT, AudioChannelMasks::TOP_FRONT_RIGHT},
    {AV_CH_TOP_BACK_LEFT, AudioChannelMasks::TOP_BACK_LEFT},
    {AV_CH_TOP_BACK_CENTER, AudioChannelMasks::TOP_BACK_CENTER},
    {AV_CH_TOP_BACK_RIGHT, AudioChannelMasks::TOP_BACK_RIGHT},
    {AV_CH_STEREO_LEFT, AudioChannelMasks::STEREO_LEFT},
    {AV_CH_STEREO_RIGHT, AudioChannelMasks::STEREO_RIGHT},
};

const std::map<std::string, Tag> g_tagMap = {
    {"title", Tag::MEDIA_TITLE},
    {"artist", Tag::MEDIA_ARTIST},
    {"lyricist", Tag::MEDIA_LYRICIST},
    {"album", Tag::MEDIA_ALBUM},
    {"album-artist", Tag::MEDIA_ALBUM_ARTIST},
    {"date", Tag::MEDIA_DATE},
    {"comment", Tag::MEDIA_COMMENT},
    {"genre", Tag::MEDIA_GENRE},
    {"copyright", Tag::MEDIA_COPYRIGHT},
    {"language", Tag::MEDIA_LANGUAGE},
    {"description", Tag::MEDIA_DESCRIPTION},
    {"lyrics", Tag::MEDIA_LYRICS},
};
const std::vector<std::pair<AudioAacProfile, int32_t>> g_AacProfileMap = {
    {AudioAacProfile::MAIN, FF_PROFILE_AAC_MAIN},
    {AudioAacProfile::LC, FF_PROFILE_AAC_LOW},
    {AudioAacProfile::SSR, FF_PROFILE_AAC_SSR},
    {AudioAacProfile::LTP, FF_PROFILE_AAC_LTP},
    {AudioAacProfile::HE, FF_PROFILE_AAC_HE},
    {AudioAacProfile::HE_PS, FF_PROFILE_AAC_HE_V2},
    {AudioAacProfile::LD, FF_PROFILE_AAC_LD},
    {AudioAacProfile::ELD, FF_PROFILE_AAC_ELD},
};

// Histreamer pixel format to ffmpeg pixel format
std::map<VideoPixelFormat, AVPixelFormat> g_pixelFormatMap = {
    {VideoPixelFormat::YUV420P, AV_PIX_FMT_YUV420P},
    {VideoPixelFormat::YUV420P, AV_PIX_FMT_YUYV422},
    {VideoPixelFormat::RGB24, AV_PIX_FMT_RGB24},
    {VideoPixelFormat::BGR24, AV_PIX_FMT_BGR24},
    {VideoPixelFormat::YUV422P, AV_PIX_FMT_YUV422P},
    {VideoPixelFormat::YUV444P, AV_PIX_FMT_YUV444P},
    {VideoPixelFormat::YUV410P, AV_PIX_FMT_YUV410P},
    {VideoPixelFormat::YUV411P, AV_PIX_FMT_YUV411P},
    {VideoPixelFormat::GRAY8, AV_PIX_FMT_GRAY8},
    {VideoPixelFormat::MONOWHITE, AV_PIX_FMT_MONOWHITE},
    {VideoPixelFormat::MONOBLACK, AV_PIX_FMT_MONOBLACK},
    {VideoPixelFormat::PAL8, AV_PIX_FMT_PAL8},
    {VideoPixelFormat::YUVJ420P, AV_PIX_FMT_YUVJ420P},
    {VideoPixelFormat::YUVJ422P, AV_PIX_FMT_YUVJ422P},
    {VideoPixelFormat::YUVJ444P, AV_PIX_FMT_YUVJ444P},
    {VideoPixelFormat::NV12, AV_PIX_FMT_NV12},
    {VideoPixelFormat::NV21, AV_PIX_FMT_NV21},
};

std::map<VideoH264Profile, int32_t> g_H264ProfileMap = {
    {VideoH264Profile::BASELINE, FF_PROFILE_H264_BASELINE},
    {VideoH264Profile::MAIN, FF_PROFILE_H264_MAIN},
    {VideoH264Profile::EXTENDED, FF_PROFILE_H264_EXTENDED},
    {VideoH264Profile::HIGH, FF_PROFILE_H264_HIGH},
    {VideoH264Profile::HIGH10, FF_PROFILE_H264_HIGH_10},
    {VideoH264Profile::HIGH422, FF_PROFILE_H264_HIGH_422},
    {VideoH264Profile::HIGH444, FF_PROFILE_H264_HIGH_444}
};
} // namespace

std::string AVStrError(int errnum)
{
    char errbuf[AV_ERROR_MAX_STRING_SIZE] = {0};
    av_strerror(errnum, errbuf, AV_ERROR_MAX_STRING_SIZE);
    return std::string(errbuf);
}

uint64_t ConvertTimeFromFFmpeg(int64_t pts, AVRational base)
{
    uint64_t out;
    if (pts == AV_NOPTS_VALUE) {
        out = static_cast<uint64_t>(-1);
    } else {
        AVRational bq = {1, HST_SECOND};
        out = av_rescale_q(pts, base, bq);
    }
    return out;
}

int64_t ConvertTimeToFFmpeg(int64_t timestampUs, AVRational base)
{
    int64_t result;
    if (base.num == 0) {
        result = AV_NOPTS_VALUE;
    } else {
        AVRational bq = {1, HST_SECOND};
        result = av_rescale_q(timestampUs, bq, base);
    }
    return result;
}

int FillAVPicture(AVFrame* picture, uint8_t* ptr, enum AVPixelFormat pixFmt, int width, int height)
{
    (void)picture;
    (void)ptr;
    (void)pixFmt;
    (void)width;
    (void)height;
    return 0;
}

int GetAVPictureSize(int pixFmt, int width, int height)
{
    AVFrame dummy;
    return FillAVPicture(&dummy, nullptr, static_cast<AVPixelFormat>(pixFmt), width, height);
}

std::string RemoveDelimiter(const char* str, char delimiter)
{
    std::string tmp(str);
    RemoveDelimiter(delimiter, tmp);
    return tmp;
}

void RemoveDelimiter(char delimiter, std::string& str)
{
    for (auto it = std::find(str.begin(), str.end(), delimiter); it != str.end();) {
        it = str.erase(it);
        if (*it != delimiter) {
            it = std::find(it, str.end(), delimiter);
        }
    }
}

void ReplaceDelimiter(const std::string& delmiters, char newDelimiter, std::string& str)
{
    for (auto it = str.begin(); it != str.end(); ++it) {
        if (delmiters.find(newDelimiter) != std::string::npos) {
            *it = newDelimiter;
        }
    }
}

std::vector<std::string> SplitString(const char* str, char delimiter)
{
    std::vector<std::string> rtv;
    if (str) {
        SplitString(std::string(str), delimiter).swap(rtv);
    }
    return rtv;
}

std::vector<std::string> SplitString(const std::string& str, char delimiter)
{
    if (str.empty()) {
        return {};
    }
    std::vector<std::string> rtv;
    std::string::size_type startPos = 0;
    std::string::size_type endPos = str.find_first_of(delimiter, startPos);
    while (startPos != endPos) {
        rtv.emplace_back(str.substr(startPos, endPos - startPos));
        if (endPos == std::string::npos) {
            break;
        }
        startPos = endPos + 1;
        endPos = str.find_first_of(delimiter, startPos);
    }
    return rtv;
}

AudioSampleFormat Trans2Format(AVSampleFormat sampleFormat)
{
    switch (sampleFormat) {
        case AV_SAMPLE_FMT_U8:
            return AudioSampleFormat::U8;
        case AV_SAMPLE_FMT_U8P:
            return AudioSampleFormat::U8P;
        case AV_SAMPLE_FMT_S16:
            return AudioSampleFormat::S16;
        case AV_SAMPLE_FMT_S16P:
            return AudioSampleFormat::S16P;
        case AV_SAMPLE_FMT_S32:
            return AudioSampleFormat::S32;
        case AV_SAMPLE_FMT_S32P:
            return AudioSampleFormat::S32P;
        case AV_SAMPLE_FMT_FLT:
            return AudioSampleFormat::F32;
        case AV_SAMPLE_FMT_FLTP:
            return AudioSampleFormat::F32P;
        case AV_SAMPLE_FMT_DBL:
            return AudioSampleFormat::F64;
        case AV_SAMPLE_FMT_DBLP:
            return AudioSampleFormat::F64P;
        default:
            return AudioSampleFormat::S16;
    }
}

AVSampleFormat Trans2FFmepgFormat(AudioSampleFormat sampleFormat)
{
    switch (sampleFormat) {
        case AudioSampleFormat::U8:
            return AVSampleFormat::AV_SAMPLE_FMT_U8;
        case AudioSampleFormat::U8P:
            return AVSampleFormat::AV_SAMPLE_FMT_U8P;
        case AudioSampleFormat::S16:
            return AVSampleFormat::AV_SAMPLE_FMT_S16;
        case AudioSampleFormat::S16P:
            return AVSampleFormat::AV_SAMPLE_FMT_S16P;
        case AudioSampleFormat::S32:
            return AVSampleFormat::AV_SAMPLE_FMT_S32;
        case AudioSampleFormat::S32P:
            return AVSampleFormat::AV_SAMPLE_FMT_S32P;
        case AudioSampleFormat::F32:
            return AVSampleFormat::AV_SAMPLE_FMT_FLT;
        case AudioSampleFormat::F32P:
            return AVSampleFormat::AV_SAMPLE_FMT_FLTP;
        case AudioSampleFormat::F64:
            return AVSampleFormat::AV_SAMPLE_FMT_DBL;
        case AudioSampleFormat::F64P:
            return AVSampleFormat::AV_SAMPLE_FMT_DBLP;
        default:
            return AV_SAMPLE_FMT_NONE;
    }
}

AudioChannelLayout ConvertChannelLayoutFromFFmpeg(int channels, uint64_t ffChannelLayout)
{
    uint64_t channelLayout = 0;
    uint64_t mask = 1;
    for (uint8_t bitPos = 0, channelNum = 0; (bitPos < 64) && (channelNum < channels); ++bitPos) { // 64
        mask = 1ULL << bitPos;
        if (!(mask & ffChannelLayout)) {
            continue;
        }
        channelNum++;
        auto it = g_fromFFMPEGChannelLayout.find(mask);
        if (it != g_fromFFMPEGChannelLayout.end()) {
            channelLayout |= static_cast<uint64_t>(it->second);
        } else {
            MEDIA_LOG_W("unsupported audio channel layout: " PUBLIC_LOG PRIu64, mask);
        }
    }
    auto ret = static_cast<AudioChannelLayout>(channelLayout);
    if (ffChannelLayout == 0) {
        if (channels == 1) {
            ret = AudioChannelLayout::MONO;
        }
        if (channels == 2) { // 2
            ret = AudioChannelLayout::STEREO;
        }
    }
    return ret;
}

uint64_t ConvertChannelLayoutToFFmpeg(AudioChannelLayout channelLayout)
{
    auto it = g_toFFMPEGChannelLayout.find(channelLayout);
    if (it == g_toFFMPEGChannelLayout.end()) {
        MEDIA_LOG_E("ConvertChannelLayoutToFFmpeg: unknown audio channel layout: " PUBLIC_LOG PRIu64, channelLayout);
        return 0;
    }
    return it->second;
}

bool FindAvMetaNameByTag(Tag tag, std::string& metaName)
{
    for (const auto& pair : g_tagMap) {
        if (pair.second == tag) {
            metaName = pair.first;
            return true;
        }
    }
    return false;
}

bool FindTagByAvMetaName(const std::string& metaName, Tag& tag)
{
    auto ite = g_tagMap.find(metaName);
    if (ite == std::end(g_tagMap)) {
        return false;
    }
    tag = ite->second;
    return true;
}

AudioAacProfile ConvAacProfileFromFfmpeg (int32_t ffmpegProfile)
{
    auto ite = std::find_if(g_AacProfileMap.begin(), g_AacProfileMap.end(),
        [&] (const std::pair<AudioAacProfile, int32_t>& tmp) -> bool {
        return tmp.second == ffmpegProfile;
    });
    return ite == g_AacProfileMap.end() ? AudioAacProfile::NONE : ite->first;
}

int32_t ConvAacProfileToFfmpeg (AudioAacProfile profile)
{
    auto ite = std::find_if(g_AacProfileMap.begin(), g_AacProfileMap.end(),
        [&] (const std::pair<AudioAacProfile, int32_t>& tmp) -> bool {
        return tmp.first == profile;
    });
    return ite == g_AacProfileMap.end() ? FF_PROFILE_UNKNOWN : ite->second;
}

VideoPixelFormat ConvertPixelFormatFromFFmpeg(int32_t ffmpegPixelFormat)
{
    auto iter = std::find_if(g_pixelFormatMap.begin(), g_pixelFormatMap.end(),
        [&] (const std::pair<VideoPixelFormat, AVPixelFormat>& tmp) -> bool {
        return tmp.second == ffmpegPixelFormat;
    });
    return iter == g_pixelFormatMap.end() ? VideoPixelFormat::UNKNOWN : iter->first;
}

AVPixelFormat ConvertPixelFormatToFFmpeg(VideoPixelFormat pixelFormat)
{
    auto iter = std::find_if(g_pixelFormatMap.begin(), g_pixelFormatMap.end(),
        [&] (const std::pair<VideoPixelFormat, AVPixelFormat>& tmp) -> bool {
        return tmp.first == pixelFormat;
    });
    return iter == g_pixelFormatMap.end() ? AV_PIX_FMT_NONE : iter->second;
}

bool IsYuvFormat(AVPixelFormat format)
{
    return (format == AV_PIX_FMT_YUV420P || format == AV_PIX_FMT_NV12 || format == AV_PIX_FMT_NV21 ||
            format == AV_PIX_FMT_YUYV422 || format == AV_PIX_FMT_YUV422P || format == AV_PIX_FMT_YUV444P ||
            format == AV_PIX_FMT_YUV410P || format == AV_PIX_FMT_YUV411P || format == AV_PIX_FMT_YUVJ420P ||
            format == AV_PIX_FMT_YUVJ422P || format == AV_PIX_FMT_YUVJ444P);
}

bool IsRgbFormat(AVPixelFormat format)
{
    return (format == AV_PIX_FMT_RGB24 || format == AV_PIX_FMT_BGR24);
}

VideoH264Profile ConvH264ProfileFromFfmpeg(int32_t ffmpegProfile)
{
    auto iter = std::find_if(g_H264ProfileMap.begin(), g_H264ProfileMap.end(),
                             [&] (const std::pair<VideoH264Profile, int32_t>& tmp) -> bool {
        return tmp.second == ffmpegProfile;
    });
    return (iter == g_H264ProfileMap.end()) ? VideoH264Profile::UNKNOWN : iter->first;
}

int32_t ConvH264ProfileToFfmpeg(VideoH264Profile profile)
{
    auto iter = std::find_if(g_H264ProfileMap.begin(), g_H264ProfileMap.end(),
                             [&] (const std::pair<VideoH264Profile, int32_t>& tmp) -> bool {
        return tmp.first == profile;
    });
    return (iter == g_H264ProfileMap.end()) ? FF_PROFILE_UNKNOWN : iter->second;
}
} // namespace Ffmpeg
} // namespace Plugin
} // namespace Media
} // namespace OHOS
