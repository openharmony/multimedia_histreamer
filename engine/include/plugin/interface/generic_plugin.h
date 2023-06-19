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

#ifndef HISTREAMER_PLUGIN_INTF_GENERIC_PLUGIN_H
#define HISTREAMER_PLUGIN_INTF_GENERIC_PLUGIN_H

#include "plugin/common/plugin_caps.h"
#include "plugin/interface/plugin_base.h"
#include "plugin/interface/plugin_definition.h"

namespace OHOS {
namespace Media {
namespace Plugin {
/**
* @brief Generic plugin definition.
* It can be used to represent any user defined plugin (T), which must derived from PluginBase.
*
* @since 1.0
* @version 1.0
*/
struct GenericPluginDef : public PluginDefBase {
    CapabilitySet inCaps {};                        ///< Plug-in input capability, For details, @see Capability.
    CapabilitySet outCaps {};                       ///< Plug-in output capability, For details, @see Capability.
    PluginCreatorFunc<PluginBase> creator {nullptr};      ///< Generic plugin create function.
    uint32_t pkgVersion {};
    std::string pkgName {};
    LicenseType license {LicenseType::APACHE_V2};
    GenericPluginDef()
    {
        apiVersion = 0;                           ///< Generic plugin version.
        pluginType = PluginType::GENERIC_PLUGIN;  ///< Plugin type, MUST be GENERIC_PLUGIN.
    }
};
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif