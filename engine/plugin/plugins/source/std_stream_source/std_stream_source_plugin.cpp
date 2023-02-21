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
    constexpr uint32_t DEFAULT_BUFFER_SIZE = 4096;
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
}

StdStreamSourcePlugin::~StdStreamSourcePlugin()
{
    MEDIA_LOG_D("dtor called");
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
    auto ret = dataSrc_->GetSize(size);
    FALSE_RETURN_V_MSG(ret == MSERR_OK, Status::ERROR_INVALID_DATA, "Media data source get size failed!");
    FALSE_RETURN_V_MSG(size >= -1, Status::ERROR_INVALID_DATA,
        "invalid file size, if unknow file size please set size = -1");
    size_ = size;
    seekable_ = size_ == -1 ? Seekable::UNSEEKABLE : Seekable::SEEKABLE;
    avSharedMemory_ = AVDataSrcMemory::CreateFromLocal(
        DEFAULT_BUFFER_SIZE, AVSharedMemory::Flags::FLAGS_READ_WRITE, "AppSrc");
    FALSE_RETURN_V_MSG(avSharedMemory_ != nullptr, Status::ERROR_NO_MEMORY, "init AVSharedMemory failed");
    offset_ = 0;
    return Status::OK;
}

Status StdStreamSourcePlugin::Read(std::shared_ptr<Buffer>& buffer, size_t expectedLen)
{
    FALSE_RETURN_V_MSG(avSharedMemory_ != nullptr, Status::ERROR_NO_MEMORY, "no mem");
    expectedLen = std::min(DEFAULT_BUFFER_SIZE, static_cast<uint32_t>(expectedLen));
    std::static_pointer_cast<AVDataSrcMemory>(avSharedMemory_)->SetOffset(0);
    int32_t realLen;
    if (size_ == -1) {
        MEDIA_LOG_D("Read length is " PUBLIC_LOG_U32, expectedLen);
        realLen = dataSrc_->ReadAt(avSharedMemory_, expectedLen);
    } else {
        MEDIA_LOG_D("Read length is " PUBLIC_LOG_U32 ", pos is " PUBLIC_LOG_U64, expectedLen, offset_);
        realLen = dataSrc_->ReadAt(avSharedMemory_, expectedLen, offset_);
    }
    MEDIA_LOG_D("ReadAt end");
    if (realLen == MediaDataSourceError::SOURCE_ERROR_IO) {
        MEDIA_LOG_E("read data source error");
        return Status::ERROR_UNKNOWN;
    } else if (realLen == MediaDataSourceError::SOURCE_ERROR_EOF) {
        MEDIA_LOG_I("eos reached");
        return Status::END_OF_STREAM;
    } else if (realLen > 0) {
        offset_ += realLen;
        MEDIA_LOG_D("offset_ update to " PUBLIC_LOG_U64, offset_);
    }
    buffer->AllocMemory(nullptr, realLen);
    buffer->GetMemory()->Write(avSharedMemory_->GetBase(), realLen);
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