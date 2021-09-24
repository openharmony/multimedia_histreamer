/*
 * Copyright (c) 2021-2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "HdiAuUtils"

#include "hdi_au_utils.h"

#include <utility>

namespace OHOS {
namespace Media {
namespace HosLitePlugin {
namespace {
struct PluginHdiAudioFormatTable {
    AudioFormat hFormat;
    bool isSigned;
    bool isInterleaved;
    bool isBigEndian;
    OHOS::Media::Plugin::AudioSampleFormat pFormat;
};

PluginHdiAudioFormatTable g_phft[] = {
    {AUDIO_FORMAT_PCM_8_BIT, false, false, false, OHOS::Media::Plugin::AudioSampleFormat::U8},
    {AUDIO_FORMAT_PCM_8_BIT, false, true, false, OHOS::Media::Plugin::AudioSampleFormat::U8P},
    {AUDIO_FORMAT_PCM_8_BIT, true, false, false, OHOS::Media::Plugin::AudioSampleFormat::S8},
    {AUDIO_FORMAT_PCM_8_BIT, true, true, false, OHOS::Media::Plugin::AudioSampleFormat::S8P},
    {AUDIO_FORMAT_PCM_16_BIT, false, false, false, OHOS::Media::Plugin::AudioSampleFormat::U16},
    {AUDIO_FORMAT_PCM_16_BIT, false, true, false, OHOS::Media::Plugin::AudioSampleFormat::U16P},
    {AUDIO_FORMAT_PCM_16_BIT, true, false, false, OHOS::Media::Plugin::AudioSampleFormat::S16},
    {AUDIO_FORMAT_PCM_16_BIT, true, true, false, OHOS::Media::Plugin::AudioSampleFormat::S16P},
    {AUDIO_FORMAT_PCM_32_BIT, false, false, false, OHOS::Media::Plugin::AudioSampleFormat::U32},
    {AUDIO_FORMAT_PCM_32_BIT, false, true, false, OHOS::Media::Plugin::AudioSampleFormat::U32P},
    {AUDIO_FORMAT_PCM_32_BIT, true, false, false, OHOS::Media::Plugin::AudioSampleFormat::S32},
    {AUDIO_FORMAT_PCM_32_BIT, true, true, false, OHOS::Media::Plugin::AudioSampleFormat::S32P},
};

std::pair<uint32_t, AudioSampleRatesMask> g_phst[] = {
    {0, AUDIO_SAMPLE_RATE_MASK_INVALID},
    {8000, AUDIO_SAMPLE_RATE_MASK_8000},
    {12000, AUDIO_SAMPLE_RATE_MASK_12000},
    {11025, AUDIO_SAMPLE_RATE_MASK_11025},
    {16000, AUDIO_SAMPLE_RATE_MASK_16000},
    {22050, AUDIO_SAMPLE_RATE_MASK_22050},
    {24000, AUDIO_SAMPLE_RATE_MASK_24000},
    {32000, AUDIO_SAMPLE_RATE_MASK_32000},
    {44100, AUDIO_SAMPLE_RATE_MASK_44100},
    {48000, AUDIO_SAMPLE_RATE_MASK_48000},
    {64000, AUDIO_SAMPLE_RATE_MASK_64000},
    {96000, AUDIO_SAMPLE_RATE_MASK_96000},
};

std::pair<OHOS::Media::Plugin::AudioChannelLayout, AudioChannelMask> g_phcmt[] = {
    {OHOS::Media::Plugin::AudioChannelLayout::MONO, AUDIO_CHANNEL_MONO},
    {OHOS::Media::Plugin::AudioChannelLayout::STEREO, AUDIO_CHANNEL_STEREO}
};
} // namespace

bool PluginAuFormat2HdiAttrs(OHOS::Media::Plugin::AudioSampleFormat pFormat, AudioSampleAttributes& attrs)
{
    for (const auto& item : g_phft) {
        if (item.pFormat == pFormat) {
            attrs.format = item.hFormat;
            attrs.isSignedData = item.isSigned;
            attrs.isBigEndian = item.isBigEndian;
            attrs.interleaved = item.isInterleaved;
            return true;
        }
    }
    return false;
}

bool HdiAttrs2PluginAuFormat(AudioSampleAttributes attrs, OHOS::Media::Plugin::AudioSampleFormat& pFormat)
{
    for (const auto& item : g_phft) {
        if (attrs.format == item.hFormat && attrs.isSignedData == item.isSigned &&
            attrs.interleaved == item.isInterleaved && attrs.isBigEndian == item.isBigEndian) {
            pFormat = item.pFormat;
            return true;
        }
    }
    return false;
}

bool PluginSampleRate2HdiRate(uint32_t pRate, AudioSampleRatesMask& mask)
{
    for (const auto& item : g_phst) {
        if (item.first == pRate) {
            mask = item.second;
            return true;
        }
    }
    mask = AUDIO_SAMPLE_RATE_MASK_INVALID;
    return false;
}
bool HdiRate2PluginSampleRate(AudioSampleRatesMask mask, uint32_t& pRate)
{
    for (const auto& item : g_phst) {
        if (item.second == mask) {
            pRate = item.first;
            return true;
        }
    }
    return false;
}

bool PluginChannelLayout2HdiMask(OHOS::Media::Plugin::AudioChannelLayout layout, AudioChannelMask& mask)
{
    for (const auto& item : g_phcmt) {
        if (item.first == layout) {
            mask = item.second;
            return true;
        }
    }
    return false;
}
bool HdiMask2PluginChannelLayout(AudioChannelMask mask, OHOS::Media::Plugin::AudioChannelLayout& layout)
{
    for (const auto& item : g_phcmt) {
        if (item.second == mask) {
            layout = item.first;
            return true;
        }
    }
    return false;
}
} // namespace HosLitePlugin
} // namespace Media
} // namespace OHOS
