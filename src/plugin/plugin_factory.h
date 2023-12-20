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
#ifndef HISTREAMER_PLUGIN_FACTORY_H
#define HISTREAMER_PLUGIN_FACTORY_H

#include "plugin/plugin_manager.h"
#include "plugin/generic_plugin.h"

namespace OHOS {
namespace Media {
namespace Plugins {
template <typename T>
class AutoRegisterPlugin {
public:
    explicit AutoRegisterPlugin(const GenericPluginDef& pluginDef)
    {
        PluginManager::Instance().RegisterGenericPlugin(pluginDef);
    }

    explicit AutoRegisterPlugin(const std::vector<GenericPluginDef>& vecPluginDef)
    {
        PluginManager::Instance().RegisterGenericPlugins(vecPluginDef);
    }

    ~AutoRegisterPlugin() = default;
};
} // Plugin
} // Media
} // OHOS
#endif //HISTREAMER_PLUGIN_FACTORY_H
