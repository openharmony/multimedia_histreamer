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

#ifndef HISTREAMER_HTTPS_CURL_CLIENT_H
#define HISTREAMER_HTTPS_CURL_CLIENT_H

#include <memory>
#include <string>
#include "network_client.h"
#include "curl/curl.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace HttpPlugin {
class HttpsCurlClient : public NetworkClient {
public:
    explicit HttpsCurlClient(std::shared_ptr<NetworkClient> httpClient);

    ~HttpsCurlClient() override;

    int Init() override;

    int Open(const std::string& url) override;

    Status RequestData(long startPos, int len) override;

    int Close() override;

    int Deinit() override;

private:
    std::shared_ptr<NetworkClient> httpClient_;
};
}
}
}
}
#endif