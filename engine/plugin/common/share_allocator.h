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

#ifndef HISTREAMER_PLUGIN_COMMON_SHARE_ALLOCATOR_H
#define HISTREAMER_PLUGIN_COMMON_SHARE_ALLOCATOR_H

#if !defined(OHOS_LITE) && defined(VIDEO_SUPPORT)

#include "plugin_memory.h"

namespace OHOS {
namespace Media {
namespace Plugin {
/**
* @brief Enumerate the shared memory types.
*/
enum struct ShareMemType : uint8_t {
    /** Readable and writable shared memory */
    READ_WRITE_TYPE = 0x1,
    /** Readable shared memory */
    READ_ONLY_TYPE = 0x2,
};

class ShareAllocator : public Allocator {
public:
    explicit ShareAllocator(ShareMemType shareMemType = ShareMemType::READ_WRITE_TYPE);
    ~ShareAllocator() override = default;

    void* Alloc(size_t size) override;
    void Free(void* ptr) override; // NOLINT: void*

    ShareMemType GetShareMemType();

private:
    ShareMemType shareMemType_;
};
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif
#endif // HISTREAMER_PLUGIN_COMMON_SHARE_ALLOCATOR_H
