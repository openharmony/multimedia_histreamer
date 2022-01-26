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

#define HST_LOG_TAG "MediaStatStub"

#include "media_stat_stub.h"

#include <algorithm>
#include "foundation/log.h"

namespace OHOS {
namespace Media {
void MediaStatStub::Reset()
{
    mediaStats.clear();
}

void MediaStatStub::Append(MediaType mediaType)
{
    for (auto& stat : mediaStats) {
        if (stat.mediaType == mediaType) {
            return;
        }
    }
    mediaStats.emplace_back(mediaType);
}

void MediaStatStub::ReceiveEvent(const EventType& eventType, int64_t param)
{
    switch (eventType) {
        case EventType::EVENT_COMPLETE:
            for (auto& stat : mediaStats) {
                if (stat.mediaType == MediaType::AUDIO) {
                    stat.completeEventReceived = true;
                    break;
                }
            }
            break;
        case EventType::EVENT_AUDIO_PROGRESS:
            for (auto& stat : mediaStats) {
                if (stat.mediaType == MediaType::AUDIO) {
                    stat.currentPosition = param;
                    break;
                }
            }
            break;
        default:
            MEDIA_LOG_W("MediaStats::ReceiveEvent receive unexpected event %d", static_cast<int>(eventType));
            break;
    }
}

int64_t MediaStatStub::GetCurrentPosition()
{
    int64_t currentPosition = 0;
    for (const auto& stat : mediaStats) {
        currentPosition = std::max(currentPosition, stat.currentPosition.load());
    }
    return currentPosition;
}

bool MediaStatStub::IsEventCompleteAllReceived()
{
    return std::all_of(mediaStats.begin(), mediaStats.end(), [](const MediaStat& stat) {
        return stat.completeEventReceived.load();
    });
}
} // Media
} // OHOS