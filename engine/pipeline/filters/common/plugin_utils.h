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

#ifndef HISTREAMER_PIPELINE_FILTER_PLUGIN_UTILS_H
#define HISTREAMER_PIPELINE_FILTER_PLUGIN_UTILS_H

#include <functional>
#include <memory>

#include "foundation/error_code.h"
#include "utils/type_define.h"
#include "pipeline/core/compatible_check.h"
#include "plugin/common/plugin_types.h"
#include "plugin/common/plugin_tags.h"
#include "plugin/common/plugin_buffer.h"
#include "plugin/core/plugin_info.h"
#include "plugin/core/plugin_manager.h"
#include "plugin/core/plugin_meta.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
#define RETURN_PLUGIN_NOT_FOUND_IF_NULL(plugin)                                                                        \
    if ((plugin) == nullptr) {                                                                                         \
        return PLUGIN_NOT_FOUND;                                                                                       \
    }

/**
 * translate plugin error into pipeline error code
 * @param pluginError
 * @return
 */
ErrorCode TranslatePluginStatus(Plugin::Status pluginError);

bool TranslateIntoParameter(const int &key, OHOS::Media::Plugin::Tag &tag);

template <typename T>
ErrorCode FindPluginAndUpdate(const std::shared_ptr<const Plugin::Meta> &inMeta,
    Plugin::PluginType pluginType, std::shared_ptr<T>& plugin, std::shared_ptr<Plugin::PluginInfo>& pluginInfo,
    std::function<std::shared_ptr<T>(const std::string&)> pluginCreator);
}
}
}
#endif // HISTREAMER_PIPELINE_FILTER_PLUGIN_UTILS_H
