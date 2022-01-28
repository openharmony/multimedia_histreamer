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

#define HST_LOG_TAG "HttpSource"
#include "http_source.h"
#include "foundation/log.h"
#include "libcurl_client.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace HttpPlugin {
using namespace std;
HttpSource::HttpSource()
{
    MEDIA_LOG_D("IN");
    client_ = std::make_shared<LibcurlClient>();
}

HttpSource::~HttpSource()
{
    MEDIA_LOG_D("IN");
}

int HttpSource::Init(RxHeader headCallback, RxBody bodyCallback, void *userParam)
{
    return client_->Init(headCallback, bodyCallback, userParam);
}

int HttpSource::RequestData(long startPos, int len)
{
    return client_->RequestData(startPos, len);
}

bool HttpSource::Open(const string &url)
{
    return client_->Open(url);
}

void HttpSource::Close()
{
    client_->Close();
}
}
}
}
}