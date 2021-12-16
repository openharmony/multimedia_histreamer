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
#ifdef RECORDER_SUPPORT
#include "file_sink_filter.h"

#include "factory/filter_factory.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
static AutoRegisterFilter<FileSinkFilter> g_registerFilterHelper("builtin.recorder.file_sink");

FileSinkFilter::FileSinkFilter(std::string name) : FilterBase(std::move(name)) {}

FileSinkFilter::~FileSinkFilter() {}

ErrorCode FileSinkFilter::SetOutputPath(const std::string &path)
{
    return ErrorCode::SUCCESS;
}

ErrorCode FileSinkFilter::SetFd(int32_t fd)
{
    return ErrorCode::SUCCESS;
}

ErrorCode FileSinkFilter::SetMaxFileSize(uint64_t maxFileSize)
{
    return ErrorCode::SUCCESS;
}
} // Pipeline
} // Media
} // OHOS
#endif