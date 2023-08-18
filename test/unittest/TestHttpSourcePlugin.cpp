/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#include <string>
#include "gtest/gtest.h"
#include "plugin/plugins/source/http_source/download/downloader.h"

namespace OHOS {
namespace Media {
namespace Test {
using namespace OHOS::Media::Plugin::HttpPlugin;
using namespace testing::ext;

HWTEST(HttpSourcePluginTest, test_download_request_save_header, TestSize.Level1)
{
    std::shared_ptr<HeaderInfo> headerInfo = std::make_shared<HeaderInfo>();

    const size_t fileContentLen = 20;
    headerInfo->fileContentLen = fileContentLen;

    const char contentType[] = "audio";
    strcpy_s(headerInfo->contentType, sizeof(headerInfo->contentType), contentType);

    headerInfo->isChunked = true;
    headerInfo->isClosed = true;

    DataSaveFunc dataSaveFunc = [](uint8_t*, uint32_t) {
        return false;
    };

    StatusCallbackFunc statusCallbackFunc = [](DownloadStatus, std::shared_ptr<Downloader>&,
        std::shared_ptr<DownloadRequest>&) {};

    const std::string url = "http://test";
    DownloadRequest downloadRequest{url, dataSaveFunc, statusCallbackFunc};
    downloadRequest.SaveHeader(headerInfo.get());

    EXPECT_EQ(fileContentLen, downloadRequest.GetFileContentLength());

    EXPECT_EQ(url, downloadRequest.GetUrl());

    EXPECT_EQ(true, downloadRequest.IsChunked());

    EXPECT_EQ(false, downloadRequest.IsClosed());

    downloadRequest.Close();

    EXPECT_EQ(true, downloadRequest.IsClosed());
}
} // namespace Test
} // namespace Media
} // namespace OHOS
