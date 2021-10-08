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

#ifndef HISTREAMER_PLUGIN_CORE_UTILS_H
#define HISTREAMER_PLUGIN_CORE_UTILS_H

namespace OHOS {
namespace Media {
namespace Plugin {
#define LOG_WARN_IF_NOT_OK(plugin, status) \
do { \
    if ((status) != Status::OK) { \
        MEDIA_LOG_W("plugin %s %s failed with status %d", (plugin)->GetName().c_str(), __FUNCTION__, status); \
    } \
} while (0)

#define RETURN_WRONG_STATE_IF_CON_TRUE(condition, plugin, state) \
do { \
    if (condition) { \
        MEDIA_LOG_E("plugin %s cannot %s in state %s", (plugin)->GetName().c_str(), __FUNCTION__, \
            GetStateString(state)); \
        return Status::ERROR_WRONG_STATE; \
    } \
} while (0)

#define RETURN_WRONG_STATE_IF_CON_FALSE(condition, plugin, state) \
do { \
    if (!(condition)) { \
        MEDIA_LOG_E("plugin %s cannot %s in state %s", (plugin)->GetName().c_str(), __FUNCTION__, \
        GetStateString(state)); \
        return Status::ERROR_WRONG_STATE; \
    } \
} while (0)

inline const char* GetStateString(OHOS::Media::Plugin::State state)
{
    using namespace OHOS::Media::Plugin;
    switch (state) {
        case State::CREATED:
            return "Created";
        case State::INITIALIZED:
            return "Initialized";
        case State::DESTROYED:
            return "Destroyed";
        case State::PREPARED:
            return "Prepared";
        case State::RUNNING:
            return "Running";
        case State::PAUSED:
            return "Paused";
        default:
            return "Invalid";
    }
}
}
}
}

#endif //HISTREAMER_PLUGIN_CORE_UTILS_H
