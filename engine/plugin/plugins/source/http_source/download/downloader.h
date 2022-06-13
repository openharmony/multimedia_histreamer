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
#include "osal/utils/util.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace HttpPlugin {
enum struct DownloadStatus {
    PARTTAL_DOWNLOAD,
    SERVER_ERROR,
    CLIENT_ERROR
};

struct HeaderInfo {
    char contentType[32]; // 32 chars
    size_t fileContentLen {0};
    long contentLen {0};
    bool isChunked {false};

    void Update(const HeaderInfo* info)
    {
        (void)memcpy_s(contentType, sizeof(contentType), info->contentType, sizeof(contentType));
        fileContentLen = info->fileContentLen;
        contentLen = info->contentLen;
        isChunked = info->isChunked;
    }

    size_t GetFileContentLength() const
    {
        while (fileContentLen == 0 && !isChunked) {
            OSAL::SleepFor(10); // 10, wait for fileContentLen updated
        }
        return fileContentLen;
    }
};

struct RequestInfo {
    char url[2048];
    int64_t fileContentLen;
    int64_t downloadPos;
    int retryTimes;
};
// uint8_t* : the data should save
// uint32_t : length
using DataSaveFunc = std::function<void(uint8_t*, uint32_t, int64_t)>;
class DownloadRequest;
using StatusCallbackFunc = std::function<void(DownloadStatus, std::shared_ptr<DownloadRequest>&, int32_t)>;

class DownloadRequest {
public:
    DownloadRequest(const std::string& url, DataSaveFunc saveData, StatusCallbackFunc statusCallback,
                    bool requestWholeFile = false);
    size_t GetFileContentLength() const;
    void SaveHeader(const HeaderInfo* header);
    bool IsChunked() const;
    bool IsEos() const;
    bool IsValidRequestFor(const std::string& url);
    int64_t GetDownloadPos();

private:
    void WaitHeaderUpdated() const;

    std::string url_;
    DataSaveFunc saveData_;
    StatusCallbackFunc statusCallback_;

    HeaderInfo headerInfo_;

    bool isHeaderUpdated {false};
    bool isEos_ {false}; // file download finished
    int64_t startPos_;
    bool isDownloading_;
    bool requestWholeFile_ {false};
    int requestSize_;
    int retryTimes_ {0};
    NetworkClientErrorCode clientError_ {NetworkClientErrorCode::ERROR_OK};
    NetworkServerErrorCode serverError_ {0};

    friend class Downloader;
};

class Downloader {
public:
    Downloader() noexcept;
    ~Downloader() = default;

    bool Download(const std::shared_ptr<DownloadRequest>& request, int32_t waitMs);
    void Start();
    void Pause();
    void Resume();
    void Stop();
    bool Seek(int64_t offset);
    bool Retry(std::string& url, int64_t offset);
private:
    bool BeginDownload();
    void EndDownload();

    void HttpDownloadLoop();
    static size_t RxBodyData(void* buffer, size_t size, size_t nitems, void* userParam);
    static size_t RxHeaderData(void* buffer, size_t size, size_t nitems, void* userParam);

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