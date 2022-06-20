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

#ifndef HISTREAMER_ADAPTIVE_STREAMING_H
#define HISTREAMER_ADAPTIVE_STREAMING_H

#include <vector>
#include "plugin/plugins/source/http_source/download/downloader.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace HttpPlugin {
struct FragmentListChangeCallback {
    virtual ~FragmentListChangeCallback() = default;
    virtual void OnFragmentListChanged(const std::vector<std::string>& fragmentList) = 0;
};
class AdaptiveStreaming {
public:
    AdaptiveStreaming();
    virtual ~AdaptiveStreaming();

    virtual void Open(std::string url) = 0;
    virtual void UpdateManifest() = 0;
    virtual void ParseManifest() = 0;
    virtual void FragmentListUpdateLoop() = 0;
    virtual void SetFragmentListCallback(FragmentListChangeCallback* callback) = 0;
    virtual double GetDuration() const = 0;
    virtual Seekable GetSeekable() const = 0;
    void Resume();
    void Pause();
    void Close();
    void SetStatusCallback(StatusCallbackFunc cb);

protected:
    void SaveData(uint8_t* data, uint32_t len, int64_t offset);
    void OnDownloadStatus(DownloadStatus status, std::shared_ptr<Downloader>&,
                          std::shared_ptr<DownloadRequest>& request);
    void DoOpen(const std::string& url);

protected:
    std::shared_ptr<Downloader> downloader;
    std::shared_ptr<DownloadRequest> downloadRequest_;
    std::shared_ptr<OSAL::Task> updateTask_;
    DataSaveFunc dataSave_;
    StatusCallbackFunc statusCallback_;
    std::string playList_;
};
}
}
}
}
#endif