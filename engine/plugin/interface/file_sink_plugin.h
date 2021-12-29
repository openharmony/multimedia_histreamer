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

#ifndef HISTREAMER_PLUGIN_INTF_FILE_SINK_PLUGIN_H
#define HISTREAMER_PLUGIN_INTF_FILE_SINK_PLUGIN_H

#include "plugin_base.h"
#include "plugin_definition.h"
#include "plugin/common/plugin_caps.h"

namespace OHOS {
namespace Media {
namespace Plugin {
struct FileSinkPlugin : public Plugin::PluginBase {
    virtual Status SetSink(const Plugin::ValueType &sink) = 0;
    virtual bool IsSeekable() = 0;
    virtual Status SeekTo(uint64_t offset) = 0;
    virtual Status Write(const std::shared_ptr<Buffer>& buffer) = 0;
    virtual Status Flush() = 0;
};

/// File Sink plugin api major number.
#define FILE_SINK_API_VERSION_MAJOR (1)

/// File Sink plugin api minor number
#define FILE_SINK_API_VERSION_MINOR (0)

/// File Sink plugin version
#define FILE_SINK_API_VERSION MAKE_VERSION(FILE_SINK_API_VERSION_MAJOR, FILE_SINK_API_VERSION_MINOR)

enum struct OutputType {
    UNKNOWN = -1,
    URI, ///< sink type is uri, sink value is std::string
    FD, ///< sink type is fd, sink value is int32_t
};

struct FileSinkPluginDef : public PluginDefBase {
    OutputType outputType;
    PluginCreatorFunc<FileSinkPlugin> creator {nullptr}; ///< Muxer plugin create function.
    FileSinkPluginDef()
    {
        apiVersion = FILE_SINK_API_VERSION; ///< file sink plugin version.
        pluginType = PluginType::FILE_SINK; ///< Plugin type, MUST be FILE_SINK.
    }
};
} // Plugin
} // Media
} // OHOS
#endif // HISTREAMER_PLUGIN_INTF_FILE_SINK_PLUGIN_H
