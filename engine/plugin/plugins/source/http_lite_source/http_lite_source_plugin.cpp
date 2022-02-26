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
#define HST_LOG_TAG "HttpLiteSourcePlugin"
#include "http_lite_source_plugin.h"
#include "foundation/log.h"
#include "plugin/common/plugin_types.h"
#include "plugin/core/plugin_manager.h"
#include "utils/util.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace HttpLitePlugin {
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
    definition.name = "HttpLiteSource";
    definition.description = "Http lite source";
    definition.rank = 100; // 100
    definition.protocol.emplace_back(ProtocolType::HTTP);
    definition.protocol.emplace_back(ProtocolType::HTTPS);
    definition.creator = HttpSourcePluginCreater;
    return reg->AddPlugin(definition);
}
PLUGIN_DEFINITION(HttpLiteSource, LicenseType::APACHE_V2, HttpSourceRegister, [] {});

void* HttpSourceAllocator::Alloc(size_t size)
{
    if (size == 0) {
        return nullptr;
    }
    return reinterpret_cast<void*>(new (std::nothrow) uint8_t[size]); // NOLINT: cast
}

void HttpSourceAllocator::Free(void* ptr) // NOLINT: void*
{
    if (ptr != nullptr) {
        delete[](uint8_t*) ptr;
    }
}

HttpSourcePlugin::HttpSourcePlugin(const std::string name) noexcept
    : SourcePlugin(std::move(name)),
      url_(""),
      certFile_(""),
      needExit_(false),
      isSeekable_(false),
      bufferSize_(DEFAULT_BUFFER_SIZE),
      position_(0),
      waterline_(0),
      fileSize_(-1),
      httpHandle_(nullptr),
      mAllocator_(nullptr),
      httpMutex_()
{
    MEDIA_LOG_D("HttpSourcePlugin IN");
}

HttpSourcePlugin::~HttpSourcePlugin()
{
    MEDIA_LOG_D("~HttpSourcePlugin IN");
}

Status HttpSourcePlugin::Init()
{
    OSAL::ScopedLock lock(httpMutex_);
    MEDIA_LOG_D("Init IN");
    httpHandle_ = std::make_shared<HttpLiteManager>();
    if (httpHandle_ == nullptr) {
        MEDIA_LOG_E("httpHandle_ create error");
        return Status::ERROR_UNKNOWN;
    }
    mAllocator_ = std::make_shared<HttpSourceAllocator>();
    if (mAllocator_ == nullptr) {
        MEDIA_LOG_E("mAllocator_ create error");
        return Status::ERROR_UNKNOWN;
    }
    MEDIA_LOG_D("Init OUT");
    return Status::OK;
}

Status HttpSourcePlugin::Deinit()
{
    OSAL::ScopedLock lock(httpMutex_);
    MEDIA_LOG_D("IN");
    CloseUri();
    return Status::OK;
}

Status HttpSourcePlugin::Prepare()
{
    OSAL::ScopedLock lock(httpMutex_);
    MEDIA_LOG_D("IN");
    return Status::OK;
}

Status HttpSourcePlugin::Reset()
{
    needExit_ = true;
    {
        OSAL::ScopedLock lock(httpMutex_);
        needExit_ = false;
        MEDIA_LOG_D("IN");
        CloseUri();
        return Status::OK;
    }
}

Status HttpSourcePlugin::Start()
{
    OSAL::ScopedLock lock(httpMutex_);
    MEDIA_LOG_D("IN");
    if (isSeekable_ && httpHandle_ != nullptr) {
        waterline_ = 20; // 20
        httpHandle_->SetWaterline(waterline_, 0);
    }
    MEDIA_LOG_D("OUT");
    return Status::OK;
}

Status HttpSourcePlugin::Stop()
{
    needExit_ = true;
    {
        MEDIA_LOG_D("IN");
        OSAL::ScopedLock lock(httpMutex_);
        needExit_ = false;
        if (httpHandle_ != nullptr) {
            httpHandle_->HttpClose();
            httpHandle_ = nullptr;
        }
        MEDIA_LOG_D("OUT");
        return Status::ERROR_UNKNOWN;
    }
}

Status HttpSourcePlugin::GetParameter(Tag tag, ValueType &value)
{
    OSAL::ScopedLock lock(httpMutex_);
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
    OSAL::ScopedLock lock(httpMutex_);
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
    return Status::OK;
}

Status HttpSourcePlugin::SetSource(std::shared_ptr<MediaSource> source)
{
    OSAL::ScopedLock lock(httpMutex_);
    MEDIA_LOG_D("SetSource IN");
    if (httpHandle_ == nullptr) {
        MEDIA_LOG_D("httpHandle_ null error");
        return Status::ERROR_UNKNOWN;
    }
    auto uri = source->GetSourceUri();
    MEDIA_LOG_D(PUBLIC_LOG "s", uri.c_str());
    Status ret = OpenUri(uri);
    if (ret != Status::OK) {
        MEDIA_LOG_D("OpenUri error");
        return ret;
    }
    MEDIA_LOG_D("OpenUri success");
    unsigned int downloadPos = 0;
    httpHandle_->GetHttpBufferRange(&position_, &downloadPos);
    MEDIA_LOG_D("position_ " PUBLIC_LOG "d downloadPos " PUBLIC_LOG "d",
                (uint32_t)position_, (uint32_t)downloadPos);
    int8_t retryTimes = 0;
    while (!needExit_ && position_ == downloadPos && retryTimes < 60) { // 60
        OHOS::Media::OSAL::SleepFor(200); // 200
        httpHandle_->GetHttpBufferRange(&position_, &downloadPos);
        retryTimes++;
    }
    MEDIA_LOG_D("position_ " PUBLIC_LOG "d downloadPos " PUBLIC_LOG "d", position_, downloadPos);
    if (position_ == downloadPos) {
        MEDIA_LOG_D("position_ == downloadPos");
        httpHandle_->HttpClose();
        return Status::ERROR_UNKNOWN;
    }
    isSeekable_ = httpHandle_->IsStreaming();
    fileSize_ = isSeekable_ ? httpHandle_->GetContentLength() : -1;
    MEDIA_LOG_D("SetSource OUT fileSize_ " PUBLIC_LOG "d", fileSize_);
    return Status::OK;
}

std::shared_ptr<Allocator> HttpSourcePlugin::GetAllocator()
{
    MEDIA_LOG_D("GetAllocator IN");
    return mAllocator_;
}

void HttpSourcePlugin::OnError(int httpError, int localError, void *param, int support_retry)
{
    MEDIA_LOG_D("httpError " PUBLIC_LOG "d localError " PUBLIC_LOG "d", httpError, localError);
    auto plugin = reinterpret_cast<HttpSourcePlugin *>(param);
    if (plugin == nullptr) {
        return;
    }
    plugin->needExit_ = true;
    plugin->OnHttpEvent(param, httpError, localError);
}

Status HttpSourcePlugin::OnHttpEvent(void *priv, int errorType, int32_t errorCode)
{
    if (priv == nullptr) {
        MEDIA_LOG_D("priv null error");
        return Status::ERROR_UNKNOWN;
    }
    auto plugin = reinterpret_cast<HttpSourcePlugin *>(priv);
    plugin->callback_->OnEvent(
        PluginEvent{PluginEventType::OTHER_ERROR, errorCode, "http lite error"});
    return Status::OK;
}

Status HttpSourcePlugin::Read(std::shared_ptr<Buffer> &buffer, size_t expectedLen)
{
    MEDIA_LOG_D("Read in");
    if (httpHandle_ == nullptr || buffer == nullptr) {
        MEDIA_LOG_D("Read error");
        return Status::ERROR_INVALID_PARAMETER;
    }
    {
        OSAL::ScopedLock lock(httpMutex_);
        std::shared_ptr<Memory>bufData;

        if (buffer->IsEmpty()) {
            bufData = buffer->AllocMemory(GetAllocator(), expectedLen);
        } else {
            bufData = buffer->GetMemory();
        }
        unsigned int read = 0;
        unsigned int write = 0;
        unsigned int realReadSize = 0;
        bool isEos = false;

        httpHandle_->GetHttpBufferRange(&read, &write);

        MEDIA_LOG_I("read pos " PUBLIC_LOG "d write pos " PUBLIC_LOG "d expectedLen " PUBLIC_LOG "d",
                    read, write, expectedLen);

        expectedLen = std::min(static_cast<size_t>(write - read), expectedLen);
        expectedLen = std::min(bufData->GetCapacity(), expectedLen);

        MEDIA_LOG_I("bufData->GetCapacity() " PUBLIC_LOG "d", bufData->GetCapacity());
        httpHandle_->HttpRead(bufData->GetWritableAddr(expectedLen), expectedLen, realReadSize, isEos);
        bufData->UpdateDataSize(realReadSize);
        httpHandle_->GetHttpBufferRange(&position_, &write);
        MEDIA_LOG_D("position_ : " PUBLIC_LOG "d, readSize = " PUBLIC_LOG "d, isEos " PUBLIC_LOG "d",
                    position_, bufData->GetSize(), isEos);
        return Status::OK;
    }
}

Status HttpSourcePlugin::GetSize(size_t &size)
{
    OSAL::ScopedLock lock(httpMutex_);
    MEDIA_LOG_D("IN");
    size = fileSize_;
    return Status::OK;
}

bool HttpSourcePlugin::IsSeekable()
{
    OSAL::ScopedLock lock(httpMutex_);
    MEDIA_LOG_D("IN");
    return isSeekable_;
}

Status HttpSourcePlugin::SeekTo(uint64_t offset)
{
    OSAL::ScopedLock lock(httpMutex_);
    unsigned int readPos = 0;
    unsigned int writePos = 0;
    uint32_t readLength;
    uint8_t tmpBuf;
    bool sourceFlag;
    if ((httpHandle_ == nullptr) || (!isSeekable_) || (position_ == offset) || (offset > fileSize_)) {
        MEDIA_LOG_E("Invalid operation");
        return Status::ERROR_INVALID_PARAMETER;
    }
    if (!httpHandle_->HttpSeek(offset)) {
        MEDIA_LOG_D("seek to position_ " PUBLIC_LOG "d failed", position_);
        return Status::ERROR_UNKNOWN;
    }
    position_ = static_cast<unsigned int>(offset);
    httpHandle_->GetHttpBufferRange(&readPos, &writePos);
    MEDIA_LOG_D("offset = " PUBLIC_LOG "d, after SeekTo readPos = " PUBLIC_LOG "d, writePos = " PUBLIC_LOG
                "d", static_cast<uint32_t>(offset), readPos, writePos);
    MEDIA_LOG_D("seek to position_ " PUBLIC_LOG "d success", position_);
    return Status::OK;
}

Status HttpSourcePlugin::OpenUri(std::string &url)
{
    MEDIA_LOG_D("OpenUri IN");
    if (httpHandle_ == nullptr) {
        return Status::ERROR_UNIMPLEMENTED;
    }
    httpHandle_->HttpClose();
    HttpLiteAttr httpAttr;
    httpAttr.certFile = certFile_;
    httpAttr.priority = -1;
    httpAttr.bufferSize = bufferSize_;
    httpAttr.pluginHandle = this;
    httpAttr.callbackFunc = OnError;
    MEDIA_LOG_D("OpenUri httpAttr.pluginHandle " PUBLIC_LOG "p httpAttr.callbackFunc " PUBLIC_LOG "p",
                httpAttr.pluginHandle, httpAttr.callbackFunc);
    return httpHandle_->HttpOpen(url, httpAttr) ? Status::OK : Status::ERROR_UNKNOWN;
}

void HttpSourcePlugin::CloseUri()
{
    if (httpHandle_ != nullptr) {
        MEDIA_LOG_D("close uri");
        httpHandle_->HttpClose();
        httpHandle_ = nullptr;
    }
}
} // namespace HttpLitePlugin
} // namespace Plugin
} // namespace Media
} // namespace OHOS