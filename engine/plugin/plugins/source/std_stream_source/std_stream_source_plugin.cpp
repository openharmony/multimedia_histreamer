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
#ifndef OHOS_LITE
#include "std_stream_source_plugin.h"
#include "foundation/log.h"
#include "media_errors.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace StdStreamSource {
namespace {
    constexpr uint32_t INIT_MEM_CNT = 10;
    constexpr int32_t MEM_SIZE = 10240;
    constexpr uint32_t MAX_MEM_CNT = 100;
}
std::shared_ptr<SourcePlugin> StdStreamSourcePluginCreator(const std::string& name)
{
    return std::make_shared<StdStreamSourcePlugin>(name);
}

Status StdStreamSourceRegister(const std::shared_ptr<Register>& reg)
{
    SourcePluginDef definition;
    definition.name = "StdStreamSource";
    definition.description = "standard stream source";
    definition.rank = 100; // 100: max rank
    definition.protocol.emplace_back(ProtocolType::STREAM);
    definition.creator = StdStreamSourcePluginCreator;
    return reg->AddPlugin(definition);
}

PLUGIN_DEFINITION(StdStreamSource, LicenseType::APACHE_V2, StdStreamSourceRegister, [] {});

StdStreamSourcePlugin::StdStreamSourcePlugin(std::string name)
    : SourcePlugin(std::move(name))
{
    MEDIA_LOG_D("ctor called");
    pool_ = std::make_shared<AVSharedMemoryPool>("pool");
    InitPool();
}

StdStreamSourcePlugin::~StdStreamSourcePlugin()
{
    MEDIA_LOG_D("dtor called");
    ResetPool();
}

Status StdStreamSourcePlugin::Init()
{
    MEDIA_LOG_D("IN");
    return Status::OK;
}

Status StdStreamSourcePlugin::Deinit()
{
    MEDIA_LOG_D("IN");
    return Status::OK;
}

Status StdStreamSourcePlugin::Prepare()
{
    MEDIA_LOG_D("IN");
    return Status::OK;
}

Status StdStreamSourcePlugin::Reset()
{
    MEDIA_LOG_D("IN");
    return Status::OK;
}

Status StdStreamSourcePlugin::Start()
{
    MEDIA_LOG_D("IN");
    return Status::OK;
}

Status StdStreamSourcePlugin::Stop()
{
    MEDIA_LOG_D("IN");
    return Status::OK;
}

Status StdStreamSourcePlugin::GetParameter(Tag tag, ValueType& value)
{
    MEDIA_LOG_D("IN");
    return Status::OK;
}

Status StdStreamSourcePlugin::SetParameter(Tag tag, const ValueType& value)
{
    MEDIA_LOG_D("IN");
    return Status::OK;
}

Status StdStreamSourcePlugin::SetCallback(Callback* cb)
{
    MEDIA_LOG_D("IN");
    return Status::OK;
}

Status StdStreamSourcePlugin::SetSource(std::shared_ptr<MediaSource> source)
{
    dataSrc_ = source->GetDataSrc();
    FALSE_RETURN_V(dataSrc_ != nullptr, Status::ERROR_INVALID_PARAMETER);
    int64_t size = 0;
    if (dataSrc_->GetSize(size) != MSERR_OK) {
        MEDIA_LOG_E("Get size failed");
    }
    size_ = size;
    seekable_ = size == -1 ? Seekable::UNSEEKABLE : Seekable::SEEKABLE;
    return Status::OK;
}

std::shared_ptr<Buffer> StdStreamSourcePlugin::WrapAVSharedMemory(const std::shared_ptr<AVSharedMemory>& avSharedMemory,
                                                                  int32_t realLen)
{
    std::shared_ptr<Buffer> buffer = std::make_shared<Buffer>();
    std::shared_ptr<uint8_t> address = std::shared_ptr<uint8_t>(avSharedMemory->GetBase(),
                                                                [avSharedMemory](uint8_t* ptr) { ptr = nullptr; });
    buffer->WrapMemoryPtr(address, avSharedMemory->GetSize(), realLen);
    return buffer;
}

void StdStreamSourcePlugin::InitPool()
{
    AVSharedMemoryPool::InitializeOption InitOption {
        INIT_MEM_CNT,
        MEM_SIZE,
        MAX_MEM_CNT,
        AVSharedMemory::Flags::FLAGS_READ_WRITE,
        true,
        nullptr,
    };
    pool_->Init(InitOption);
}

std::shared_ptr<AVSharedMemory> StdStreamSourcePlugin::GetMemory()
{
    return pool_->AcquireMemory(MEM_SIZE); // 10240
}

void StdStreamSourcePlugin::ResetPool()
{
    pool_->Reset();
}

Status StdStreamSourcePlugin::Read(std::shared_ptr<Buffer>& buffer, size_t expectedLen)
{
    std::shared_ptr<AVSharedMemory> memory = GetMemory();
    FALSE_RETURN_V_MSG(memory != nullptr, Status::ERROR_NO_MEMORY, "allocate memory failed!");
    int32_t realLen;
    if (seekable_ == Seekable::SEEKABLE) {
        FALSE_RETURN_V(static_cast<int64_t>(offset_) < size_, Status::END_OF_STREAM);
        expectedLen = std::min(static_cast<size_t>(size_ - offset_), expectedLen);
        expectedLen = std::min(static_cast<size_t>(memory->GetSize()), expectedLen);
        realLen = dataSrc_->ReadAt(static_cast<int64_t>(offset_), expectedLen, memory);
    } else {
        expectedLen = std::min(static_cast<size_t>(memory->GetSize()), expectedLen);
        realLen = dataSrc_->ReadAt(expectedLen, memory);
    }
    if (realLen == MediaDataSourceError::SOURCE_ERROR_IO) {
        MEDIA_LOG_E("read data source error");
        return Status::ERROR_UNKNOWN;
    }
    if (realLen == MediaDataSourceError::SOURCE_ERROR_EOF) {
        MEDIA_LOG_I("eos reached");
        return Status::END_OF_STREAM;
    }
    offset_ += realLen;
    buffer = WrapAVSharedMemory(memory, realLen);
    return Status::OK;
}

Status StdStreamSourcePlugin::GetSize(size_t& size)
{
    size = size_;
    return Status::OK;
}

Seekable StdStreamSourcePlugin::GetSeekable()
{
    return seekable_;
}

Status StdStreamSourcePlugin::SeekTo(uint64_t offset)
{
    if (seekable_ == Seekable::UNSEEKABLE) {
        MEDIA_LOG_E("source is unseekable!");
        return Status::ERROR_INVALID_OPERATION;
    }
    if (offset >= static_cast<uint64_t>(size_)) {
        MEDIA_LOG_E("Invalid parameter");
        return Status::ERROR_INVALID_PARAMETER;
    }
    offset_ = offset;
    MEDIA_LOG_D("seek to offset_ " PUBLIC_LOG_U64 " success", offset_);
    return Status::OK;
}
} // namespace StdStreamSource
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif