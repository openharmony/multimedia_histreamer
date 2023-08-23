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

#ifndef HISTREAMER_PIPELINE_FILTER_PLUGIN_SETTINGS_H
#define HISTREAMER_PIPELINE_FILTER_PLUGIN_SETTINGS_H

#include <functional>
#include <tuple>

#include "pipeline/core/filter_type.h"
#include "plugin/common/plugin_tags.h"
#include "plugin/common/plugin_types.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
constexpr uint8_t PARAM_GET = 0x01;
constexpr uint8_t PARAM_SET = 0x02;
using PluginParaAllowedMap = std::map<Plugin::Tag,
    std::pair<std::function<bool(Plugin::Tag, const Plugin::ValueType&)>, uint8_t>>;

class PluginParameterTable {
public:
    static PluginParameterTable& GetInstance();
    PluginParameterTable();
    const PluginParaAllowedMap& FindAllowedParameterMap(FilterType category);

private:
    void Init();
    std::map<FilterType, PluginParaAllowedMap> table_;
    void InitAudioTable();
    void InitMuxerTable();
    void InitVideoTable();
};
} // Pipeline
} // Media
} // OHOS
#endif // HISTREAMER_PIPELINE_FILTER_PLUGIN_SETTINGS_H
