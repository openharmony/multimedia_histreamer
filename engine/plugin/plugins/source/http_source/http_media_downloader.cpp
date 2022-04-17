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
#define HST_LOG_TAG "HttpMediaDownloader"

#include "http_media_downloader.h"
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
constexpr size_t RETRY_TIMES = 200;  // Retry 200 times
}

using namespace std::placeholders;

HttpMediaDownloader::HttpMediaDownloader() noexcept
{
    buffer_ = std::make_shared<RingBuffer>(RING_BUFFER_SIZE);
    buffer_->Init();

    downloader = std::make_shared<Downloader>();

    factory_ = std::make_shared<ClientFactory>(&RxHeaderData, &RxBodyData, this);

    task_ = std::make_shared<OSAL::Task>(std::string("HttpMediaDownloader"));
    task_->RegisterHandler(std::bind(&HttpMediaDownloader::HttpDownloadThread, this));

    (void)memset_s(&headerInfo_, sizeof(HeaderInfo), 0x00, sizeof(HeaderInfo));
    headerInfo_.fileContentLen = 0;
    startPos_ = 0;
    isDownloading_ = false;
}

HttpMediaDownloader::~HttpMediaDownloader() {}

bool HttpMediaDownloader::Open(const std::string &url)
{
    MEDIA_LOG_I("Open download " PUBLIC_LOG_S, url.c_str());
#if 1
    std::shared_ptr<DownloadRequest> request = std::make_shared<DownloadRequest>(url,
        std::bind(&HttpMediaDownloader::SaveHeader, this, _1),
        std::bind(&HttpMediaDownloader::SaveData, this, _1, _2, _3),
        std::bind(&HttpMediaDownloader::OnDownloadStatus, this, _1, _2));
    downloader->Download(request, -1);
    downloader->Start();
#else
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
#endif
    return true;
}


void HttpMediaDownloader::Close()
{
    buffer_->SetActive(false);
    downloader->Stop();
    task_->Stop();
    startPos_ = 0;
    if (client_ != nullptr) {
        client_->Close();
        client_ = nullptr;
    }
}

bool HttpMediaDownloader::Read(unsigned char *buff, unsigned int wantReadLength,
                               unsigned int &realReadLength, bool &isEos)
{
    FALSE_RETURN_V(buffer_ != nullptr, false);
    isEos = false;
    if (isEos_ && buffer_->GetSize() == 0) {
        isEos = true;
        realReadLength = 0;
        return false;
    }
    realReadLength = buffer_->ReadBuffer(buff, wantReadLength, 2); // wait 2 times
    MEDIA_LOG_D("Read: wantReadLength " PUBLIC_LOG_D32 ", realReadLength " PUBLIC_LOG_D32 ", isEos "
                PUBLIC_LOG_D32, wantReadLength, realReadLength, isEos);
    return true;
}

bool HttpMediaDownloader::Seek(int offset)
{
    FALSE_RETURN_V(buffer_ != nullptr, false);
    MEDIA_LOG_I("Seek: buffer size " PUBLIC_LOG_ZU ", offset " PUBLIC_LOG_D32, buffer_->GetSize(), offset);
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

size_t HttpMediaDownloader::GetContentLength() const
{
    WaitHeaderUpdated();
    FALSE_RETURN_V_MSG_E(headerInfo_.fileContentLen > 0, 0, "Could not get content length.");
    return headerInfo_.fileContentLen;
}

bool HttpMediaDownloader::IsStreaming() const
{
    WaitHeaderUpdated();
    return headerInfo_.isChunked;
}

void HttpMediaDownloader::SetCallback(Callback* cb)
{
    callback_ = cb;
}

void HttpMediaDownloader::WaitHeaderUpdated() const
{
    size_t times = 0;
    while (!isHeaderUpdated && times < RETRY_TIMES) { // Wait Header(fileContentLen etc.) updated
        OSAL::SleepFor(SLEEP_TIME);
        times++;
    }
    MEDIA_LOG_D("isHeaderUpdated " PUBLIC_LOG_D32 ", times " PUBLIC_LOG_ZU,
                isHeaderUpdated, times);
}

void HttpMediaDownloader::HttpDownloadThread()
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

size_t HttpMediaDownloader::RxBodyData(void *buffer, size_t size, size_t nitems, void *userParam)
{
    auto mediaDownloader = static_cast<HttpMediaDownloader *>(userParam);
    HeaderInfo *header = &(mediaDownloader->headerInfo_);
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
    if (!mediaDownloader->isDownloading_) {
        mediaDownloader->isDownloading_ = true;
    }
    mediaDownloader->buffer_->WriteBuffer(buffer, dataLen, mediaDownloader->startPos_);
    mediaDownloader->isDownloading_ = false;
    MEDIA_LOG_I("RxBodyData: dataLen " PUBLIC_LOG_ZU ", startPos_ " PUBLIC_LOG_D64 ", buffer size "
                PUBLIC_LOG_ZU, dataLen, mediaDownloader->startPos_, mediaDownloader->buffer_->GetSize());
    mediaDownloader->startPos_ = mediaDownloader->startPos_ + dataLen;

    int bufferSize = mediaDownloader->buffer_->GetSize();
    double ratio = (static_cast<double>(bufferSize)) / RING_BUFFER_SIZE;
    if (bufferSize >= WATER_LINE && !mediaDownloader->aboveWaterline_) {
        mediaDownloader->aboveWaterline_ = true;
        MEDIA_LOG_I("Send http aboveWaterline event, ringbuffer ratio " PUBLIC_LOG_F, ratio);
        mediaDownloader->callback_->OnEvent({PluginEventType::ABOVE_LOW_WATERLINE, {ratio}, "http"});
    } else if (bufferSize < WATER_LINE && mediaDownloader->aboveWaterline_) {
        mediaDownloader->aboveWaterline_ = false;
        MEDIA_LOG_I("Send http belowWaterline event, ringbuffer ratio " PUBLIC_LOG_F, ratio);
        mediaDownloader->callback_->OnEvent({PluginEventType::BELOW_LOW_WATERLINE, {ratio}, "http"});
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

size_t HttpMediaDownloader::RxHeaderData(void *buffer, size_t size, size_t nitems, void *userParam)
{
    auto mediaDownloader = reinterpret_cast<HttpMediaDownloader *>(userParam);
    HeaderInfo *info = &(mediaDownloader->headerInfo_);
    char *next = nullptr;
    char *key = strtok_s(reinterpret_cast<char *>(buffer), ":", &next);
    FALSE_RETURN_V(key != nullptr, size * nitems);
    if (!strncmp(key, "Content-Type", strlen("Content-Type"))) {
        char *token = strtok_s(nullptr, ":", &next);
        FALSE_RETURN_V(token != nullptr, size * nitems);
        char *type = StringTrim(token);
        (void)memcpy_s(info->contentType, sizeof(info->contentType), type, sizeof(info->contentType));
    }

    if (!strncmp(key, "Content-Length", strlen("Content-Length")) ||
        !strncmp(key, "content-length", strlen("content-length"))) {
        char *token = strtok_s(nullptr, ":", &next);
        FALSE_RETURN_V(token != nullptr, size * nitems);
        char *contLen = StringTrim(token);
        info->contentLen = atol(contLen);
    }

    if (!strncmp(key, "Transfer-Encoding", strlen("Transfer-Encoding")) ||
        !strncmp(key, "transfer-encoding", strlen("transfer-encoding"))) {
        char *token = strtok_s(nullptr, ":", &next);
        FALSE_RETURN_V(token != nullptr, size * nitems);
        char *transEncode = StringTrim(token);
        if (!strncmp(transEncode, "chunked", strlen("chunked"))) {
            info->isChunked = true;
        }
    }

    if (!strncmp(key, "Content-Range", strlen("Content-Range")) ||
        !strncmp(key, "content-range", strlen("content-range"))) {
        char *token = strtok_s(nullptr, ":", &next);
        FALSE_RETURN_V(token != nullptr, size * nitems);
        char *strRange = StringTrim(token);
        size_t start, end, fileLen;
        FALSE_LOG_MSG(sscanf_s(strRange, "bytes %ld-%ld/%ld", &start, &end, &fileLen) != -1,
            "sscanf get range failed");
        if (info->fileContentLen > 0 && info->fileContentLen != fileLen) {
            MEDIA_LOG_E("FileContentLen doesn't equal to fileLen");
        }
        if (info->fileContentLen == 0) {
            info->fileContentLen = fileLen;
        }
    }
    mediaDownloader->isHeaderUpdated = true;
    return size * nitems;
}

void HttpMediaDownloader::SaveHeader(const HeaderInfo* header)
{
    headerInfo_.Update(header);
    isHeaderUpdated = true;
}

void HttpMediaDownloader::SaveData(uint8_t* data, uint32_t len, int64_t offset)
{
    buffer_->WriteBuffer(data, len, offset);

    size_t bufferSize = buffer_->GetSize();
    double ratio = (static_cast<double>(bufferSize)) / RING_BUFFER_SIZE;
    if (bufferSize >= WATER_LINE && !aboveWaterline_) {
        aboveWaterline_ = true;
        MEDIA_LOG_I("Send http aboveWaterline event, ringbuffer ratio " PUBLIC_LOG_F, ratio);
        callback_->OnEvent({PluginEventType::ABOVE_LOW_WATERLINE, {ratio}, "http"});
    } else if (bufferSize < WATER_LINE && aboveWaterline_) {
        aboveWaterline_ = false;
        MEDIA_LOG_I("Send http belowWaterline event, ringbuffer ratio " PUBLIC_LOG_F, ratio);
        callback_->OnEvent({PluginEventType::BELOW_LOW_WATERLINE, {ratio}, "http"});
    }
}

void HttpMediaDownloader::OnDownloadStatus(DownloadStatus status, int32_t code)
{
    switch (status) {
        case DownloadStatus::FINISHED:
            isEos_ = true;
            break;
        case DownloadStatus::CLIENT_ERROR:
            MEDIA_LOG_I("Send http client error, code " PUBLIC_LOG_D32, code);
            callback_->OnEvent({PluginEventType::CLIENT_ERROR, {code}, "http"});
            break;
        case DownloadStatus::SERVER_ERROR:
            MEDIA_LOG_I("Send http server error, code " PUBLIC_LOG_D32, code);
            callback_->OnEvent({PluginEventType::SERVER_ERROR, {code}, "http"});
            break;
        default:
            MEDIA_LOG_E("Unknown download status.");

    }
}
}
}
}
}