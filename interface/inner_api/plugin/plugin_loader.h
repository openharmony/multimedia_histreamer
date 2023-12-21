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

#ifndef HISTREAMER_PLUGIN_LOADER_H
#define HISTREAMER_PLUGIN_LOADER_H

#include "plugin/plugin_base.h"
#include "plugin/plugin_definition.h"

namespace OHOS {
namespace Media {
namespace Plugins {
class PluginLoader {
public:
    PluginLoader(const PluginLoader &) = delete;

    PluginLoader operator=(const PluginLoader &) = delete;

    ~PluginLoader() = default;

    static std::shared_ptr<PluginLoader> Create(const std::string &name, const std::string &path);

    RegisterFunc FetchRegisterFunction();

    UnregisterFunc FetchUnregisterFunction();

private:
    PluginLoader() = default;

    static void* LoadPluginFile(const std::string &path);

    static std::shared_ptr<PluginLoader> CheckSymbol(void* handler, const std::string &name); // NOLINT: void*
private:
    RegisterFunc registerFunc_ {nullptr};
    UnregisterFunc unregisterFunc_ {nullptr};
};
} // namespace Plugins
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PLUGIN_LOADER_H
