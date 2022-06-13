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
#include "monitor/download_monitor.h"
#include "hls/hls_media_downloader.h"
#include "http/http_media_downloader.h"
#include "foundation/log.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace HttpPlugin {
namespace {
constexpr int DEFAULT_BUFFER_SIZE = 200 * 1024;
}

std::shared_ptr<SourcePlugin> HttpSourcePluginCreater(const std::string& name)
{
    return std::make_shared<HttpSourcePlugin>(name);
}

Status HttpSourceRegister(std::shared_ptr<Register> reg)
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
      bufferSize_(DEFAULT_BUFFER_SIZE),
      waterline_(0),
      executor_(nullptr),
      type_(HttpType::Http_TYPE)
{
    MEDIA_LOG_D("HttpSourcePlugin IN");
}

HttpSourcePlugin::~HttpSourcePlugin()
{
    MEDIA_LOG_D("~HttpSourcePlugin IN");
    CloseUri();
}

Status HttpSourcePlugin::Init()
{
    MEDIA_LOG_D("Init IN");
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
    if (type_ == HttpType::Http_TYPE_M3U8) {
        return Status::OK;
    } else if (type_ == HttpType::Http_TYPE) {
        return Status::ERROR_DELAY_READY;
    }
    return Status::ERROR_UNKNOWN;
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
    MEDIA_LOG_I("IN");
    CloseUri();
    return Status::OK;
}

#undef ERROR_INVALID_PARAMETER

Status HttpSourcePlugin::GetParameter(Tag tag, ValueType& value)
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

Status HttpSourcePlugin::SetParameter(Tag tag, const ValueType& value)
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
    OSAL::ScopedLock lock(mutex_);
    if (executor_ != nullptr) {
        executor_->SetCallback(cb);
    }
    return Status::OK;
}

Status HttpSourcePlugin::SetSource(std::shared_ptr<MediaSource> source)
{
    MEDIA_LOG_D("SetSource IN");
    OSAL::ScopedLock lock(mutex_);
    auto uri = source->GetSourceUri();
    if (uri.find(".m3u8") != std::string::npos) {
        executor_ = std::make_shared<DownloadMonitor>(std::make_shared<HlsMediaDownloader>());
        type_ = HttpType::Http_TYPE_M3U8;
        delayReady = false;
    } else if (uri.compare(0, 4, "http") == 0) { // 0 : position, 4: count
        executor_ = std::make_shared<DownloadMonitor>(std::make_shared<HttpMediaDownloader>());
        type_ = HttpType::Http_TYPE;
    }
    FALSE_RETURN_V(executor_ != nullptr, Status::ERROR_NULL_POINTER);

    if (callback_ != nullptr) {
        executor_->SetCallback(callback_);
    }

    MEDIA_LOG_I("SetSource: " PUBLIC_LOG_S, uri.c_str());
    FALSE_RETURN_V(executor_->Open(uri), Status::ERROR_UNKNOWN);
    return Status::OK;
}

std::shared_ptr<Allocator> HttpSourcePlugin::GetAllocator()
{
    MEDIA_LOG_D("GetAllocator IN");
    return nullptr;
}

Status HttpSourcePlugin::Read(std::shared_ptr<Buffer>& buffer, size_t expectedLen)
{
    MEDIA_LOG_D("Read in");
    OSAL::ScopedLock lock(mutex_);
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
    MEDIA_LOG_D("Read finished, read size = " PUBLIC_LOG_ZU ", isEos " PUBLIC_LOG_D32, bufData->GetSize(), isEos);
    return result ? Status::OK : Status::END_OF_STREAM;
}

Status HttpSourcePlugin::GetSize(size_t& size)
{
    MEDIA_LOG_D("IN");
    OSAL::ScopedLock lock(mutex_);
    FALSE_RETURN_V(executor_ != nullptr, Status::ERROR_NULL_POINTER);
    size = executor_->GetContentLength();
    return Status::OK;
}

Seekable HttpSourcePlugin::GetSeekable()
{
    MEDIA_LOG_D("IN");
    OSAL::ScopedLock lock(mutex_);
    FALSE_RETURN_V(executor_ != nullptr, Seekable::INVALID);
    return !executor_->IsStreaming() ? Seekable::SEEKABLE : Seekable::UNSEEKABLE;
}

Status HttpSourcePlugin::SeekTo(uint64_t offset)
{
    OSAL::ScopedLock lock(mutex_);
    FALSE_RETURN_V(executor_ != nullptr, Status::ERROR_NULL_POINTER);
    FALSE_RETURN_V(!executor_->IsStreaming(), Status::ERROR_INVALID_OPERATION);
    FALSE_RETURN_V(offset <= executor_->GetContentLength(), Status::ERROR_INVALID_PARAMETER);
    FALSE_RETURN_V(executor_->Seek(offset), Status::ERROR_UNKNOWN);
    return Status::OK;
}

void HttpSourcePlugin::CloseUri()
{
    OSAL::ScopedLock lock(mutex_);
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