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
#include <functional>
#include "securec.h"
#include "foundation/log.h"
#include "http_curl_client.h"
#include "osal/utils/util.h"
#include "steady_clock.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace HttpPlugin {
constexpr int RING_BUFFER_SIZE = 5 * 48 * 1024;
constexpr int PER_REQUEST_SIZE = 48 * 1024;

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
    isDownloading = false;
}

StreamingExecutor::~StreamingExecutor() {}

bool StreamingExecutor::Open(const std::string &url)
{
    MEDIA_LOG_D("Open in");
    FALSE_RETURN_V(!url.empty(), false);

    client_ = factory_->CreateClient(url);
    FALSE_RETURN_V(client_ != nullptr, false);

    client_->Open(url);

    startPos_ = 0;
    isEos_ = false;
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

bool StreamingExecutor::Read(unsigned char *buff, unsigned int wantReadLength, unsigned int &realReadLength, bool &isEos)
{
    FALSE_RETURN_V(buffer_ != nullptr, false);
    isEos = false;
    realReadLength = buffer_->ReadBuffer(buff, wantReadLength);
    if (isEos_ && realReadLength == 0) {
        isEos = true;
    }
    MEDIA_LOG_D("Read: wantReadLength %" PUBLIC_OUTPUT "d, realReadLength %" PUBLIC_OUTPUT "d, isEos %"
                PUBLIC_OUTPUT "d", wantReadLength, realReadLength, isEos);
    return true;
}

bool StreamingExecutor::Seek(int offset)
{
    FALSE_RETURN_V(buffer_ != nullptr, false);
    MEDIA_LOG_I("Seek: buffer size %" PUBLIC_OUTPUT "d, offset %" PUBLIC_OUTPUT "d", buffer_->GetSize(), offset);
    if (isEos_) {
        startPos_ = offset;
        task_->Start();
        isEos_ = false;
    }
    return true;
}

unsigned int StreamingExecutor::GetContentLength() const
{
    return headerInfo_.fileContentLen;
}

bool StreamingExecutor::IsStreaming()
{
    return headerInfo_.isChunked;
}

void StreamingExecutor::HttpDownloadThread()
{
    int ret = client_->RequestData(startPos_, PER_REQUEST_SIZE);
    FALSE_LOG(ret == 0);
    if (headerInfo_.fileContentLen > 0 && startPos_ >= headerInfo_.fileContentLen) { // 检查是否播放结束
        MEDIA_LOG_I("http download completed, startPos_ %" PUBLIC_OUTPUT "d", startPos_);
        isEos_ = true;
        task_->PauseAsync();
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
    if (!executor->isDownloading) {
        executor->isDownloading = true;
    }
    executor->buffer_->WriteBuffer(buffer, dataLen);
    executor->isDownloading = false;
    executor->startPos_ = executor->startPos_ + dataLen;
    MEDIA_LOG_I("RxBodyData: dataLen %" PUBLIC_OUTPUT "d, startPos_ %" PUBLIC_OUTPUT "d, buffer size %" PUBLIC_OUTPUT "d", dataLen, executor->startPos_, executor->buffer_->GetSize());
    return dataLen;
}

namespace {
char *StringTrim(char *str) {
    if (str == nullptr) {
        return nullptr;
    }
    char *p = str;
    char *p1 = p + strlen(str) - 1;

    while (*p && isspace((int) *p)) {
        p++;
    }
    while (p1 > p && isspace((int) *p1)) {
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
    if (!strncmp(key, "Content-Type", strlen("Content-Type"))) {
        char *type = StringTrim(strtok(NULL, ":"));
        memcpy_s(info->contentType, sizeof(info->contentType), type, sizeof(info->contentType));
    }

    if (!strncmp(key, "Content-Length", strlen("Content-Length")) || !strncmp(key, "content-length", strlen("content-length"))) {
        char *contLen = StringTrim(strtok(NULL, ":"));
        info->contentLen = atol(contLen);
    }

    if (!strncmp(key, "Transfer-Encoding", strlen("Transfer-Encoding")) || !strncmp(key, "transfer-encoding", strlen("transfer-encoding"))) {
        char *transEncode = StringTrim(strtok(NULL, ":"));
        if (!strncmp(transEncode, "chunked", strlen("chunked"))) {
            info->isChunked = true;
        }
    }

    if (!strncmp(key, "Content-Range", strlen("Content-Range")) || !strncmp(key, "content-range", strlen("content-range"))) {
        char *strRange = StringTrim(strtok(NULL, ":"));
        long start, end, fileLen;
        sscanf_s(strRange, "bytes %" PUBLIC_OUTPUT "ld-%" PUBLIC_OUTPUT "ld/%" PUBLIC_OUTPUT "ld", &start, &end, &fileLen);
        if (info->fileContentLen > 0 && info->fileContentLen != fileLen) {
            MEDIA_LOG_E("FileContentLen doesn't equal to fileLen");
        }
        if (info->fileContentLen == 0) {
            info->fileContentLen = fileLen;
        }
    }
    return size * nitems;
}

}
}
}
}