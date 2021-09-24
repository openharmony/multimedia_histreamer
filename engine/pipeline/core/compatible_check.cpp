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

#define LOG_TAG "Compatible_Check"

#include "compatible_check.h"

#include <algorithm>
#include <functional>
#include <map>

#include "foundation/log.h"
#include "plugin/common/plugin_audio_tags.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
static constexpr uint8_t ALLOW_FIXED = 1 << 0;
static constexpr uint8_t ALLOW_INTERVAL = 1 << 1;
static constexpr uint8_t ALLOW_DISCRETE = 1 << 2;

static inline bool IsFixedAllowed(uint8_t flags)
{
    return ALLOW_FIXED & flags;
}

static inline bool IsIntervalAllowed(uint8_t flags)
{
    return ALLOW_INTERVAL & flags;
}

static inline bool IsDiscreteAllowed(uint8_t flags)
{
    return ALLOW_DISCRETE & flags;
}

__attribute__((unused)) static bool StringCapabilityCheck(const std::pair<CapabilityID, Plugin::ValueType>& tagEntry,
                                                          const Meta& meta, uint8_t flags);
template <typename T>
bool NumericalCapabilityCheck(const std::pair<CapabilityID, Plugin::ValueType>& tagEntry, const Meta& meta,
                              uint8_t flags, std::function<int(T, T)> compareFunc);

template <typename T>
bool FixInvalDiscNumericalCheck(const std::pair<CapabilityID, Plugin::ValueType>& capability, const Meta& meta)
{
    return NumericalCapabilityCheck<T>(capability, meta, ALLOW_FIXED | ALLOW_INTERVAL | ALLOW_DISCRETE,
                                       [](T a, T b) { return a - b; });
}

template <typename T, typename U>
bool FixDiscNumericalCheck(const std::pair<CapabilityID, Plugin::ValueType>& capability, const Meta& meta)
{
    return NumericalCapabilityCheck<T>(capability, meta, ALLOW_FIXED | ALLOW_DISCRETE,
                                       [](T a, T b) { return static_cast<U>(a) - static_cast<U>(b); });
}

static std::map<CapabilityID, std::function<bool(const std::pair<CapabilityID, Plugin::ValueType>&, const Meta&)>>
    g_capabilityCheckMap = {
        {CapabilityID::AUDIO_CHANNELS, FixInvalDiscNumericalCheck<uint32_t>},
        {CapabilityID::AUDIO_SAMPLE_RATE, FixInvalDiscNumericalCheck<uint32_t>},
        {CapabilityID::AUDIO_MPEG_VERSION, FixInvalDiscNumericalCheck<uint32_t>},
        {CapabilityID::AUDIO_MPEG_LAYER, FixInvalDiscNumericalCheck<uint32_t>},
        {CapabilityID::AUDIO_CHANNEL_LAYOUT, FixDiscNumericalCheck<Plugin::AudioChannelLayout, uint64_t>},
        {CapabilityID::AUDIO_SAMPLE_FORMAT, FixDiscNumericalCheck<Plugin::AudioSampleFormat, uint8_t>},
        {CapabilityID::AUDIO_AAC_PROFILE, FixDiscNumericalCheck<Plugin::AudioAacProfile, uint8_t>},
        {CapabilityID::AUDIO_AAC_LEVEL, FixDiscNumericalCheck<Plugin::AudioAacProfile, uint8_t>},
        {CapabilityID::AUDIO_AAC_STREAM_FORMAT, FixDiscNumericalCheck<Plugin::AudioAacStreamFormat, uint8_t>},
};

static bool StringEqIgnoreCase(const std::string& s1, const std::string& s2)
{
    if (s1.length() == s2.length()) {
        return std::equal(s1.begin(), s1.end(), s2.begin(), [](char a, char b) { return tolower(a) == tolower(b); });
    }
    return false;
}

bool CompatibleWith(const Capability& capability, const Meta& meta)
{
    // first check mime
    std::string mimeInMeta;
    if (!meta.GetString(MetaID::MIME, mimeInMeta)) {
        MEDIA_LOG_E("mime is not found in meta when check compatible");
        return false;
    }

    size_t devLinePosInMeta = mimeInMeta.find_first_of('/');
    if (devLinePosInMeta == 0 || devLinePosInMeta == std::string::npos) {
        MEDIA_LOG_E("wrong format of meta mime, must be xx/xxx");
        return false;
    }

    if (capability.mime == "*") {
        return true;
    }

    size_t devLinePosInCap = capability.mime.find_first_of('/');
    if (devLinePosInCap == 0 || devLinePosInCap == std::string::npos) {
        MEDIA_LOG_E("wrong format of capability mime, must be * or xx/* or xx/xxx");
        return false;
    }

    // if media type is not the same, return false
    if (!StringEqIgnoreCase(mimeInMeta.substr(0, devLinePosInMeta), capability.mime.substr(0, devLinePosInCap))) {
        return false;
    }
    // if media type of capability is like audio/* video/* image/* etc. always return true
    if (capability.mime.substr(devLinePosInCap + 1) == "*") {
        return true;
    }
    // left mime string compare
    if (!StringEqIgnoreCase(capability.mime.substr(devLinePosInCap + 1), mimeInMeta.substr(devLinePosInMeta + 1))) {
        return false;
    }

    for (const auto& keyEntry : capability.keys) {
        auto ite = g_capabilityCheckMap.find(keyEntry.first);
        if (ite == g_capabilityCheckMap.end()) {
            MEDIA_LOG_E("found one capability which cannot be checked");
            return false;
        }
        if (!ite->second(keyEntry, meta)) {
            return false;
        }
    }
    return true;
}

bool CompatibleWith(const CapabilitySet& capability, const Meta& meta)
{
    for (const auto& cap : capability) {
        if (CompatibleWith(cap, meta)) {
            return true;
        }
    }
    return false;
}

static bool StringCapabilityCheck(const std::pair<CapabilityID, Plugin::ValueType>& tagEntry, const Meta& meta,
                                  uint8_t flags)
{
    std::string metaValue;
    if (!meta.GetString(static_cast<MetaID>(tagEntry.first), metaValue) || IsIntervalAllowed(flags)) {
        return false;
    }
    if (IsFixedAllowed(flags)) {
        if (tagEntry.second.Type() == typeid(const char*)) {
            auto capabilityValue = Plugin::AnyCast<const char*>(tagEntry.second);
            return metaValue == capabilityValue;
        } else if (tagEntry.second.Type() == typeid(char*)) {
            auto capabilityValue = Plugin::AnyCast<char*>(tagEntry.second);
            return metaValue == capabilityValue;
        } else if (tagEntry.second.Type() == typeid(std::string)) {
            auto capabilityValue = Plugin::AnyCast<std::string>(tagEntry.second);
            return metaValue == capabilityValue;
        } else if (tagEntry.second.Type() == typeid(Plugin::DiscreteCapability<const char*>)) { // 列表
            auto capabilityValues = Plugin::AnyCast<Plugin::DiscreteCapability<const char*>>(tagEntry.second);
            return std::any_of(capabilityValues.begin(), capabilityValues.end(),
                               [&metaValue](const char* cap) { return metaValue == cap; });
        }
    }
    if (IsDiscreteAllowed(flags)) {
        if (tagEntry.second.Type() == typeid(Plugin::DiscreteCapability<char*>)) { // 列表
            auto capabilityValues = Plugin::AnyCast<Plugin::DiscreteCapability<char*>>(tagEntry.second);
            return std::any_of(capabilityValues.begin(), capabilityValues.end(),
                               [&metaValue](const char* cap) { return metaValue == cap; });
        } else if (tagEntry.second.Type() == typeid(Plugin::DiscreteCapability<std::string>)) {
            auto capabilityValues = Plugin::AnyCast<Plugin::DiscreteCapability<std::string>>(tagEntry.second);
            return std::any_of(capabilityValues.begin(), capabilityValues.end(),
                               [&metaValue](const std::string& cap) { return metaValue == cap; });
        }
    }
    return false;
}

template <typename T>
bool NumericalCapabilityCheck(const std::pair<CapabilityID, Plugin::ValueType>& tagEntry, const Meta& meta,
                              uint8_t flags, std::function<int(T, T)> compareFunc)
{
    T metaValue;
    if (!meta.GetData<T>(static_cast<MetaID>(tagEntry.first), metaValue)) {
        return false;
    }
    if (IsFixedAllowed(flags) && tagEntry.second.Type() == typeid(T)) {
        auto capabilityValue = Plugin::AnyCast<T>(tagEntry.second);
        return metaValue == capabilityValue;
    }
    if (IsIntervalAllowed(flags) && tagEntry.second.Type() == typeid(Plugin::IntervalCapability<T>)) {
        auto capabilityValueRange = Plugin::AnyCast<Plugin::IntervalCapability<T>>(tagEntry.second);
        T max = std::max(capabilityValueRange.first, capabilityValueRange.second);
        T min = std::min(capabilityValueRange.first, capabilityValueRange.second);
        return compareFunc(metaValue, min) >= 0 && compareFunc(metaValue, max) <= 0;
    }
    if (IsDiscreteAllowed(flags) && tagEntry.second.Type() == typeid(Plugin::DiscreteCapability<T>)) {
        auto capabilityValues = Plugin::AnyCast<Plugin::DiscreteCapability<T>>(tagEntry.second);
        for (const auto& cap : capabilityValues) {
            if (compareFunc(metaValue, cap) == 0) {
                return true;
            }
        }
    }
    return false;
}
} // namespace Pipeline
} // namespace Media
} // namespace OHOS
