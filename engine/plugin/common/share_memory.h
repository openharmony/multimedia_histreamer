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

#ifndef HISTREAMER_PLUGIN_COMMON_SHARE_MEMORY_H
#define HISTREAMER_PLUGIN_COMMON_SHARE_MEMORY_H

#if !defined(OHOS_LITE) && defined(VIDEO_SUPPORT)

#include "plugin_buffer.h"
#include "share_allocator.h"

namespace OHOS {
namespace Media {
namespace Plugin {
class ShareMemory : public Memory {
public:
    ~ShareMemory() override;

    explicit ShareMemory(size_t capacity, std::shared_ptr<Allocator> allocator = nullptr, size_t align = 1);

    size_t Write(const uint8_t* in, size_t writeSize, size_t position) override;

    size_t Read(uint8_t* out, size_t readSize, size_t position) override;

    int GetShareMemoryFd();

private:
    void InitShareMemory(ShareMemType type);

    /// share buffer
    std::shared_ptr<Ashmem> sharedMem_ {nullptr};

    std::shared_ptr<ShareAllocator> shareAllocator_ {nullptr};

    int fd_;
};
} // namespace Plugin
} // namespace Media
} // namespace OHOS

#endif
#endif // HISTREAMER_PLUGIN_COMMON_SHARE_MEMORY_H
