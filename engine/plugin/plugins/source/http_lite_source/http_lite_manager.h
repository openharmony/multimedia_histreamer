/*
 * Copyright (c) 2021-2021 Huawei Device Co., Ltd.
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

#ifndef HISTREAMER_HTTP_LITE_PLUGIN_HTTP_MANAGER_H
#define HISTREAMER_HTTP_LITE_PLUGIN_HTTP_MANAGER_H

#include <cstdio>
#include <string>
#include <memory>

using OnError = void(*)(int httpError, int localError, void *param, int supportRetry);
constexpr unsigned int DEFAULT_SOURCE_SIZE = 20 * 1024;
constexpr unsigned int DEFAULT_PRIORITY = 32;

struct HttpLiteAttr {
    std::string certFile;
    OnError callbackFunc;
    void *pluginHandle;
    int bufferSize;
    int priority;
};

enum HttpLiteStatus {
    HTTP_STATUS_IDLE = 0,
    HTTP_STATUS_PLAY,
    HTTP_STATUS_PAUSE,
    HTTP_STATUS_SEEK,
    HTTP_STATUS_END,
    HTTP_STATUS_STOP
};

struct HttpLiteRunningInfo {
    unsigned int readPos;
    unsigned int writePos;
    unsigned int lastReadTime;
    HttpLiteStatus state;
    bool isRetry;
};

enum HttpLiteUrlType {
    URL_HTTP,
    URL_HLS,
    URL_WEBSOCKET,
    URL_UNKNOWN
};

class HttpLiteManager {
public:
    HttpLiteManager() noexcept;
    virtual ~HttpLiteManager();
    bool HttpOpen(std::string &url, HttpLiteAttr &attr);
    void HttpClose();
    bool HttpRead(unsigned char *buff, unsigned int wantReadLength, unsigned int &realReadLength, bool &flag);
    bool HttpPeek(unsigned char *buff, unsigned int wantReadLength, unsigned int &realReadLength);
    bool HttpSeek(int offset);
    bool HttpPause();
    bool HttpReset();
    unsigned int GetContentLength() const;
    HttpLiteStatus GetHttpStatus() const;
    unsigned int GetLastReadTime() const;
    void GetHttpBufferRange(unsigned int *read, unsigned int *write);
    void SetWaterline(int high, int low);
    bool IsStreaming();
    HttpLiteUrlType IsHlsSource(std::string &url);
    void GetHttpRunningInfo(HttpLiteRunningInfo &info);
    void SetHttpRunningInfo(bool isRetry);

private:
    friend void ReceiveData(unsigned char *data, int len, void *priv);
    friend void OnFinished(void *priv);
    friend void OnFailed(int httpError, int localError, void *priv, int supportRetry);
    bool IsNeedRetry(int localError, int supportRetry);
    HttpLiteAttr httpAttr_ {"", nullptr, nullptr, DEFAULT_SOURCE_SIZE, DEFAULT_PRIORITY};
    HttpLiteStatus status_ {HTTP_STATUS_IDLE};
    unsigned int lastReadTime_ {0};
    bool isRetry_ {false};
    int retryTimes_ {0};
};


#endif