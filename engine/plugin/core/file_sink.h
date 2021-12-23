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

#ifndef HISTREAMER_PLUGIN_CORE_FILE_SINK_H
#define HISTREAMER_PLUGIN_CORE_FILE_SINK_H

#include "base.h"
#include "common/plugin_buffer.h"
#include "plugin/interface/file_sink_plugin.h"

namespace OHOS {
namespace Media {
namespace Plugin {
class FileSink : public Base {
public:
    enum struct Type{
        UNKNOWN = -1,
        URI, ///< sink type is uri, sink value is std::string
        FD, ///< sink type is fd, sink value is int32_t
    };

    FileSink(const FileSink &) = delete;
    FileSink operator = (const FileSink &) = delete;
    ~FileSink() override = default;

    Status SetSink(Type type, const Plugin::ValueType& sink);
    bool IsSeekable();
    Status SeekTo(uint64_t offset);
    Status Write(const std::shared_ptr<Buffer>& buffer);
    Status Flush();
private:
    friend class PluginManager;

    FileSink(uint32_t pkgVer, uint32_t apiVer, std::shared_ptr<FileSinkPlugin> plugin);

private:
    std::shared_ptr<FileSinkPlugin> fileSink_;
};
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PLUGIN_CORE_FILE_SINK_H
