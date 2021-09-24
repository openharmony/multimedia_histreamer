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

#ifndef MEDIA_PIPELINE_FILE_SOURCE_PLUGIN_H
#define MEDIA_PIPELINE_FILE_SOURCE_PLUGIN_H

#include <fstream>
#include "plugin/common/plugin_types.h"
#include "plugin/interface/source_plugin.h"

namespace OHOS {
namespace Media {
namespace Plugin {
class FileSourceAllocator : public Allocator {
public:
    FileSourceAllocator() = default;
    ~FileSourceAllocator() = default;

    void* Alloc(size_t size) override;
    void Free(void* ptr) override;
};

class FileSourcePlugin : public SourcePlugin {
public:
    explicit FileSourcePlugin(const std::string &name);
    ~FileSourcePlugin();

    Status Init() override;
    Status Deinit() override;
    Status Prepare() override;
    Status Reset() override;
    Status Start() override;
    Status Stop() override;
    bool IsParameterSupported(Tag tag) override;
    Status GetParameter(Tag tag, ValueType &value) override;
    Status SetParameter(Tag tag, const ValueType &value) override;
    Status GetState(State &state) override;
    std::shared_ptr<Allocator> GetAllocator() override;
    Status SetCallback(const std::shared_ptr<Callback> &cb) override;
    Status SetSource(
                std::string& uri, std::shared_ptr<std::map<std::string, ValueType>> params) override;
    Status Read(std::shared_ptr<Buffer> &buffer, size_t expectedLen) override;
    Status GetSize(size_t& size) override;
    bool IsSeekable() override;
    Status SeekTo(uint64_t offset) override;

private:
    std::string name_;
    State state_;
    std::string fileName_ {};
    std::ifstream fin_;
    size_t fileSize_;
    bool isSeekable_;
    uint64_t position_;
    std::shared_ptr<FileSourceAllocator> mAllocator_ {nullptr};

    Status ParseFileName(std::string& uri);
    Status OpenFile();
    void CloseFile();
};
}
}
}

#endif // MEDIA_PIPELINE_FILE_SOURCE_PLUGIN_H
