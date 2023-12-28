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

#ifndef AV_VIRTUAL_MEMORY_H
#define AV_VIRTUAL_MEMORY_H

#include "buffer/avallocator.h"
#include "buffer/avbuffer.h"

namespace OHOS {
namespace Media {
class AVVirtualMemory : public AVMemory {
public:
    AVVirtualMemory();
    ~AVVirtualMemory() override;
    MemoryType GetMemoryType() override;

private:
    Status Init() override;
};
} // namespace Media
} // namespace OHOS
#endif // AV_VIRTUAL_MEMORY_H