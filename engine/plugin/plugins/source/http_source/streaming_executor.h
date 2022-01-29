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
 
#ifndef HISTREAMER_STREAMING_EXECUTOR_H
#define HISTREAMER_STREAMING_EXECUTOR_H

#include <string>
#include <memory>
#include "ring_buffer.h"
#include "network_client.h"
#include "osal/thread/task.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace HttpPlugin {

struct HeaderInfo {
    char contentType[32];
    long fileContentLen;
    long contentLen;
    bool isChunked {false};
};

enum UrlType {
    URL_HTTP,
    URL_HLS,
    URL_WEBSOCKET,
    URL_UNKNOWN
};

class StreamingExecutor {
public:
    StreamingExecutor() noexcept;
    virtual ~StreamingExecutor();
    bool Open(const std::string &url);
    void Close();
    bool Read(unsigned char *buff, unsigned int wantReadLength, unsigned int &realReadLength, bool &isEos);
    bool Seek(int offset);

    unsigned int GetContentLength() const;
    bool IsStreaming();

private:
    UrlType GetUrlType(const std::string &url);
    void HttpDownloadThread();
    static size_t RxBodyData(void *buffer, size_t size, size_t nitems, void *userParam);
    static size_t RxHeaderData(void *buffer, size_t size, size_t nitems, void *userParam);

private:
    std::shared_ptr<RingBuffer> buffer_;
    std::shared_ptr<NetworkClient> client_;
    bool isEos_ {false}; // file download finished
    int64_t startPos_;
    HeaderInfo headerInfo_;
    std::shared_ptr<OSAL::Task> task_;
    bool isDownloading;
};
}
}
}
}
#endif