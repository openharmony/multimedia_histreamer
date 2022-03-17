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
#define HST_LOG_TAG "StreamingExecutor"

#include "streaming_executor.h"
#include <algorithm>
#include <functional>
#include "securec.h"
#include "osal/utils/util.h"
#include "steady_clock.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace HttpPlugin {
namespace {
constexpr int RING_BUFFER_SIZE = 5 * 48 * 1024;
constexpr int PER_REQUEST_SIZE = 48 * 1024;
constexpr int WATER_LINE = RING_BUFFER_SIZE * 0.1;
constexpr unsigned int SLEEP_TIME = 5;    // Sleep 5ms
constexpr unsigned int RETRY_TIMES = 200;  // Retry 200 times
}

StreamingExecutor::StreamingExecutor() noexcept
{
    buffer_ = std::make_shared<RingBuffer>(RING_BUFFER_SIZE);
    buffer_->Init();

    factory_ = std::make_shared<ClientFactory>(&RxHeaderData, &RxBodyData, this);

    task_ = std::make_shared<OSAL::Task>(std::string("StreamingExecutor"));
    task_->RegisterHandler(std::bind(&StreamingExecutor::HttpDownloadThread, this));

    memset_s(&headerInfo_, sizeof(HeaderInfo), 0x00, sizeof(HeaderInfo));
    headerInfo_.fileContentLen = 0;
    startPos_ = 0;
    isDownloading_ = false;
}

StreamingExecutor::~StreamingExecutor() {}

bool StreamingExecutor::Open(const std::string &url)
{
    MEDIA_LOG_D("Open in");
    FALSE_RETURN_V(!url.empty(), false);

    client_ = factory_->CreateClient(url);
    FALSE_RETURN_V(client_ != nullptr, false);

    client_->Open(url);

    requestSize_ = PER_REQUEST_SIZE;
    startPos_ = 0;
    isEos_ = false;
    isHeaderUpdated = false;
    task_->Start();
    return true;
}


void StreamingExecutor::Close()
{
    task_->Stop();
    startPos_ = 0;
    if (client_ != nullptr) {
        client_->Close();
        client_ = nullptr;
    }
}

bool StreamingExecutor::Read(unsigned char *buff, unsigned int wantReadLength,
                             unsigned int &realReadLength, bool &isEos)
{
    FALSE_RETURN_V(buffer_ != nullptr, false);
    isEos = false;
    realReadLength = buffer_->ReadBuffer(buff, wantReadLength, 2); // wait 2 times
    if (isEos_ && realReadLength == 0) {
        isEos = true;
    }
    MEDIA_LOG_D("Read: wantReadLength " PUBLIC_LOG_D32 ", realReadLength " PUBLIC_LOG_D32 ", isEos "
                PUBLIC_LOG_D32, wantReadLength, realReadLength, isEos);
    return true;
}

bool StreamingExecutor::Seek(int offset)
{
    FALSE_RETURN_V(buffer_ != nullptr, false);
    MEDIA_LOG_I("Seek: buffer size " PUBLIC_LOG_D32 ", offset " PUBLIC_LOG_D32, buffer_->GetSize(), offset);
    if (buffer_->Seek(offset)) {
        return true;
    }
    buffer_->Clear(); // First clear buffer, avoid no available buffer then task pause never exit.
    task_->Pause();
    buffer_->Clear();
    startPos_ = offset;
    int64_t temp = headerInfo_.fileContentLen - offset;
    temp = temp >= 0 ? temp : PER_REQUEST_SIZE;
    requestSize_ = static_cast<int>(std::min(temp, static_cast<int64_t>(PER_REQUEST_SIZE)));
    task_->Start();
    isEos_ = false;
    return true;
}

size_t StreamingExecutor::GetContentLength() const
{
    WaitHeaderUpdated();
    FALSE_RETURN_V_MSG_E(headerInfo_.fileContentLen > 0, 0, "Could not get content length.");
    return headerInfo_.fileContentLen;
}

bool StreamingExecutor::IsStreaming() const
{
    WaitHeaderUpdated();
    return headerInfo_.isChunked;
}

void StreamingExecutor::SetCallback(Callback* cb)
{
    callback_ = cb;
}

void StreamingExecutor::WaitHeaderUpdated() const
{
    int times = 0;
    while (!isHeaderUpdated && times < RETRY_TIMES) { // Wait Header(fileContentLen etc.) updated
        OSAL::SleepFor(SLEEP_TIME);
        times++;
    }
    MEDIA_LOG_D("isHeaderUpdated " PUBLIC_LOG_D32 ", times " PUBLIC_LOG_D32,
                isHeaderUpdated, times);
}

void StreamingExecutor::HttpDownloadThread()
{
    NetworkClientErrorCode clientCode;
    NetworkServerErrorCode serverCode;
    Status ret = client_->RequestData(startPos_, requestSize_, serverCode, clientCode);

    if (ret == Status::ERROR_CLIENT) {
        MEDIA_LOG_I("Send http client error, code " PUBLIC_LOG_D32, clientCode);
        callback_->OnEvent({PluginEventType::CLIENT_ERROR, {clientCode}, "http"});
    } else if (ret == Status::ERROR_SERVER) {
        MEDIA_LOG_I("Send http server error, code " PUBLIC_LOG_D32, serverCode);
        callback_->OnEvent({PluginEventType::SERVER_ERROR, {serverCode}, "http"});
    }
    FALSE_LOG(ret == Status::OK);

    int64_t remaining = headerInfo_.fileContentLen - startPos_;
    if (headerInfo_.fileContentLen > 0 && remaining <= 0) { // 检查是否播放结束
        MEDIA_LOG_I("http transfer reach end, startPos_ " PUBLIC_LOG_D64, startPos_);
        isEos_ = true;
        task_->PauseAsync();
        requestSize_ = PER_REQUEST_SIZE;
        return;
    }
    if(remaining < PER_REQUEST_SIZE){
        requestSize_ = remaining;
    }
}

size_t StreamingExecutor::RxBodyData(void *buffer, size_t size, size_t nitems, void *userParam)
{
    auto executor = static_cast<StreamingExecutor *>(userParam);
    HeaderInfo *header = &(executor->headerInfo_);
    size_t dataLen = size * nitems;

    if (header->fileContentLen == 0) {
        if (header->contentLen > 0) {
            MEDIA_LOG_W("Unsupported range, use content length as content file length");
            header->fileContentLen = header->contentLen;
        } else {
            MEDIA_LOG_E("fileContentLen and contentLen are both zero.");
            return 0;
        }
    }
    if (!executor->isDownloading_) {
        executor->isDownloading_ = true;
    }
    executor->buffer_->WriteBuffer(buffer, dataLen, executor->startPos_);
    executor->isDownloading_ = false;
    MEDIA_LOG_I("RxBodyData: dataLen " PUBLIC_LOG_ZU ", startPos_ " PUBLIC_LOG_D64 ", buffer size "
                PUBLIC_LOG_ZU, dataLen, executor->startPos_, executor->buffer_->GetSize());
    executor->startPos_ = executor->startPos_ + dataLen;

    int bufferSize = executor->buffer_->GetSize();
    double ratio = (static_cast<double>(bufferSize)) / RING_BUFFER_SIZE;
    if (bufferSize >= WATER_LINE && !executor->aboveWaterline_) {
        executor->aboveWaterline_ = true;
        MEDIA_LOG_I("Send http aboveWaterline event, ringbuffer ratio " PUBLIC_LOG_F, ratio);
        executor->callback_->OnEvent({PluginEventType::ABOVE_LOW_WATERLINE, {ratio}, "http"});
    } else if (bufferSize < WATER_LINE && executor->aboveWaterline_) {
        executor->aboveWaterline_ = false;
        MEDIA_LOG_I("Send http belowWaterline event, ringbuffer ratio " PUBLIC_LOG_F, ratio);
        executor->callback_->OnEvent({PluginEventType::BELOW_LOW_WATERLINE, {ratio}, "http"});
    }

    return dataLen;
}

namespace {
char *StringTrim(char *str)
{
    if (str == nullptr) {
        return nullptr;
    }
    char *p = str;
    char *p1 = p + strlen(str) - 1;

    while (*p && isspace(static_cast<int>(*p))) {
        p++;
    }
    while (p1 > p && isspace(static_cast<int>(*p1))) {
        *p1-- = 0;
    }
    return p;
}
}

size_t StreamingExecutor::RxHeaderData(void *buffer, size_t size, size_t nitems, void *userParam)
{
    auto executor = reinterpret_cast<StreamingExecutor *>(userParam);
    HeaderInfo *info = &(executor->headerInfo_);
    char *key = strtok(reinterpret_cast<char *>(buffer), ":");
    FALSE_RETURN_V(key != nullptr, size * nitems);
    if (!strncmp(key, "Content-Type", strlen("Content-Type"))) {
        char *token = strtok(nullptr, ":");
        FALSE_RETURN_V(token != nullptr, size * nitems);
        char *type = StringTrim(token);
        memcpy_s(info->contentType, sizeof(info->contentType), type, sizeof(info->contentType));
    }

    if (!strncmp(key, "Content-Length", strlen("Content-Length")) ||
        !strncmp(key, "content-length", strlen("content-length"))) {
        char *token = strtok(nullptr, ":");
        FALSE_RETURN_V(token != nullptr, size * nitems);
        char *contLen = StringTrim(token);
        info->contentLen = atol(contLen);
    }

    if (!strncmp(key, "Transfer-Encoding", strlen("Transfer-Encoding")) ||
        !strncmp(key, "transfer-encoding", strlen("transfer-encoding"))) {
        char *token = strtok(nullptr, ":");
        FALSE_RETURN_V(token != nullptr, size * nitems);
        char *transEncode = StringTrim(token);
        if (!strncmp(transEncode, "chunked", strlen("chunked"))) {
            info->isChunked = true;
        }
    }

    if (!strncmp(key, "Content-Range", strlen("Content-Range")) ||
        !strncmp(key, "content-range", strlen("content-range"))) {
        char *token = strtok(nullptr, ":");
        FALSE_RETURN_V(token != nullptr, size * nitems);
        char *strRange = StringTrim(token);
        long start, end, fileLen;
        FALSE_LOG_MSG(sscanf_s(strRange, "bytes %ld-%ld/%ld", &start, &end, &fileLen) != -1,
                        "sscanf get range failed");
        if (info->fileContentLen > 0 && info->fileContentLen != fileLen) {
            MEDIA_LOG_E("FileContentLen doesn't equal to fileLen");
        }
        if (info->fileContentLen == 0) {
            info->fileContentLen = fileLen;
        }
    }
    executor->isHeaderUpdated = true;
    return size * nitems;
}
}
}
}
}