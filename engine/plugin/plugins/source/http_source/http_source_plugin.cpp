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

Status HttpSourceRegister(std::shared_ptr<Register> reg)
{
    SourcePluginDef definition;
    definition.packageName = "HttpSource";
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
      bufferSize_(DEFAULT_BUFFER_SIZE),
      waterline_(0),
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
    CloseUri();
    return Status::OK;
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
    MEDIA_LOG_I("SetSource: " PUBLIC_LOG_S, uri.c_str());
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
    FALSE_RETURN_V(executor_ != nullptr, Status::ERROR_NULL_POINTER);

    if (buffer == nullptr) {
        buffer = std::make_shared<Buffer>();
    }

    std::shared_ptr<Memory>bufData;
    if (buffer->IsEmpty()) {
        bufData = buffer->AllocMemory(GetAllocator(), expectedLen);
    } else {
        bufData = buffer->GetMemory();
    }

    bool isEos = false;
    unsigned int realReadSize = 0;
    bool result = executor_->Read(bufData->GetWritableAddr(expectedLen), expectedLen, realReadSize, isEos);
    bufData->UpdateDataSize(realReadSize);
    MEDIA_LOG_D("Read finished, read size = " PUBLIC_LOG_D32 ", isEos " PUBLIC_LOG_D32, bufData->GetSize(), isEos);
    return result ? Status::OK : Status::END_OF_STREAM;
}

Status HttpSourcePlugin::GetSize(size_t &size)
{
    MEDIA_LOG_D("IN");
    size = executor_->GetContentLength();
    return Status::OK;
}

bool HttpSourcePlugin::IsSeekable()
{
    MEDIA_LOG_D("IN");
    return !executor_->IsStreaming();
}

Status HttpSourcePlugin::SeekTo(uint64_t offset)
{
    FALSE_RETURN_V(executor_ != nullptr, Status::ERROR_NULL_POINTER);
    FALSE_RETURN_V(!executor_->IsStreaming(), Status::ERROR_INVALID_OPERATION);
    FALSE_RETURN_V(offset <= executor_->GetContentLength(), Status::ERROR_INVALID_PARAMETER);
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