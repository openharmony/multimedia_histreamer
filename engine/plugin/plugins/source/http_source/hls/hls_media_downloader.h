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

#include <string>
#include <memory>
#include "plugin/plugins/source/http_source/download/client_factory.h"
#include "plugin/plugins/source/http_source/download/downloader.h"
#include "plugin/plugins/source/http_source/media_downloader.h"
#include "ring_buffer.h"
#include "plugin/plugins/source/http_source/download/network_client.h"
#include "osal/thread/task.h"
#include "plugin/interface/plugin_base.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace HttpPlugin {

class HlsMediaDownloader : public MediaDownloader {
public:
    HlsMediaDownloader() noexcept {}
    ~HlsMediaDownloader() override {}
    bool Open(const std::string &url) override { return false; }
    void Close() override {}
    bool Read(unsigned char *buff, unsigned int wantReadLength, unsigned int &realReadLength, bool &isEos) override
    {
        return false;
    }
    bool Seek(int offset) override { return false; }

    size_t GetContentLength() const override { return 0; }
    bool IsStreaming() const override { return false;}
    void SetCallback(Callback* cb) override {}

private:
    std::shared_ptr<RingBuffer> buffer_;
    std::shared_ptr<Downloader> downloader;
    std::shared_ptr<DownloadRequest> request_;
    bool isEos_ {false}; // file download finished
    HeaderInfo headerInfo_;
    bool isHeaderUpdated {false};
    Callback* callback_ {nullptr};
    bool aboveWaterline_ {false};
};
}
}
}
}
#endif