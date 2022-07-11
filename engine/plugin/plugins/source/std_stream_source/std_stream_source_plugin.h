/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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
#ifndef HISTREAMER_STD_STREAM_SOURCE_PLUGIN_H
#define HISTREAMER_STD_STREAM_SOURCE_PLUGIN_H

#ifndef OHOS_LITE

#include "avsharedmemorypool.h"
#include "plugin/common/plugin_types.h"
#include "plugin/interface/source_plugin.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace StdStreamSource {
class StdStreamSourcePlugin : public SourcePlugin {
public:
    explicit StdStreamSourcePlugin(std::string name);
    ~StdStreamSourcePlugin();

    Status Init() override;
    Status Deinit() override;
    Status Prepare() override;
    Status Reset() override;
    Status Start() override;
    Status Stop() override;
    Status GetParameter(Tag tag, ValueType& value) override;
    Status SetParameter(Tag tag, const ValueType& value) override;
    Status SetCallback(Callback* cb) override;
    Status SetSource(std::shared_ptr<MediaSource> source) override;
    Status Read(std::shared_ptr<Buffer>& buffer, size_t expectedLen) override;
    Status GetSize(size_t& size) override;
    Seekable GetSeekable() override;
    Status SeekTo(uint64_t offset) override;
private:
    std::shared_ptr<Buffer> WrapAVSharedMemory(const std::shared_ptr<AVSharedMemory>& avSharedMemory, int32_t realLen);
    void InitPool();
    std::shared_ptr<AVSharedMemory> GetMemory();
    void ResetPool();
    Seekable seekable_ {Seekable::INVALID};
    std::shared_ptr<IMediaDataSource> dataSrc_;
    std::shared_ptr<AVSharedMemoryPool> pool_;
    int64_t size_ {0};
    uint64_t offset_ {0};
};
} // namespace StdStreamSource
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif
#endif // HISTREAMER_STD_STREAM_SOURCE_PLUGIN_H
