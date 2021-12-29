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
#define HST_LOG_TAG "FilePathSinkPlugin"
#include "file_path_sink_plugin.h"
#include "foundation/log.h"

namespace OHOS {
namespace Media {
namespace Plugin {
std::shared_ptr<FileSinkPlugin> FilePathSinkPluginCreator(const std::string& name)
{
    return std::make_shared<FilePathSinkPlugin>(name);
}

const Status FilePathSinkRegister(const std::shared_ptr<Register>& reg)
{
    FileSinkPluginDef definition;
    definition.name = "file_path_sink";
    definition.description = "file path sink";
    definition.rank = 100; // 100
    definition.outputType = OutputType::URI;
    definition.creator = FilePathSinkPluginCreator;
    return reg->AddPlugin(definition);
}

PLUGIN_DEFINITION(FilePathSink, LicenseType::APACHE_V2, FilePathSinkRegister, [] {});

FilePathSinkPlugin::FilePathSinkPlugin(std::string name)
    : FileSinkPlugin(std::move(name)), fp_(nullptr), isSeekable_(true)
{
}

Status FilePathSinkPlugin::Stop()
{
    MEDIA_LOG_D("OUT");
    CloseFile();
    return Status::OK;
}

Status FilePathSinkPlugin::SetSink(const Plugin::ValueType& sink)
{
    MEDIA_LOG_D("OUT");
    if(sink.Type() != typeid(std::string)) {
        MEDIA_LOG_E("Invalid parameter to file_path_sink plugin");
        return Status::ERROR_INVALID_PARAMETER;
    }
    fileName_ = Plugin::AnyCast<std::string>(sink);
    return OpenFile();
}

bool FilePathSinkPlugin::IsSeekable()
{
    MEDIA_LOG_D("OUT");
    return isSeekable_;
}

Status FilePathSinkPlugin::SeekTo(uint64_t offset)
{
    if (fp_ == nullptr ||
        std::fseek(fp_, 0L, SEEK_END) != 0 ||
        (std::feof(fp_) && (fileSize_ = std::ftell(fp_)) == -1) ||
        (fileSize_ != -1 && offset > fileSize_)) {
        MEDIA_LOG_E("Invalid operation");
        return Status::ERROR_WRONG_STATE;
    }
    std::clearerr(fp_);
    if ((std::fseek(fp_, 0L, SEEK_SET) == 0) && (std::fseek(fp_, offset, SEEK_SET) == 0)) {
        if (std::feof(fp_)) {
            MEDIA_LOG_I("It is the end of file!");
        }
        return Status::OK;
    }
    std::clearerr(fp_);
    MEDIA_LOG_E("Seek to %" PRIu64, offset);
    return Status::ERROR_UNKNOWN;
}

Status FilePathSinkPlugin::Write(const std::shared_ptr<Buffer>& buffer)
{
    MEDIA_LOG_D("FilePathSink write begin");
    if (buffer == nullptr || buffer->IsEmpty()) {
        return Status::OK;
    }
    auto bufferData = buffer->GetMemory();
    std::fwrite(bufferData->GetReadOnlyData(), bufferData->GetSize(), 1, fp_);
    return Status::OK;
}

Status FilePathSinkPlugin::Flush()
{
    MEDIA_LOG_D("OUT");
    if (fp_) {
        MEDIA_LOG_I("flush file");
        std::fflush(fp_);
    }
    return Status::OK;
}

Status FilePathSinkPlugin::OpenFile()
{
    fp_ = std::fopen(fileName_.c_str(), "a+");
    if (fp_ == nullptr) {
        MEDIA_LOG_E("Fail to load file from %s", fileName_.c_str());
        return Status::ERROR_UNKNOWN;
    }
    MEDIA_LOG_D("fileName_: %s", fileName_.c_str());
    return Status::OK;
}

void FilePathSinkPlugin::CloseFile()
{
    if (fp_) {
        MEDIA_LOG_I("close file");
        std::fclose(fp_);
        fp_ = nullptr;
    }
}
} // namespace Plugin
} // namespace Media
} // namespace OHOS