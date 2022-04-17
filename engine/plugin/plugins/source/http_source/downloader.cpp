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
#define HST_LOG_TAG "Downloader"

#include "downloader.h"
#include <algorithm>
#include <plugin/plugins/source/http_source/downloader.h>

#include "securec.h"
#include "osal/utils/util.h"
#include "steady_clock.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace HttpPlugin {
namespace {
constexpr int PER_REQUEST_SIZE = 48 * 1024;
}

Downloader::Downloader() noexcept
{
    shouldStartNextRequest = true;

    factory_ = std::make_shared<ClientFactory>(&RxHeaderData, &RxBodyData, this);
    requestQue_ = std::make_shared<BlockingQueue<std::shared_ptr<DownloadRequest>>>("downloadRequestQue", 10);

    task_ = std::make_shared<OSAL::Task>(std::string("HttpDownloader"));
    task_->RegisterHandler(std::bind(&Downloader::HttpDownloadLoop, this));
}

bool Downloader::Download(const std::shared_ptr<DownloadRequest>& request, int32_t waitMs)
{
    if (waitMs == -1) { // wait until push success
        requestQue_->Push(request);
        return true;
    }
    return requestQue_->Push(request, waitMs);
}

void Downloader::Start()
{
    task_->Start();
}

void Downloader::Pause()
{
    task_->Stop();
}

void Downloader::Stop()
{
    task_->Stop();
}

bool Downloader::Open()
{
    MEDIA_LOG_D("Open in");
    std::string url = currentRequest_->url_;
    FALSE_RETURN_V(!url.empty(), false);

    client_ = factory_->CreateClient(url);
    FALSE_RETURN_V(client_ != nullptr, false);

    client_->Open(url);

    currentRequest_->requestSize_ = PER_REQUEST_SIZE;
    currentRequest_->startPos_ = 0;
    currentRequest_->isEos_ = false;

    task_->Start();
    return true;
}


void Downloader::Close()
{
    if (client_ != nullptr) {
        client_->Close();
        client_ = nullptr;
    }
}

void Downloader::HttpDownloadLoop()
{
    if (shouldStartNextRequest) {
        currentRequest_ = requestQue_->Pop(); //1000); // 1000
        Open();
        shouldStartNextRequest = false;
    }
    FALSE_RETURN_W(currentRequest_ != nullptr);

    NetworkClientErrorCode clientCode;
    NetworkServerErrorCode serverCode;
    Status ret = client_->RequestData(currentRequest_->startPos_, currentRequest_->requestSize_,
                                      serverCode, clientCode);
    if (ret == Status::ERROR_CLIENT) {
        MEDIA_LOG_I("Send http client error, code " PUBLIC_LOG_D32, clientCode);
        currentRequest_->statusCallback_(DownloadStatus::CLIENT_ERROR, static_cast<int32_t>(clientCode));
    } else if (ret == Status::ERROR_SERVER) {
        MEDIA_LOG_I("Send http server error, code " PUBLIC_LOG_D32, serverCode);
        currentRequest_->statusCallback_(DownloadStatus::SERVER_ERROR, static_cast<int32_t>(serverCode));
    }
    FALSE_LOG(ret == Status::OK);

    int64_t remaining = currentRequest_->headerInfo_.fileContentLen - currentRequest_->startPos_;
    if (currentRequest_->headerInfo_.fileContentLen > 0 && remaining <= 0) { // 检查是否播放结束
        MEDIA_LOG_I("http transfer reach end, startPos_ " PUBLIC_LOG_D64, currentRequest_->startPos_);
        task_->PauseAsync();
        Close();
        shouldStartNextRequest = true;
    } else if(remaining < PER_REQUEST_SIZE){
        currentRequest_->requestSize_ = remaining;
    }
}

size_t Downloader::RxBodyData(void *buffer, size_t size, size_t nitems, void *userParam)
{
    auto mediaDownloader = static_cast<Downloader *>(userParam);
    HeaderInfo *header = &(mediaDownloader->currentRequest_->headerInfo_);
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
    if (!mediaDownloader->currentRequest_->isDownloading_) {
        mediaDownloader->currentRequest_->isDownloading_ = true;
    }
    mediaDownloader->currentRequest_->saveData_(static_cast<uint8_t*>(buffer), dataLen,
                                                mediaDownloader->currentRequest_->startPos_);
    mediaDownloader->currentRequest_->isDownloading_ = false;
    MEDIA_LOG_I("RxBodyData: dataLen " PUBLIC_LOG_ZU ", startPos_ " PUBLIC_LOG_D64, dataLen,
                mediaDownloader->currentRequest_->startPos_);
    mediaDownloader->currentRequest_->startPos_ = mediaDownloader->currentRequest_->startPos_ + dataLen;

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

size_t Downloader::RxHeaderData(void *buffer, size_t size, size_t nitems, void *userParam)
{
    auto mediaDownloader = reinterpret_cast<Downloader *>(userParam);
    HeaderInfo *info = &(mediaDownloader->currentRequest_->headerInfo_);
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
    mediaDownloader->currentRequest_->saveHeader_(info);
    return size * nitems;
}
}
}
}
}