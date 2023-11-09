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

#ifndef AV_SHARED_MEMORY_EXT_H
#define AV_SHARED_MEMORY_EXT_H

#include "buffer/avallocator.h"
#include "buffer/avbuffer.h"
#include "common/status.h"

namespace OHOS {
namespace Media {
class AVSharedMemoryExt : public AVMemory {
public:
    explicit AVSharedMemoryExt();
    ~AVSharedMemoryExt() override;
    uint8_t *GetAddr() override;
    MemoryType GetMemoryType() override;
    int32_t GetFileDescriptor() override;
    MemoryFlag GetMemFlag();

private:
    int32_t Init() override;
    int32_t Init(MessageParcel &parcel) override;
    bool WriteToMessageParcel(MessageParcel &parcel) override;
    void Close() noexcept;
    int32_t MapMemoryAddr();
    int32_t fd_;
    bool isFirstFlag_;
    MemoryFlag memFlag_;
};
} // namespace Media
} // namespace OHOS
#endif // AV_SHARED_MEMORY_EXT_H