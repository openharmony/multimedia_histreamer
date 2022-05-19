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
#define HST_LOG_TAG "AdaptiveStreaming"
#include "adaptive_streaming.h"
#include "securec.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace HttpPlugin {
AdaptiveStreaming::AdaptiveStreaming(const std::string& url)
    :uri_(url)
{
    playListDownloader_ = std::make_shared<Downloader>();
    playListDataSave_ =  [this] (auto&& data, auto&& len, auto&& offset) {
        SavePlayListData(std::forward<decltype(data)>(data), std::forward<decltype(len)>(len),
                std::forward<decltype(offset)>(offset)); };
    playListStatusCallback_ = [this] (auto&& status, auto&& code) {
        OnDownloadPlayListStatus(std::forward<decltype(status)>(status), std::forward<decltype(code)>(code)); };
}

bool AdaptiveStreaming::GetPlaylist(const std::string& url)
{
    memset_s(playList, PLAY_LIST_SIZE, 0, PLAY_LIST_SIZE);
    playListRequest_ = std::make_shared<DownloadRequest>(url, playListDataSave_, playListStatusCallback_);
    playListDownloader_->Download(playListRequest_, -1); // -1
    playListDownloader_->Start();

    while (!playListRequest_->IsEos()) {
        OSAL::SleepFor(200); // 200 time to download playlist, size is not more than 5*1024 usually
    }

    return playListRequest_->IsEos();
}

void AdaptiveStreaming::SavePlayListData(uint8_t* data, uint32_t len, int64_t offset)
{
    memcpy_s(playList+offset, len, data, len);
}

void AdaptiveStreaming::OnDownloadPlayListStatus(DownloadStatus status, int32_t code)
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