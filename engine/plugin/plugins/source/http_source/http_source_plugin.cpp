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
#define HST_LOG_TAG "HttpSourcePlugin"
#include "http_source_plugin.h"
#include "plugin/core/plugin_manager.h"
#include "utils/util.h"
#include "foundation/log.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace HttpPlugin {
namespace {
constexpr int DEFAULT_BUFFER_SIZE = 200 * 1024;
}

std::shared_ptr<SourcePlugin> HttpSourcePluginCreater(const std::string &name)
{
    return std::make_shared<HttpSourcePlugin>(name);
}

const Status HttpSourceRegister(std::shared_ptr<Register> reg)
{
    SourcePluginDef definition;
    definition.name = "HttpSource";
    definition.description = "Http source";
    definition.rank = 100; // 100
    definition.protocol.emplace_back(ProtocolType::HTTP);
    definition.protocol.emplace_back(ProtocolType::HTTPS);
    definition.creator = HttpSourcePluginCreater;
    return reg->AddPlugin(definition);
}
PLUGIN_DEFINITION(HttpSource, LicenseType::APACHE_V2, HttpSourceRegister, [] {});

HttpSourcePlugin::HttpSourcePlugin(std::string name) noexcept
    : SourcePlugin(std::move(name)),
      isSeekable_(false),
      bufferSize_(DEFAULT_BUFFER_SIZE),
      waterline_(0),
      fileSize_(-1),
      executor_(nullptr)
{
    MEDIA_LOG_D("HttpSourcePlugin IN");
}

HttpSourcePlugin::~HttpSourcePlugin()
{
    MEDIA_LOG_D("~HttpSourcePlugin IN");
    if (executor_ != nullptr) {
        executor_->Close();
        executor_ = nullptr;
    }
}

Status HttpSourcePlugin::Init()
{
    MEDIA_LOG_D("Init IN");
    executor_ = std::make_shared<StreamingExecutor>();
    return Status::OK;
}

Status HttpSourcePlugin::Deinit()
{
    MEDIA_LOG_D("IN");
    CloseUri();
    return Status::OK;
}

Status HttpSourcePlugin::Prepare()
{
    MEDIA_LOG_D("IN");
    return Status::ERROR_DELAY_READY;
}

Status HttpSourcePlugin::Reset()
{
    MEDIA_LOG_D("IN");
    CloseUri();
    return Status::OK;
}

Status HttpSourcePlugin::Start()
{
    MEDIA_LOG_D("IN");
    return Status::OK;
}

Status HttpSourcePlugin::Stop()
{
    MEDIA_LOG_D("IN");
    return Status::OK;
}

bool HttpSourcePlugin::IsParameterSupported(Tag tag)
{
    MEDIA_LOG_D("IN");
    if (tag == Tag::BUFFERING_SIZE || tag == Tag::WATERLINE_HIGH) {
        return true;
    }
    return false;
}

#undef ERROR_INVALID_PARAMETER

Status HttpSourcePlugin::GetParameter(Tag tag, ValueType &value)
{
    MEDIA_LOG_D("IN");
    switch (tag) {
        case Tag::BUFFERING_SIZE:
            value = bufferSize_;
            return Status::OK;
        case Tag::WATERLINE_HIGH:
            value = waterline_;
            return Status::OK;
        default:
            return Status::ERROR_INVALID_PARAMETER;
    }
}

Status HttpSourcePlugin::SetParameter(Tag tag, const ValueType &value)
{
    MEDIA_LOG_D("IN");
    switch (tag) {
        case Tag::BUFFERING_SIZE:
            bufferSize_ = AnyCast<uint32_t>(value);
            return Status::OK;
        case Tag::WATERLINE_HIGH:
            waterline_ = AnyCast<uint32_t>(value);
            return Status::OK;
        default:
            return Status::ERROR_INVALID_PARAMETER;
    }
}

Status HttpSourcePlugin::SetCallback(Callback* cb)
{
    MEDIA_LOG_D("IN");
    callback_ = cb;
    executor_->SetCallback(cb);
    return Status::OK;
}

Status HttpSourcePlugin::SetSource(std::shared_ptr<MediaSource> source)
{
    MEDIA_LOG_D("SetSource IN");
    FALSE_RETURN_V(executor_ != nullptr, Status::ERROR_NULL_POINTER);

    auto uri = source->GetSourceUri();
    FALSE_RETURN_V(executor_->Open(uri), Status::ERROR_FUNCTION_CALL);
    isSeekable_ = !executor_->IsStreaming();
    fileSize_ = isSeekable_ ? executor_->GetContentLength() : -1;
    MEDIA_LOG_I("SetSource(%" PUBLIC_LOG "s), seekable: %" PUBLIC_LOG "d, file size: %" PUBLIC_LOG "d",
                uri.c_str(), isSeekable_, fileSize_);
    return Status::OK;
}

std::shared_ptr<Allocator> HttpSourcePlugin::GetAllocator()
{
    MEDIA_LOG_D("GetAllocator IN");
    return nullptr;
}

Status HttpSourcePlugin::Read(std::shared_ptr<Buffer> &buffer, size_t expectedLen)
{
    MEDIA_LOG_D("Read in");
    FALSE_RETURN_V(executor_ != nullptr && buffer != nullptr, Status::ERROR_NULL_POINTER);

    std::shared_ptr<Memory>bufData;
    if (buffer->IsEmpty()) {
        bufData = buffer->AllocMemory(GetAllocator(), expectedLen);
    } else {
        bufData = buffer->GetMemory();
    }

    bool isEos = false;
    unsigned int realReadSize = 0;
    executor_->Read(bufData->GetWritableAddr(expectedLen), expectedLen, realReadSize, isEos);
    bufData->UpdateDataSize(realReadSize);
    MEDIA_LOG_D("Read finished, read size = %" PUBLIC_LOG "d, isEos %" PUBLIC_LOG "d", bufData->GetSize(), isEos);
    return Status::OK;
}

Status HttpSourcePlugin::GetSize(size_t &size)
{
    MEDIA_LOG_D("IN");
    fileSize_ = executor_->GetContentLength();
    size = fileSize_;
    return Status::OK;
}

bool HttpSourcePlugin::IsSeekable()
{
    MEDIA_LOG_D("IN");
    return isSeekable_;
}

Status HttpSourcePlugin::SeekTo(uint64_t offset)
{
    FALSE_RETURN_V(executor_ != nullptr, Status::ERROR_NULL_POINTER);
    FALSE_RETURN_V(isSeekable_, Status::ERROR_INVALID_OPERATION);
    FALSE_RETURN_V(offset <= fileSize_, Status::ERROR_INVALID_PARAMETER);
    FALSE_RETURN_V(executor_->Seek(offset), Status::ERROR_FUNCTION_CALL);
    return Status::OK;
}

void HttpSourcePlugin::CloseUri()
{
    if (executor_ != nullptr) {
        MEDIA_LOG_D("Close uri");
        executor_->Close();
        executor_ = nullptr;
    }
}
}
}
}
}