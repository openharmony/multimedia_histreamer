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

#ifndef HISTREAMER_MONITOR_CALLBACK_H
#define HISTREAMER_MONITOR_CALLBACK_H

#include <string>
#include <memory>
#include "plugin/interface/plugin_base.h"
#include "plugin/plugins/source/http_source/download/downloader.h"
#include "plugin/plugins/source/http_source/media_downloader.h"
#include "ring_buffer.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace HttpPlugin {
class MonitorCallback {
public:
    virtual ~MonitorCallback() = default;
    virtual void OnDownloadStatus(std::shared_ptr<DownloadRequest>& request) = 0;
};
}
}
}
}
#endif
