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

#if !defined(OHOS_LITE) && defined(VIDEO_SUPPORT)

#define HST_LOG_TAG "ShareAllocator"

#include "share_allocator.h"
#include "ashmem.h"

namespace OHOS {
namespace Media {
namespace Plugin {
ShareAllocator::ShareAllocator(ShareMemType shareMemType)
    : Allocator(MemoryType::SHARE_MEMORY), shareMemType_(shareMemType)
{
}

void* ShareAllocator::Alloc(size_t size)
{
    return reinterpret_cast<void*>(AshmemCreate(0, size));
}

void ShareAllocator::Free(void* ptr) // NOLINT: void*
{
    (void)ptr;
}

ShareMemType ShareAllocator::GetShareMemType()
{
    return shareMemType_;
}
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif