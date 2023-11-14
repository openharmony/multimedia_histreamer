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

#ifndef HISTREAMER_PLAYLIST_DOWNLOADER_H
#define HISTREAMER_PLAYLIST_DOWNLOADER_H

#include <vector>
#include "plugin/plugins/source/http_source/download/downloader.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace HttpPlugin {
struct PlayInfo {
    std::string url_;
    double duration_;
};
struct PlayListChangeCallback {
    virtual ~PlayListChangeCallback() = default;
    virtual void OnPlayListChanged(const std::vector<PlayInfo>& playList) = 0;
};
class PlayListDownloader {
public:
    PlayListDownloader();
    virtual ~PlayListDownloader();

    virtual void Open(const std::string& url) = 0;
    virtual void UpdateManifest() = 0;
    virtual void ParseManifest() = 0;
    virtual void PlayListUpdateLoop() = 0;
    virtual void SetPlayListCallback(PlayListChangeCallback* callback) = 0;
    virtual int64_t GetDuration() const = 0;
    virtual Seekable GetSeekable() const = 0;
    virtual void SelectBitRate(uint32_t bitRate) = 0;
    virtual std::vector<uint32_t> GetBitRates() = 0;
    virtual bool IsBitrateSame(uint32_t bitRate) = 0;
    void Resume();
    void Pause();
    void Close();
    void Stop();
    void Start();
    void SetStatusCallback(StatusCallbackFunc cb);
    bool GetPlayListDownloadStatus();

protected:
    bool SaveData(uint8_t* data, uint32_t len);
    void OnDownloadStatus(DownloadStatus status, std::shared_ptr<Downloader>&,
                          std::shared_ptr<DownloadRequest>& request);
    void DoOpen(const std::string& url);

protected:
    std::shared_ptr<Downloader> downloader_;
    std::shared_ptr<DownloadRequest> downloadRequest_;
    std::shared_ptr<OSAL::Task> updateTask_;
    DataSaveFunc dataSave_;
    StatusCallbackFunc statusCallback_;
    std::string playList_;
    bool startedDownloadStatus_ {false};
};
}
}
}
}
#endif