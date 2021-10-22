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

#define LOG_TAG "FileSourcePlugin"

#include "file_source_plugin.h"
#include "foundation/log.h"
#include "plugin/common/plugin_buffer.h"
#include "plugin/common/plugin_types.h"
#include "plugin/core/plugin_manager.h"
#include "utils/utils.h"

namespace OHOS {
namespace Media {
namespace Plugin {
std::shared_ptr<SourcePlugin> FileSourcePluginCreater(const std::string& name)
{
    return std::make_shared<FileSourcePlugin>(name);
}

const Status FileSourceRegister(const std::shared_ptr<Register>& reg)
{
    SourcePluginDef definition;
    definition.name = "FileSource";
    definition.description = "File source";
    definition.rank = 100; // 100: max rank
    definition.protocol = "file";
    definition.creator = FileSourcePluginCreater;
    return reg->AddPlugin(definition);
}

PLUGIN_DEFINITION(FileSource, LicenseType::APACHE_V2, FileSourceRegister, [] {});

void* FileSourceAllocator::Alloc(size_t size)
{
    if (size == 0) {
        return nullptr;
    }
    return reinterpret_cast<void*>(new (std::nothrow) uint8_t[size]); // NOLINT: cast
}

void FileSourceAllocator::Free(void* ptr) // NOLINT: void*
{
    if (ptr != nullptr) {
        delete[](uint8_t*) ptr;
    }
}

FileSourcePlugin::FileSourcePlugin(std::string name)
    : SourcePlugin(std::move(name)), state_(State::CREATED), fileSize_(0), isSeekable_(true), position_(0)
{
    MEDIA_LOG_D("IN");
    state_ = State::CREATED;
}

FileSourcePlugin::~FileSourcePlugin()
{
    MEDIA_LOG_D("IN");
    state_ = State::DESTROYED;
}

Status FileSourcePlugin::Init()
{
    MEDIA_LOG_D("IN");
    mAllocator_ = std::make_shared<FileSourceAllocator>();
    state_ = State::INITIALIZED;
    return Status::OK;
}

Status FileSourcePlugin::Deinit()
{
    MEDIA_LOG_D("IN");
    CloseFile();
    state_ = State::DESTROYED;
    return Status::OK;
}

Status FileSourcePlugin::Prepare()
{
    MEDIA_LOG_D("IN");
    state_ = State::PREPARED;
    return Status::OK;
}

Status FileSourcePlugin::Reset()
{
    MEDIA_LOG_D("IN");
    CloseFile();
    state_ = State::INITIALIZED;
    return Status::OK;
}

Status FileSourcePlugin::Start()
{
    MEDIA_LOG_D("IN");
    state_ = State::RUNNING;
    return Status::OK;
}

Status FileSourcePlugin::Stop()
{
    MEDIA_LOG_D("IN");
    state_ = State::PREPARED;
    return Status::OK;
}

bool FileSourcePlugin::IsParameterSupported(Tag tag)
{
    MEDIA_LOG_D("IN");
    return true;
}

Status FileSourcePlugin::GetParameter(Tag tag, ValueType& value)
{
    MEDIA_LOG_D("IN");
    return Status::OK;
}

Status FileSourcePlugin::SetParameter(Tag tag, const ValueType& value)
{
    MEDIA_LOG_D("IN");
    return Status::OK;
}

std::shared_ptr<Allocator> FileSourcePlugin::GetAllocator()
{
    MEDIA_LOG_D("IN");
    return mAllocator_;
}

Status FileSourcePlugin::SetCallback(const std::shared_ptr<Callback>& cb)
{
    MEDIA_LOG_D("IN");
    return Status::OK;
}

Status FileSourcePlugin::SetSource(std::string& uri, std::shared_ptr<std::map<std::string, ValueType>> params)
{
    MEDIA_LOG_D("IN");
    if (state_ != State::INITIALIZED) {
        MEDIA_LOG_W("Wrong state: %d", state_);
        return Status::ERROR_WRONG_STATE;
    }
    auto err = ParseFileName(uri);
    if (err != Status::OK) {
        MEDIA_LOG_E("Parse file name from uri fail, uri: %s", uri.c_str());
        return err;
    }
    return OpenFile();
}

Status FileSourcePlugin::Read(std::shared_ptr<Buffer>& buffer, size_t expectedLen)
{
    if (fin_.eof() == true) {
        MEDIA_LOG_W("It is the end of file!");
        return Status::END_OF_STREAM;
    }

    std::shared_ptr<Memory> bufData;

    // There is no buffer, so alloc it
    if (buffer->IsEmpty()) {
        bufData = buffer->AllocMemory(GetAllocator(), expectedLen);
    } else {
        bufData = buffer->GetMemory();
    }
    expectedLen = std::min(static_cast<size_t>(fileSize_ - position_), expectedLen);
    expectedLen = std::min(bufData->GetCapacity(), expectedLen);

    auto ptr = bufData->GetWritableData(expectedLen);
    size_t offset = 0;
    MEDIA_LOG_I("buffer addr %p offset %zu", ptr, offset);
    fin_.read((char*)(ptr + offset), expectedLen);
    bufData->GetWritableData(fin_.gcount());
    position_ += bufData->GetSize();
    MEDIA_LOG_D("position_: %" PRIu64 ", readSize: %zu", position_, bufData->GetSize());
    return Status::OK;
}

Status FileSourcePlugin::GetSize(size_t& size)
{
    MEDIA_LOG_D("IN");
    if (!fin_.is_open()) {
        MEDIA_LOG_E("Need call SetSource() to open file first");
        return Status::ERROR_WRONG_STATE;
    }
    size = fileSize_;
    MEDIA_LOG_D("fileSize_: %zu", size);
    return Status::OK;
}

bool FileSourcePlugin::IsSeekable()
{
    MEDIA_LOG_D("IN");
    return isSeekable_;
}

Status FileSourcePlugin::SeekTo(uint64_t offset)
{
    if (!fin_.is_open() || (offset > fileSize_) || (position_ == offset)) {
        MEDIA_LOG_E("Invalid operation");
        return Status::ERROR_WRONG_STATE;
    }
    fin_.clear();
    fin_.seekg(offset, std::ios::beg);
    if (!fin_.good() || (fin_.tellg() != offset)) {
        fin_.clear();
        fin_.seekg(position_, std::ios::beg);
        MEDIA_LOG_E("Seek to %" PRIu64, offset);
        return Status::ERROR_UNKNOWN;
    }
    position_ = offset;
    if (fin_.eof()) {
        MEDIA_LOG_I("It is the end of file!");
    }
    MEDIA_LOG_D("seek to position_: %" PRIu64 " success", position_);
    return Status::OK;
}

Status FileSourcePlugin::ParseFileName(std::string& uri)
{
    if (uri.empty()) {
        MEDIA_LOG_E("uri is empty");
        return Status::ERROR_INVALID_DATA;
    }
    MEDIA_LOG_D("uri: %s", uri.c_str());
    if (uri.find("file:/") != std::string::npos) {
        if (uri.find('#') != std::string::npos) {
            MEDIA_LOG_E("Invalid file uri format: %s", uri.c_str());
            return Status::ERROR_INVALID_DATA;
        }
        auto pos = uri.find("file:");
        if (pos == std::string::npos) {
            MEDIA_LOG_E("Invalid file uri format: %s", uri.c_str());
            return Status::ERROR_INVALID_DATA;
        }
        pos += 5; // 5: offset
        if (uri.find("///", pos) != std::string::npos) {
            pos += 3; // 3: offset
        } else if (uri.find("//", pos) != std::string::npos) {
            pos += 2;                 // 2: offset
            pos = uri.find('/', pos); // skip host name
            if (pos == std::string::npos) {
                MEDIA_LOG_E("Invalid file uri format: %s", uri.c_str());
                return Status::ERROR_INVALID_DATA;
            }
            pos++;
        }
        fileName_ = uri.substr(pos);
    } else {
        fileName_ = uri;
    }
    MEDIA_LOG_I("fileName_: %s", fileName_.c_str());
    return Status::OK;
}

Status FileSourcePlugin::OpenFile()
{
    MEDIA_LOG_D("IN");
    CloseFile();
    fin_.open(fileName_.c_str(), std::ios::in | std::ios::binary);
    if (!fin_.is_open()) {
        MEDIA_LOG_E("Fail to load file from %s", fileName_.c_str());
        return Status::ERROR_UNKNOWN;
    }
    fileSize_ = GetFileSize(fileName_.c_str());
    MEDIA_LOG_D("fileName_: %s, fileSize_: %zu", fileName_.c_str(), fileSize_);
    return Status::OK;
}

void FileSourcePlugin::CloseFile()
{
    if (fin_.is_open()) {
        MEDIA_LOG_I("close file");
        fin_.close();
    }
}
} // namespace Plugin
} // namespace Media
} // namespace OHOS
