/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef HISTREAMER_FOUNDATION_OSAL_UTILS_UTIL_H
#define HISTREAMER_FOUNDATION_OSAL_UTILS_UTIL_H

#ifndef MEDIA_NO_OHOS
#ifndef MEDIA_OHOS
#define MEDIA_OHOS
#endif
#else
#ifdef MEDIA_OHOS
#undef MEDIA_OHOS
#endif
#endif

#include <string>

namespace OHOS {
namespace Media {
namespace OSAL {
void SleepFor(unsigned ms);
bool ConvertFullPath(const std::string& partialPath, std::string& fullPath);
std::string AVStrError(int errnum);
} // namespace OSAL
} // namespace Media
} // namespace OHOS

#endif // HISTREAMER_FOUNDATION_OSAL_UTILS_UTIL_H
