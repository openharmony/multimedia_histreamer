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
 
#ifndef HISTREAMER_HLS_MEDIA_DOWNLOADER_H
#define HISTREAMER_HLS_MEDIA_DOWNLOADER_H

#include "adaptive_streaming.h"
#include "ring_buffer.h"
#include "plugin/plugins/source/http_source/media_downloader.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace HttpPlugin {
struct FragmentStatus {
    std::string url_;
    bool isDownloading {false};
    size_t len_ {0};
    bool isEos_ {false};
    int32_t error1_ {0};
    int32_t error2_ {0};
};

class HlsMediaDownloader : public MediaDownloader {
public:
    HlsMediaDownloader() noexcept;
    ~HlsMediaDownloader() override = default;
    bool Open(const std::string& url) override;
    void Close() override;
    bool Read(unsigned char* buff, unsigned int wantReadLength, unsigned int& realReadLength, bool& isEos) override;
    bool Seek(int offset) override;

    size_t GetContentLength() const override;
    bool IsStreaming() const override;
    void SetCallback(Callback* cb) override;
private:
    void SaveData(uint8_t* data, uint32_t len, int64_t offset);
    void OnDownloadStatus(DownloadStatus status, int32_t code);
    void SaveRequestCallback(std::shared_ptr<RequestCallback> r);

    void PlaylistUpdatesLoop();
    void FragmentDownloadLoop();

private:
    size_t fragmentCounter_ {1};
    std::shared_ptr<RingBuffer> buffer_;
    std::shared_ptr<Downloader> downloader;
    std::shared_ptr<DownloadRequest> request_;

    bool isEos_ {false};
    Callback* callback_ {nullptr};
    DataSaveFunc dataSave_;
    StatusCallbackFunc statusCallback_;
    RequestCallbackFunc requestCallbackFunc_;

    std::shared_ptr<AdaptiveStreaming> adaptiveStreaming_;

    std::shared_ptr<OSAL::Task> updateTask_;
    std::shared_ptr<OSAL::Task> downloadTask_;

    std::shared_ptr<BlockingQueue<std::string>> downloadList_;

    std::map<std::string, FragmentStatus> fragmentStatus_;
    std::map<size_t, std::string> fragmentList_;
};
}
}
}
}
#endif