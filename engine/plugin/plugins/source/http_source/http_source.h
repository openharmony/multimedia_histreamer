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
#ifndef HISTREAMER_HTTP_SOURCE_STREAMING_H
#define HISTREAMER_HTTP_SOURCE_STREAMING_H

#include <string>
#include <memory>
#include "network_client.h"
#include "streaming_source.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace HttpPlugin {
class HttpSource : public StreamingSource {
public:
    HttpSource();
    ~HttpSource();
    int Init(RxHeader headCallback, RxBody bodyCallback, void *userParam) override;
    bool Open(const std::string &url) override;
    void Close() override;
    int RequestData(long startPos, int len) override;
private:
    std::shared_ptr<NetworkClient> client_;
};
}
}
}
}

#endif