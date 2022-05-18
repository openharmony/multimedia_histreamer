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
#define HST_LOG_TAG "HlsMediaDownloader"

#include "hls_media_downloader.h"
#include "hls_streaming.h"
#include "securec.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace HttpPlugin {
namespace {
constexpr int RING_BUFFER_SIZE = 5 * 48 * 1024;
}

using namespace std::placeholders;

HlsMediaDownloader::HlsMediaDownloader() noexcept
{
    buffer_ = std::make_shared<RingBuffer>(RING_BUFFER_SIZE);
    buffer_->Init();

    downloader = std::make_shared<Downloader>();

    updateTask_ = std::make_shared<OSAL::Task>(std::string("HlsUpdates"));
    updateTask_->RegisterHandler(std::bind(&HlsMediaDownloader::PlaylistUpdatesLoop, this));

    downloadTask_ = std::make_shared<OSAL::Task>(std::string("HlsDownload"));
    downloadTask_->RegisterHandler(std::bind(&HlsMediaDownloader::FragmentDownloadLoop, this));

    downloadList_ = std::make_shared<BlockingQueue<std::string>>("HlsDownloadList", 50); // 50
}

HlsMediaDownloader::~HlsMediaDownloader() {}

void HlsMediaDownloader::PlaylistUpdatesLoop()
{
    OSAL::SleepFor(8000); // 8000 how often is playlist updated
    adaptiveStreaming_->UpdateManifest();
    adaptiveStreaming_->GetDownloadList(downloadList_);
}

void HlsMediaDownloader::FragmentDownloadLoop()
{
    std::string url = downloadList_->Pop();
    if (fragmentStatus_[url].isDownloading == false) {
        fragmentStatus_[url].isDownloading = true;
        request_ = std::make_shared<DownloadRequest>(url,
                                                     std::bind(&HlsMediaDownloader::SaveData, this, _1, _2, _3),
                                                     std::bind(&HlsMediaDownloader::OnDownloadStatus, this, _1, _2));
        request_->SetRequestCallback(std::bind(&HlsMediaDownloader::SaveRequestCallback, this, _1));
        downloader->Download(request_, -1);
        downloader->Start();
        fragmentList_[fragmentCounter_] = url;
        fragmentCounter_++;
    }
}

bool HlsMediaDownloader::Open(const std::string &url)
{
    adaptiveStreaming_ = std::make_shared<HLSStreaming>(url);
    adaptiveStreaming_->ProcessManifest();
    adaptiveStreaming_->GetDownloadList(downloadList_);

    updateTask_->Start();
    downloadTask_->Start();
    return true;
}

void HlsMediaDownloader::Close()
{
    buffer_->SetActive(false);
    updateTask_->Stop();
    downloadTask_->Stop();
    downloader->Stop();
}

bool HlsMediaDownloader::Read(unsigned char *buff, unsigned int wantReadLength,
                              unsigned int &realReadLength, bool &isEos)
{
    FALSE_RETURN_V(buffer_ != nullptr, false);
    isEos = false;
    realReadLength = buffer_->ReadBuffer(buff, wantReadLength, 2); // wait 2 times
    MEDIA_LOG_D("Read: wantReadLength " PUBLIC_LOG_D32 ", realReadLength " PUBLIC_LOG_D32 ", isEos "
                PUBLIC_LOG_D32, wantReadLength, realReadLength, isEos);
    return true;
}

bool HlsMediaDownloader::Seek(int offset)
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

size_t HlsMediaDownloader::GetContentLength() const
{
    return 0;
}

bool HlsMediaDownloader::IsStreaming() const
{
    return true;
}

void HlsMediaDownloader::SetCallback(Callback* cb)
{
    callback_ = cb;
}

void HlsMediaDownloader::SaveRequestCallback(std::shared_ptr<RequestCallback> r)
{
    std::string url_ = r->url_;

    fragmentStatus_[url_].len_ = r->len_;
    fragmentStatus_[url_].isEos_ = r->isEos_;
    fragmentStatus_[url_].error1_ = r->error1_;
    fragmentStatus_[url_].error2_ = r->error2_;
}

void HlsMediaDownloader::SaveData(uint8_t* data, uint32_t len, int64_t offset)
{
    buffer_->WriteBuffer(data, len, offset);
}

void HlsMediaDownloader::OnDownloadStatus(DownloadStatus status, int32_t code)
{
    MEDIA_LOG_I("OnDownloadStatus " PUBLIC_LOG_D32, status);
    switch (status) {
        case DownloadStatus::FINISHED:
            break;
        case DownloadStatus::CLIENT_ERROR:
            MEDIA_LOG_I("Send http client error, code " PUBLIC_LOG_D32, code);
            break;
        case DownloadStatus::SERVER_ERROR:
            MEDIA_LOG_I("Send http server error, code " PUBLIC_LOG_D32, code);
            break;
        default:
            MEDIA_LOG_E("Unknown download status.");
    }
}
}
}
}
}