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

#define DECLARE_PARAMETER_ITEM(tag, type) \
{tag, {#tag, CheckParameterType<type>}}

namespace OHOS {
namespace Media {
namespace Pipeline {
const PluginParaAllowedMap g_emptyMap;

template <typename T>
static bool CheckParameterType(const Plugin::ValueType& value)
{
    return value.Type() == typeid(T);
}

const PluginParaAllowedMap& PluginParameterTable::FindAllowedParameterMap(FilterType category)
{
    auto ite = table_.find(category);
    if (ite == table_.end()) {
        return g_emptyMap;
    }
    return ite->second;
}

const std::map<FilterType, PluginParaAllowedMap> PluginParameterTable::table_ = {
    {FilterType::AUDIO_DECODER, {
        DECLARE_PARAMETER_ITEM(Plugin::Tag::AUDIO_CHANNELS, uint32_t),
        DECLARE_PARAMETER_ITEM(Plugin::Tag::AUDIO_SAMPLE_RATE, uint32_t),
        DECLARE_PARAMETER_ITEM(Plugin::Tag::MEDIA_BITRATE, int64_t),
        DECLARE_PARAMETER_ITEM(Plugin::Tag::AUDIO_SAMPLE_FORMAT, Plugin::AudioSampleFormat),
        DECLARE_PARAMETER_ITEM(Plugin::Tag::AUDIO_SAMPLE_PER_FRAME, uint32_t),
        DECLARE_PARAMETER_ITEM(Plugin::Tag::MEDIA_CODEC_CONFIG, std::vector<uint8_t>),
    }},
    {FilterType::AUDIO_SINK,    {
        DECLARE_PARAMETER_ITEM(Plugin::Tag::AUDIO_CHANNELS, uint32_t),
        DECLARE_PARAMETER_ITEM(Plugin::Tag::AUDIO_SAMPLE_RATE, uint32_t),
        DECLARE_PARAMETER_ITEM(Plugin::Tag::AUDIO_SAMPLE_FORMAT, Plugin::AudioSampleFormat),
        DECLARE_PARAMETER_ITEM(Plugin::Tag::AUDIO_CHANNEL_LAYOUT, Plugin::AudioChannelLayout),
        DECLARE_PARAMETER_ITEM(Plugin::Tag::AUDIO_SAMPLE_PER_FRAME, uint32_t),
    }},
};
}
}
}