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

#include "file_sink.h"
#include "plugin_wrapper.h"

namespace OHOS {
namespace Media {
namespace Plugin {
FileSink::FileSink(uint32_t pkgVer, uint32_t apiVer, std::shared_ptr<FileSinkPlugin> plugin)
        : Base(pkgVer, apiVer, plugin), fileSink_(std::move(plugin)) {}
Status FileSink::SetSink(Type type, const Plugin::ValueType& sink)
{
    FileSinkPlugin::Type pluginType = FileSinkPlugin::Type::UNKNOWN;
    ConvertToSinkType(pkgVersion_, type, pluginType);
    return fileSink_->SetSink(pluginType, sink);
}
bool FileSink::IsSeekable()
{
    return fileSink_->IsSeekable();
}
Status FileSink::SeekTo(uint64_t offset)
{
    return fileSink_->SeekTo(offset);
}
Status FileSink::Write(const std::shared_ptr<Buffer>& buffer)
{
    return fileSink_->Write(buffer);
}
Status FileSink::Flush()
{
    return fileSink_->Flush();
}
} // namespace Plugin
} // namespace Media
} // namespace OHOS