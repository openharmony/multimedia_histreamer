/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef HISTREAMER_PLUGIN_CAPS_H
#define HISTREAMER_PLUGIN_CAPS_H

#include <utility>
#include <map> // NOLINT: used it
#include <vector> // NOLINT: used it
#include "meta/meta_key.h"
#include "meta/any.h"

namespace OHOS {
namespace Media {
namespace Plugins {
/// Indicates that the available capability type is an fixed value.
template <typename T> using FixedCapability = T;

/// Indicates that the available capability type is an interval value.
template <typename T> using IntervalCapability  = std::pair<T, T>;

/// Indicates that the available capability types are discrete values.
template <typename T> using DiscreteCapability = std::vector<T>;

/**
 * @brief The Capability describes the input and output capabilities of the plugin.
 *
 * It is basically a set of tags attached to the mime-type in order to
 * describe the mime-type more closely.
 *
 * @since 1.0
 * @version 1.0
 */
struct Capability {
    /// default constructor
    Capability() = default;

    /**
     * @brief constructor one capability with mime of m
     *
     * @param m mime string
     */
    explicit Capability(std::string m) : mime(std::move(m)) {}

    /**
     * @brief Append one fix key into KeyMap
     *
     * @tparam T type of value
     * @param key Capability::Key
     * @param val value
     * @return reference of object
     */
    template<typename T>
    Capability& AppendFixedKey(TagType key, const T& val)
    {
        keys[key] = val;
        return *this;
    }

    template<typename T>
    void GetFixedValue(TagType key, T& val)
    {
        auto it = keys.find(key);
        if (it != keys.end()) {
            val = it->second;
        }
    }

    /**
     * @brief Append one interval key i.e. [rangeStart, rangeEnd] into KeyMap
     *
     * @tparam T type of value
     * @param key Capability::Key
     * @param rangeStart range start
     * @param rangeEnd rang end
     * @return reference of object
     */
    template<typename T>
    Capability& AppendIntervalKey(TagType key, const T& rangeStart, const T& rangeEnd)
    {
        keys[key] = std::make_pair(rangeStart, rangeEnd);
        return *this;
    }

    /**
     * @brief Append one discrete key i.e. {val1, val2, ....} into KeyMap
     *
     * @tparam T type of value
     * @param key Capability::Key
     * @param discreteValues values
     * @return reference of object
     */
    template<typename T>
    Capability& AppendDiscreteKeys(TagType key, DiscreteCapability<T> discreteValues)
    {
        keys[key] = std::move(discreteValues);
        return *this;
    }

    /**
     * @brief set mime of this capability
     *
     * @param val mime value
     * @return reference of object
     */
    Capability& SetMime(std::string val)
    {
        mime = std::move(val);
        return *this;
    }

    /// mime of capability. For details, see {@link constants.h}
    std::string mime;

    /// Store the parameters(Capability::Key, value pairs), which should be negotiated
    std::map<TagType, Any> keys;
};

/// A collection of multiple capabilities
using CapabilitySet = std::vector<Capability>;
using ValueType = Any;
} // namespace Plugins
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PLUGIN_CAPS_H
