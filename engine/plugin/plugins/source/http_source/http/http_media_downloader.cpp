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

    (void)memset_s(&headerInfo_, sizeof(HeaderInfo), 0x00, sizeof(HeaderInfo));
    headerInfo_.fileContentLen = 0;
}

HttpMediaDownloader::~HttpMediaDownloader() {}

bool HttpMediaDownloader::Open(const std::string &url)
{
    MEDIA_LOG_I("Open download " PUBLIC_LOG_S, url.c_str());
    isEos_ = false;
    isHeaderUpdated = false;
    request_ = std::make_shared<DownloadRequest>(url,
        std::bind(&HttpMediaDownloader::SaveHeader, this, _1),
        std::bind(&HttpMediaDownloader::SaveData, this, _1, _2, _3),
        std::bind(&HttpMediaDownloader::OnDownloadStatus, this, _1, _2));
    downloader->Download(request_, -1);
    downloader->Start();
    return true;
}


void HttpMediaDownloader::Close()
{
    buffer_->SetActive(false);
    downloader->Stop();
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
    downloader->Pause();
    buffer_->Clear();
    downloader->Seek(offset);
    downloader->Start();
    isEos_ = false;
    return true;
}

size_t HttpMediaDownloader::GetContentLength() const
{
    WaitHeaderUpdated();
    size_t length = headerInfo_.GetFileContentLength();
    if (length > 0) {
        return length;
    }
    return request_->GetFileContentLength();
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
    MEDIA_LOG_I("OnDownloadStatus " PUBLIC_LOG_D32, status);
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