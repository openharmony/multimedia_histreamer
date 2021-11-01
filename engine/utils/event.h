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

#ifndef HISTREAMER_FOUNDATION_EVENT_H
#define HISTREAMER_FOUNDATION_EVENT_H

#include "common/any.h"

namespace OHOS {
namespace Media {

// 各个组件向Pipeline报告的事件类型
enum EventType {
    EVENT_READY = 0,
    EVENT_PROGRESS,
    EVENT_COMPLETE,
    EVENT_ERROR,
    EVENT_BUFFERING,
    EVENT_BUFFER_PROGRESS,
    EVENT_DECODER_ERROR,
};

struct Event {
    EventType type;
    Plugin::Any param;
};
} // namespace Media
} // namespace OHOS
#endif
