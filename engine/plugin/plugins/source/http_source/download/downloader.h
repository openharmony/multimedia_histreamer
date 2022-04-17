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
 
#ifndef HISTREAMER_DOWNLOADER_H
#define HISTREAMER_DOWNLOADER_H

#include <functional>
#include <memory>
#include <string>
#include "client_factory.h"
#include "network_client.h"
#include "osal/thread/task.h"
#include "blocking_queue.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace HttpPlugin {
enum struct DownloadStatus {
    FINISHED,
    SERVER_ERROR,
    CLIENT_ERROR
};

struct HeaderInfo {
    char contentType[32]; // 32 chars
    size_t fileContentLen;
    long contentLen;
    bool isChunked {false};

    void Update(const HeaderInfo* info)
    {
        (void)memcpy_s(contentType, sizeof(contentType), info->contentType, sizeof(contentType));
        fileContentLen = info->fileContentLen;
        contentLen = info->contentLen;
        isChunked = info->isChunked;
    }

    size_t GetFileContentLength() const {
        if (fileContentLen > 0) {
            return fileContentLen;
        }
        return contentLen > 0 ? contentLen : 0;
    }
};

using HeaderSaveFunc = std::function<void(const HeaderInfo*)>;

// uint8_t* : the data should save
// uint32_t : length
using DataSaveFunc = std::function<void(uint8_t*, uint32_t, int64_t)>;

using StatusCallbackFunc = std::function<void(DownloadStatus, int32_t)>;

class DownloadRequest {
public:
    DownloadRequest(const std::string& url, HeaderSaveFunc saveHeader, DataSaveFunc saveData,
                    StatusCallbackFunc statusCallback) : url_(url),
                    saveHeader_(std::move(saveHeader)), saveData_(std::move(saveData)),
                    statusCallback_(std::move(statusCallback)) {}

    size_t GetFileContentLength() const
    {
        return headerInfo_.GetFileContentLength();
    }
private:
    std::string url_;
    HeaderSaveFunc saveHeader_;
    DataSaveFunc saveData_;
    StatusCallbackFunc statusCallback_;

    HeaderInfo headerInfo_;
    bool isEos_ {false}; // file download finished
    int64_t startPos_;
    bool isDownloading_;
    int requestSize_;

    friend class Downloader;
};

class Downloader {
public:
    Downloader() noexcept;
    ~Downloader() = default;

    bool Download(const std::shared_ptr<DownloadRequest>& request, int32_t waitMs);
    void Start();
    void Pause();
    void Stop();
    bool Seek(int64_t offset);

private:
    bool BeginDownload();
    void EndDownload();

    void HttpDownloadLoop();
    static size_t RxBodyData(void *buffer, size_t size, size_t nitems, void *userParam);
    static size_t RxHeaderData(void *buffer, size_t size, size_t nitems, void *userParam);

    std::shared_ptr<ClientFactory> factory_;
    std::shared_ptr<NetworkClient> client_;
    std::shared_ptr<OSAL::Task> task_;
    std::shared_ptr<BlockingQueue<std::shared_ptr<DownloadRequest>>> requestQue_;

    std::shared_ptr<DownloadRequest> currentRequest_;
    bool shouldStartNextRequest;
};
}
}
}
}
#endif