/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef AV_HARDWARE_ALLOCATOR_H
#define AV_HARDWARE_ALLOCATOR_H

#include "inner_api/buffer/avallocator.h"
#include "dmabuf_alloc.h"

namespace OHOS {
namespace Media {
class AVHardwareAllocator : public AVAllocator {
public:
    friend class AVAllocatorFactory;
    ~AVHardwareAllocator() override = default;

    void *Alloc(int32_t capacity) override;
    bool Free(void *ptr) override;
    MemoryType GetMemoryType() override;

    MemoryFlag GetMemFlag();
    int32_t GetFileDescriptor();

private:
    explicit AVHardwareAllocator();
    int32_t MapMemoryAddr();

    int32_t fd_;
    int32_t capacity_;
    uint8_t *allocBase_;
    MemoryFlag memFlag_;
};
} // namespace Media
} // namespace OHOS
#endif // AV_HARDWARE_ALLOCATOR_H