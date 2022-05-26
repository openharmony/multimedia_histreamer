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
AdaptiveStreaming::AdaptiveStreaming()
{
    downloader = std::make_shared<Downloader>();
    dataSave_ = [this] (uint8_t*&& data, uint32_t&& len, int64_t&& offset) {
        SaveData(std::forward<decltype(data)>(data), std::forward<decltype(len)>(len),
                 std::forward<decltype(offset)>(offset));
    };
    statusCallback_ = [this] (DownloadStatus&& status, std::shared_ptr<DownloadRequest>& request, int32_t code) {
        OnDownloadStatus(std::forward<decltype(status)>(status),
            std::forward<decltype(request)>(request), std::forward<decltype(code)>(code));
    };
    updateTask_ = std::make_shared<OSAL::Task>(std::string("FragmentListUpdate"));
    updateTask_->RegisterHandler([this] { FragmentListUpdateLoop(); });
}

AdaptiveStreaming::~AdaptiveStreaming()
{
    downloader->Stop();
    updateTask_->Stop();
}

void AdaptiveStreaming::Open(const std::string& url)
{
    playList_.clear();
    downloadRequest_ = std::make_shared<DownloadRequest>(url, dataSave_, statusCallback_);
    downloader->Download(downloadRequest_, -1); // -1
    downloader->Start();
    while (!downloadRequest_->IsEos()) {
        OSAL::SleepFor(200); // 200 time to download playlist, size is not more than 5*1024 usually
    }
}

void AdaptiveStreaming::SaveData(uint8_t* data, uint32_t len, int64_t offset)
{
    (void)offset;
    playList_.append(reinterpret_cast<const char*>(data), len);
}

void AdaptiveStreaming::OnDownloadStatus(DownloadStatus status, std::shared_ptr<DownloadRequest>& request,
                                         int32_t code)
{
    MEDIA_LOG_I("OnDownloadStatus " PUBLIC_LOG_D32, status);
    switch (status) {
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