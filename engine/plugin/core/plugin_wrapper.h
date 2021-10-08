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

#ifndef HISTREAMER_PLUGIN_CORE_WRAPPER_H
#define HISTREAMER_PLUGIN_CORE_WRAPPER_H

#include "demuxer.h"
#include "interface/demuxer_plugin.h"
#include "interface/plugin_base.h"
#include "foundation/utils.h"

namespace OHOS {
namespace Media {
namespace Plugin {
struct DataSourceWrapper : DataSource {
    DataSourceWrapper(uint32_t pkgVersion, std::shared_ptr<DataSourceHelper> dataSource)
        : version(pkgVersion), helper(std::move(dataSource))
    {
    }

    ~DataSourceWrapper() override = default;

    Status ReadAt(int64_t offset, std::shared_ptr<Buffer>& buffer, size_t expectedLen) override
    {
        return helper->ReadAt(offset, buffer, expectedLen);
    }

    Status GetSize(size_t& size) override
    {
        return helper->GetSize(size);
    }

private:
    MEDIA_UNUSED uint32_t version;
    std::shared_ptr<DataSourceHelper> helper;
};

struct AllocatorHelperWrapper : AllocatorHelper {
    AllocatorHelperWrapper(uint32_t pkgVersion, std::shared_ptr<Allocator> alloc)
        : version_(pkgVersion), allocator_(std::move(alloc)) {}

    ~AllocatorHelperWrapper() override = default;

    void* Alloc(size_t size) override
    {
        return allocator_->Alloc(size);
    }

    void Free(void* ptr) override
    {
        allocator_->Free(ptr);
    }

private:
    MEDIA_UNUSED uint32_t version_;
    std::shared_ptr<Allocator> allocator_;
};

void ConvertToMediaInfoHelper(uint32_t pkgVersion, const MediaInfo& src, MediaInfoHelper& dest);
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PLUGIN_CORE_WRAPPER_H
