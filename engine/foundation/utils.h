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

#ifndef HISTREAMER_FOUNDATION_UTILS_H
#define HISTREAMER_FOUNDATION_UTILS_H

#include <string>

#define CALL_PTR_FUNC(ptr, func, param)                           \
        if ((ptr)) {                                              \
            (ptr)->func(param);                                   \
        } else {                                                  \
            MEDIA_LOG_E("Call weakPtr " #func " error." );        \
        }

#define UNUSED_VARIABLE(v) ((void)(v))

namespace OHOS {
namespace Media {
inline bool StringStartsWith(const std::string& input, const std::string& prefix)
{
    return input.rfind(prefix, 0) == 0;
}

template <typename E>
constexpr typename std::underlying_type<E>::type to_underlying(E e) noexcept
{
    return static_cast<typename std::underlying_type<E>::type>(e);
}
}
}
#endif // HISTREAMER_FOUNDATION_UTILS_H
