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

#ifndef HISTREAMER_PLUGIN_INTF_AVTRANS_INPUT_PLUGIN_H
#define HISTREAMER_PLUGIN_INTF_AVTRANS_INPUT_PLUGIN_H

#include <map>
#include <string>

#include "plugin/common/media_source.h"
#include "plugin/common/plugin_source_tags.h"
#include "plugin/common/plugin_buffer.h"
#include "plugin/common/plugin_caps.h"
#include "plugin_base.h"
#include "plugin_definition.h"

namespace OHOS {
namespace Media {
namespace Plugin {
struct AvTransInputPlugin : public PluginBase {
explicit AvTransInputPlugin(std::string name): PluginBase(std::move(name)) {}

virtual OHOS::Media::Plugin::Status PushData(const std::string& inPort, std::shared_ptr<Plugin::Buffer> buffer,
    int32_t offset) = 0;

virtual OHOS::Media::Plugin::Status SetDataCallback(std::function<void(std::shared_ptr<Plugin::Buffer>)> callback) = 0;
};

/// Avtrans input plugin api major number.
#define AVTRANS_INPUT_API_VERSION_MAJOR (1)

/// Avtrans input plugin api minor number
#define AVTRANS_INPUT_API_VERSION_MINOR (0)

/// Avtrans input plugin version
#define AVTRANS_INPUT_API_VERSION MAKE_VERSION(AVTRANS_INPUT_API_VERSION_MAJOR, AVTRANS_INPUT_API_VERSION_MINOR)

/**
* @brief Describes the avtrans input plugin information.
*
* @since 1.0
* @version 1.0
*/
struct AvTransInputPluginDef : public PluginDefBase {
    CapabilitySet outCaps;                   ///< Plug-in output capability, For details, @see Capability.
    PluginCreatorFunc<AvTransInputPlugin> creator {nullptr}; ///< Avtrans input plugin create function.
    AvTransInputPluginDef()
    {
        apiVersion = AVTRANS_INPUT_API_VERSION; ///< Avtrans input plugin version.
        pluginType = PluginType::AVTRANS_INPUT; ///< Plugin type, MUST be AVTRANS_INPUT.
    }
};
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif