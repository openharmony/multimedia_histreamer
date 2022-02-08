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

#include "plugin_settings.h"

#include <vector>
#include "plugin/common/plugin_audio_tags.h"

#define DECLARE_PARAMETER_ITEM(tag, type, op) \
{tag, {#tag, CheckParameterType<type>, op}}

namespace OHOS {
namespace Media {
namespace Pipeline {
template <typename T>
static bool CheckParameterType(const Plugin::ValueType& value)
{
    return value.SameTypeWith(typeid(T));
}

const PluginParaAllowedMap& PluginParameterTable::FindAllowedParameterMap(FilterType category)
{
    static const PluginParaAllowedMap emptyMap;
    auto ite = table_.find(category);
    if (ite == table_.end()) {
        return emptyMap;
    }
    return ite->second;
}

using namespace OHOS::Media::Plugin;
const std::map<FilterType, PluginParaAllowedMap> PluginParameterTable::table_ = {
    {FilterType::AUDIO_DECODER, {
        DECLARE_PARAMETER_ITEM(Tag::AUDIO_CHANNELS, uint32_t, PARAM_SET),
        DECLARE_PARAMETER_ITEM(Tag::AUDIO_SAMPLE_RATE, uint32_t, PARAM_SET),
        DECLARE_PARAMETER_ITEM(Tag::MEDIA_BITRATE, int64_t, PARAM_SET),
        DECLARE_PARAMETER_ITEM(Tag::AUDIO_SAMPLE_FORMAT, Plugin::AudioSampleFormat, PARAM_SET),
        DECLARE_PARAMETER_ITEM(Tag::AUDIO_SAMPLE_PER_FRAME, uint32_t, PARAM_SET),
        DECLARE_PARAMETER_ITEM(Tag::MEDIA_CODEC_CONFIG, std::vector<uint8_t>, PARAM_SET),
    }},
    {FilterType::AUDIO_SINK, {
        DECLARE_PARAMETER_ITEM(Tag::AUDIO_CHANNELS, uint32_t, PARAM_SET),
        DECLARE_PARAMETER_ITEM(Tag::AUDIO_SAMPLE_RATE, uint32_t, PARAM_SET),
        DECLARE_PARAMETER_ITEM(Tag::AUDIO_SAMPLE_FORMAT, Plugin::AudioSampleFormat, PARAM_SET),
        DECLARE_PARAMETER_ITEM(Tag::AUDIO_CHANNEL_LAYOUT, Plugin::AudioChannelLayout, PARAM_SET),
        DECLARE_PARAMETER_ITEM(Tag::AUDIO_SAMPLE_PER_FRAME, uint32_t, PARAM_SET),
    }},
    {FilterType::MUXER, {
        DECLARE_PARAMETER_ITEM(Tag::MIME, std::string, PARAM_SET),
        DECLARE_PARAMETER_ITEM(Tag::AUDIO_CHANNELS, uint32_t, PARAM_SET),
        DECLARE_PARAMETER_ITEM(Tag::AUDIO_SAMPLE_RATE, uint32_t, PARAM_SET),
        DECLARE_PARAMETER_ITEM(Tag::MEDIA_BITRATE, int64_t, PARAM_SET),
        DECLARE_PARAMETER_ITEM(Tag::AUDIO_SAMPLE_FORMAT, Plugin::AudioSampleFormat, PARAM_SET),
        DECLARE_PARAMETER_ITEM(Tag::AUDIO_SAMPLE_PER_FRAME, uint32_t, PARAM_SET),
        DECLARE_PARAMETER_ITEM(Tag::MEDIA_CODEC_CONFIG, std::vector<uint8_t>, PARAM_SET),
    }},
    {FilterType::AUDIO_ENCODER, {
        DECLARE_PARAMETER_ITEM(Tag::AUDIO_CHANNELS, uint32_t, PARAM_SET),
        DECLARE_PARAMETER_ITEM(Tag::AUDIO_SAMPLE_RATE, uint32_t, PARAM_SET),
        DECLARE_PARAMETER_ITEM(Tag::MEDIA_BITRATE, int64_t, PARAM_SET),
        DECLARE_PARAMETER_ITEM(Tag::AUDIO_SAMPLE_FORMAT, Plugin::AudioSampleFormat, PARAM_SET),
        DECLARE_PARAMETER_ITEM(Tag::AUDIO_SAMPLE_PER_FRAME, uint32_t, PARAM_SET | PARAM_GET),
        DECLARE_PARAMETER_ITEM(Tag::AUDIO_CHANNEL_LAYOUT, Plugin::AudioChannelLayout, PARAM_SET),
        DECLARE_PARAMETER_ITEM(Tag::MEDIA_CODEC_CONFIG, std::vector<uint8_t>,
                               PARAM_SET | PARAM_GET),
        DECLARE_PARAMETER_ITEM(Tag::AUDIO_AAC_PROFILE, uint32_t, PARAM_SET | PARAM_SET),
        DECLARE_PARAMETER_ITEM(Tag::AUDIO_AAC_LEVEL, uint32_t, PARAM_SET | PARAM_SET),
    }},
};
}
}
}