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

#ifndef HISTREAMER_PLUGIN_TAG_VALUE_MAP_H
#define HISTREAMER_PLUGIN_TAG_VALUE_MAP_H

#include "plugin_tags.h"

namespace OHOS {
namespace Media {
namespace Plugin {
#define DEFINE_INSERT_GET_FUNC(condition, ValueType)             \
        template<Tag tag>                                        \
        inline typename std::enable_if<condition, bool>::type    \
        Insert(ValueType value)                                  \
        {                                                        \
            map.insert(std::make_pair(tag, value));              \
            return true;                                         \
        }                                                        \
        template<Tag tag>                                        \
        inline typename std::enable_if<condition, bool>::type    \
        Get(ValueType& value)                                    \
        {                                                        \
            if (map.count(tag) == 0) {                           \
                 return false;                                   \
            }                                                    \
            Any& temp = map.at(tag);                             \
            if (!temp.SameTypeWith(typeid(ValueType))) {         \
                 return false;                                   \
            }                                                    \
            value = AnyCast<ValueType>(map.at(tag));             \
            return true;                                         \
        }


class TagValueMap {
public:
    DEFINE_INSERT_GET_FUNC(
        tag == Tag::BUFFER_ALLOCATOR or
        tag == Tag::VIDEO_SURFACE, std::nullptr_t);
    DEFINE_INSERT_GET_FUNC(
        tag == Tag::TRACK_ID or
        tag == Tag::REQUIRED_OUT_BUFFER_CNT, int32_t);
    DEFINE_INSERT_GET_FUNC(
        tag == Tag::MEDIA_DURATION or
        tag == Tag::MEDIA_FILE_SIZE, int64_t);

private:
    std::map<Tag, Any> map;
};
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PLUGIN_TAG_VALUE_MAP_H
