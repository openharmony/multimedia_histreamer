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
#include "utils/utils.h"

namespace OHOS {
namespace Media {
namespace Plugin {
// Internal definitions
namespace {
// Histreamer channel layout to ffmpeg channel layout
std::map<AudioChannelLayout, uint64_t> g_toFFMPEGChannelLayout = {
    {AudioChannelLayout::MONO, AV_CH_FRONT_LEFT},
    {AudioChannelLayout::STEREO, AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT},
    {AudioChannelLayout::CH_2POINT1, AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT | AV_CH_LOW_FREQUENCY},
    {AudioChannelLayout::CH_2_1, AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT | AV_CH_BACK_CENTER},
    {AudioChannelLayout::SURROUND, AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT | AV_CH_FRONT_CENTER},
    {AudioChannelLayout::CH_3POINT1, AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT | AV_CH_FRONT_CENTER | AV_CH_LOW_FREQUENCY},
    {AudioChannelLayout::CH_4POINT0, AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT | AV_CH_FRONT_CENTER | AV_CH_BACK_CENTER},
    {AudioChannelLayout::CH_4POINT1,
     AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT | AV_CH_FRONT_CENTER | AV_CH_BACK_CENTER | AV_CH_LOW_FREQUENCY},
    {AudioChannelLayout::CH_2_2, AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT | AV_CH_SIDE_LEFT | AV_CH_SIDE_RIGHT},
    {AudioChannelLayout::QUAD, AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT | AV_CH_BACK_LEFT | AV_CH_BACK_RIGHT},
    {AudioChannelLayout::CH_5POINT0,
     AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT | AV_CH_FRONT_CENTER | AV_CH_SIDE_LEFT | AV_CH_SIDE_RIGHT},
    {AudioChannelLayout::CH_5POINT1, AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT | AV_CH_FRONT_CENTER | AV_CH_SIDE_LEFT |
                                         AV_CH_SIDE_RIGHT | AV_CH_LOW_FREQUENCY},
    {AudioChannelLayout::CH_5POINT0_BACK,
     AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT | AV_CH_FRONT_CENTER | AV_CH_BACK_LEFT | AV_CH_BACK_RIGHT},
    {AudioChannelLayout::CH_5POINT1_BACK, AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT | AV_CH_FRONT_CENTER | AV_CH_BACK_LEFT |
                                              AV_CH_BACK_RIGHT | AV_CH_LOW_FREQUENCY},
    {AudioChannelLayout::CH_6POINT0, AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT | AV_CH_FRONT_CENTER | AV_CH_BACK_LEFT |
                                         AV_CH_BACK_RIGHT | AV_CH_BACK_CENTER},
    {AudioChannelLayout::CH_6POINT0_FRONT, AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT | AV_CH_SIDE_LEFT | AV_CH_SIDE_RIGHT |
                                               AV_CH_FRONT_LEFT_OF_CENTER | AV_CH_FRONT_RIGHT_OF_CENTER},
    {AudioChannelLayout::HEXAGONAL, AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT | AV_CH_FRONT_CENTER | AV_CH_BACK_LEFT |
                                        AV_CH_BACK_RIGHT | AV_CH_BACK_CENTER},
    {AudioChannelLayout::CH_6POINT1, AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT | AV_CH_FRONT_CENTER | AV_CH_SIDE_LEFT |
                                         AV_CH_SIDE_RIGHT | AV_CH_LOW_FREQUENCY | AV_CH_BACK_CENTER},
    {AudioChannelLayout::CH_6POINT1_BACK, AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT | AV_CH_FRONT_CENTER | AV_CH_BACK_LEFT |
                                              AV_CH_BACK_RIGHT | AV_CH_LOW_FREQUENCY | AV_CH_BACK_CENTER},
    {AudioChannelLayout::CH_6POINT1_FRONT, AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT | AV_CH_SIDE_LEFT | AV_CH_SIDE_RIGHT |
                                               AV_CH_FRONT_LEFT_OF_CENTER | AV_CH_FRONT_RIGHT_OF_CENTER |
                                               AV_CH_LOW_FREQUENCY},
    {AudioChannelLayout::CH_7POINT0, AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT | AV_CH_FRONT_CENTER | AV_CH_SIDE_LEFT |
                                         AV_CH_SIDE_RIGHT | AV_CH_BACK_LEFT | AV_CH_BACK_RIGHT},
    {AudioChannelLayout::CH_7POINT0_FRONT, AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT | AV_CH_FRONT_CENTER | AV_CH_SIDE_LEFT |
                                               AV_CH_SIDE_RIGHT | AV_CH_FRONT_LEFT_OF_CENTER |
                                               AV_CH_FRONT_RIGHT_OF_CENTER},
    {AudioChannelLayout::CH_7POINT1, AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT | AV_CH_FRONT_CENTER | AV_CH_SIDE_LEFT |
                                         AV_CH_SIDE_RIGHT | AV_CH_LOW_FREQUENCY | AV_CH_BACK_LEFT | AV_CH_BACK_RIGHT},
    {AudioChannelLayout::CH_7POINT1_WIDE, AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT | AV_CH_FRONT_CENTER | AV_CH_SIDE_LEFT |
                                              AV_CH_SIDE_RIGHT | AV_CH_LOW_FREQUENCY | AV_CH_FRONT_LEFT_OF_CENTER |
                                              AV_CH_FRONT_RIGHT_OF_CENTER},
    {AudioChannelLayout::CH_7POINT1_WIDE_BACK, AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT | AV_CH_FRONT_CENTER |
                                                   AV_CH_BACK_LEFT | AV_CH_BACK_RIGHT | AV_CH_LOW_FREQUENCY |
                                                   AV_CH_FRONT_LEFT_OF_CENTER | AV_CH_FRONT_RIGHT_OF_CENTER},
    {AudioChannelLayout::OCTAGONAL, AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT | AV_CH_FRONT_CENTER | AV_CH_SIDE_LEFT |
                                        AV_CH_SIDE_RIGHT | AV_CH_BACK_LEFT | AV_CH_BACK_CENTER | AV_CH_BACK_RIGHT},
    {AudioChannelLayout::HEXADECAGONAL, AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT | AV_CH_FRONT_CENTER | AV_CH_SIDE_LEFT |
                                            AV_CH_SIDE_RIGHT | AV_CH_BACK_LEFT | AV_CH_BACK_CENTER | AV_CH_BACK_RIGHT |
                                            AV_CH_WIDE_LEFT | AV_CH_WIDE_RIGHT | AV_CH_TOP_BACK_LEFT |
                                            AV_CH_TOP_BACK_CENTER | AV_CH_TOP_BACK_RIGHT | AV_CH_TOP_FRONT_LEFT |
                                            AV_CH_TOP_FRONT_CENTER | AV_CH_TOP_FRONT_RIGHT},
    {AudioChannelLayout::STEREO_DOWNMIX, AV_CH_STEREO_LEFT | AV_CH_STEREO_RIGHT},
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
        constexpr int timeScale = 1000000;
        AVRational bq = {1, timeScale};
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
        constexpr int timeScale = 1000000;
        AVRational bq = {1, timeScale};
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
            MEDIA_LOG_W("unsupported audio channel layout: %" PRIu64, mask);
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
        MEDIA_LOG_E("ConvertChannelLayoutToFFmpeg, unexpected audio channel layout: %" PRIu64,
                    OHOS::Media::to_underlying(channelLayout));
        return 0;
    }
    return it->second;
}
} // namespace Plugin
} // namespace Media
} // namespace OHOS
