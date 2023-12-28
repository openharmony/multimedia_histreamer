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

#ifndef AV_SHARED_ALLOCATOR_H
#define AV_SHARED_ALLOCATOR_H

#include "buffer/avallocator.h"

namespace OHOS {
namespace Media {
/**
 * @brief The allocator that allocate shared memory.
 */
class AVSharedAllocator : public AVAllocator {
public:
    friend class AVAllocatorFactory;
    ~AVSharedAllocator() override = default;

    void *Alloc(int32_t capacity) override;
    bool Free(void *ptr) override;
    MemoryType GetMemoryType() override;

    MemoryFlag GetMemFlag();

private:
    AVSharedAllocator();

    MemoryFlag memFlag_;
};
} // namespace Media
} // namespace OHOS
#endif // AV_SHARED_ALLOCATOR_H