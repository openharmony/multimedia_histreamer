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
#ifndef HISTREAMER_MEDIA_DATA_SOURCE_IMPL_H
#define HISTREAMER_MEDIA_DATA_SOURCE_IMPL_H

#include "common/media_data_source.h"
#include "plugin/common/plugin_types.h"

namespace OHOS {
namespace Media {
class IMediaDataSourceImpl : public IMediaDataSource {
public:
    IMediaDataSourceImpl(std::string url, Plugin::Seekable seekable);
    int32_t ReadAt(int64_t pos, uint32_t length, const std::shared_ptr<AVSharedMemory> &mem) override;
    int32_t ReadAt(uint32_t length, const std::shared_ptr<AVSharedMemory> &mem) override;
    int32_t ReadAt(const std::shared_ptr<AVSharedMemory> &mem, uint32_t length, int64_t pos = -1) override;
    int32_t GetSize(int64_t &size) override;
private:
    int ReadDataFromFile();
    uint32_t GetDataFromSource(const std::shared_ptr<AVSharedMemory> &mem, uint32_t length);
private:
    std::string url_;
    int32_t size_;
    uint32_t readPos_;
    std::vector<uint32_t> data_;
    Plugin::Seekable seekable_;
};
}
}

#endif //HISTREAMER_MEDIA_DATA_SOURCE_IMPL_H
