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

#ifndef HISTREAMER_FOUNDATION_OSAL_BASE_COMMON_H
#define HISTREAMER_FOUNDATION_OSAL_BASE_COMMON_H

#include <cstdio>
#include <cstdlib>

namespace OHOS {
namespace Media {
namespace OSAL {
inline void Assert(const char* file, int line)
{
    (void)fprintf(stderr, "assertion failed for: %s, at line: %d", file, line);
    abort();
}
} // namespace OSAL
} // namespace Media
} // namespace OHOS

#ifdef NDEBUG
#define ASSERT(_expr) ((void)0)
#else
#define ASSERT(_expr) ((_expr) ? ((void)0) : OHOS::Media::OSAL::Assert(__FILE__, __LINE__))
#endif

#endif // HISTREAMER_FOUNDATION_OSAL_BASE_COMMON_H
