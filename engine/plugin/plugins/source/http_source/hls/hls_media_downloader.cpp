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
#include "hls_playlist_downloader.h"
#include "securec.h"
#include "plugin/common/plugin_time.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace HttpPlugin {
namespace {
constexpr int RING_BUFFER_SIZE = 5 * 48 * 1024;
}

// Description:
//   hls manifest, m3u8 --- content get from m3u8 url, we get play list from the content
//   fragment --- one item in play list, download media data according to the fragment address.
HlsMediaDownloader::HlsMediaDownloader() noexcept
{
    buffer_ = std::make_shared<RingBuffer>(RING_BUFFER_SIZE);
    buffer_->Init();

    downloader_ = std::make_shared<Downloader>("hlsMedia");
    downloadTask_ = std::make_shared<OSAL::Task>(std::string("FragmentDownload"));
    downloadTask_->RegisterHandler([this] { FragmentDownloadLoop(); });

    playList_ = std::make_shared<BlockingQueue<PlayInfo>>("PlayList", 50); // 50

    dataSave_ =  [this] (uint8_t*&& data, uint32_t&& len) {
        return SaveData(std::forward<decltype(data)>(data), std::forward<decltype(len)>(len));
    };

    playListDownloader_ = std::make_shared<HlsPlayListDownloader>();
    playListDownloader_->SetPlayListCallback(this);
}

void HlsMediaDownloader::FragmentDownloadLoop()
{   
    auto playInfo = playList_->Pop();
    std::string url = playInfo.url_;
    if (url.empty()) { // when monitor pause, playList_ set active false, it's empty
        OSAL::SleepFor(10); // 10
        return;
    }
    if (!fragmentDownloadStart[url]) {
        fragmentDownloadStart[url] = true;
        PutRequestIntoDownloader(playInfo);
    }
}

void HlsMediaDownloader::PutRequestIntoDownloader(const PlayInfo& playInfo)
{
    auto realStatusCallback = [this] (DownloadStatus&& status, std::shared_ptr<Downloader>& downloader,
                                        std::shared_ptr<DownloadRequest>& request) {
        statusCallback_(status, downloader_, std::forward<decltype(request)>(request));
    };
    // TO DO: If the fragment file is too large, should not requestWholeFile.
    downloadRequest_ = std::make_shared<DownloadRequest>(playInfo.url_, playInfo.duration_, dataSave_, realStatusCallback, true);
    // push request to back queue for seek
    backPlayList_.push_back(downloadRequest_);
    downloader_->Download(downloadRequest_, -1); // -1
    downloader_->Start();
}

bool HlsMediaDownloader::Open(const std::string& url)
{
    playListDownloader_->Open(url);
    downloadTask_->Start();
    return true;
}

void HlsMediaDownloader::Close(bool isAsync)
{
    buffer_->SetActive(false);
    playList_->SetActive(false);
    downloadTask_->Stop();
    playListDownloader_->Close();
    downloader_->Stop();
}

void HlsMediaDownloader::Pause()
{
    bool cleanData = GetSeekable() != Seekable::SEEKABLE;
    buffer_->SetActive(false, cleanData);
    playList_->SetActive(false, cleanData);
    playListDownloader_->Pause();
    downloadTask_->Pause();
    downloader_->Pause();
}

void HlsMediaDownloader::Resume()
{
    buffer_->SetActive(true);
    playList_->SetActive(true);
    playListDownloader_->Resume();
    downloadTask_->Start();
    downloader_->Resume();
}

bool HlsMediaDownloader::Read(unsigned char* buff, unsigned int wantReadLength,
                              unsigned int& realReadLength, bool& isEos)
{
    FALSE_RETURN_V(buffer_ != nullptr, false);
    realReadLength = buffer_->ReadBuffer(buff, wantReadLength, 2); // wait 2 times
    MEDIA_LOG_D("Read: wantReadLength " PUBLIC_LOG_D32 ", realReadLength " PUBLIC_LOG_D32 ", isEos "
                PUBLIC_LOG_D32, wantReadLength, realReadLength, isEos);
    return true;
}

bool HlsMediaDownloader::SeekToTime(int64_t offset)
{   
    FALSE_RETURN_V(buffer_ != nullptr, false);
    MEDIA_LOG_I("Seek: buffer size " PUBLIC_LOG_ZU ", offset " PUBLIC_LOG_D32, buffer_->GetSize(), offset);
    if (buffer_->Seek(offset)) {
        return true;
    }
    buffer_->Clear(); // First clear buffer, avoid no available buffer then task pause never exit.
    downloader_->Cancle();
    buffer_->Clear();
    downloader_->Start();
    FindSeekRequest(offset);
    MEDIA_LOG_I("SeekToTime end\n");
    return true;
}

size_t HlsMediaDownloader::GetContentLength() const
{
    return 0;
}

int64_t HlsMediaDownloader::GetDuration() const
{   
    MEDIA_LOG_I("GetDuration " PUBLIC_LOG_D64 , playListDownloader_->GetDuration());
    return playListDownloader_->GetDuration();
}

Seekable HlsMediaDownloader::GetSeekable() const
{
    return playListDownloader_->GetSeekable();
}

void HlsMediaDownloader::SetCallback(Callback* cb)
{
    callback_ = cb;
}

void HlsMediaDownloader::OnPlayListChanged(const std::vector<PlayInfo>& playList)
{   
    for (auto& fragment : playList) {
        playList_->Push(fragment);
    }
}

bool HlsMediaDownloader::GetStartedStatus()
{
    return playListDownloader_->GetPlayListDownloadStatus() && startedPlayStatus_;
}

bool HlsMediaDownloader::SaveData(uint8_t* data, uint32_t len)
{
    startedPlayStatus_ = true;
    return buffer_->WriteBuffer(data, len);
}

void HlsMediaDownloader::SetStatusCallback(StatusCallbackFunc cb)
{
    statusCallback_ = cb;
    playListDownloader_->SetStatusCallback(cb);
}

std::vector<uint32_t> HlsMediaDownloader::GetBitRates()
{   
    return playListDownloader_->GetBitRates();
}

bool HlsMediaDownloader::SelectBitRate(uint32_t bitRate)
{   
    if (playListDownloader_->IsBitrateSame(bitRate))
    {
        return 0;
    }
    buffer_->Clear(); // First clear buffer, avoid no available buffer then task pause never exit.
    downloader_->Stop();
    buffer_->Clear();
    playListDownloader_->Stop();
    // clear request queue
    playList_->SetActive(false, true);
    playList_->SetActive(true);
    fragmentDownloadStart.clear();
    backPlayList_.clear();
    // switch to des bitrate
    playListDownloader_->SelectBitRate(bitRate);
    downloader_->Start();
    playListDownloader_->Start();
    playListDownloader_->UpdateManifest();
    return 1;
}

void HlsMediaDownloader::FindSeekRequest(int64_t offset)
{   
    int64_t totalDuration = 0;
    for (const auto &item : backPlayList_) {
        int64_t hstTime;
        Plugin::Sec2HstTime(item->GetDuration(), hstTime);
        totalDuration += Plugin::HstTime2Ns(hstTime);
        if (offset < totalDuration) {   
            PlayInfo playInfo;
            playInfo.url_ = item->GetUrl();
            playInfo.duration_ = item->GetDuration();
            MEDIA_LOG_I("FindSeekRequest  url_" PUBLIC_LOG_S " totalDuration " PUBLIC_LOG_D64, playInfo.url_.c_str(), totalDuration);
            PutRequestIntoDownloader(playInfo);
        } 
    }
}

}
}
}
}