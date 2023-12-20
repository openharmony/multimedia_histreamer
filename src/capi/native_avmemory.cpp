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

#include "native_avmemory.h"
#include "buffer/avsharedmemorybase.h"
#include "common/log.h"
#include "common/status.h"
#include "common/native_mfmagic.h"

using namespace OHOS::Media;

OH_AVMemory::OH_AVMemory(const std::shared_ptr<OHOS::Media::AVSharedMemory> &mem)
    : MFObjectMagic(MFMagic::MFMAGIC_SHARED_MEMORY), memory_(mem)
{
}

OH_AVMemory::~OH_AVMemory() {}

bool OH_AVMemory::IsEqualMemory(const std::shared_ptr<OHOS::Media::AVSharedMemory> &mem)
{
    return (mem == memory_) ? true : false;
}

struct OH_AVMemory *OH_AVMemory_Create(int32_t size)
{
    FALSE_RETURN_V_MSG_E(size >= 0, nullptr, "size %{public}d is error!", size);
    std::shared_ptr<AVSharedMemoryBase> sharedMemory =
        std::make_shared<AVSharedMemoryBase>(size, AVSharedMemory::FLAGS_READ_WRITE, "userBuffer");
    FALSE_RETURN_V_MSG_E(sharedMemory->Init() == static_cast<int32_t>(Status::OK), nullptr,
                         "create OH_AVMemory failed");

    struct OH_AVMemory *mem = new (std::nothrow) OH_AVMemory(sharedMemory);
    FALSE_RETURN_V_MSG_E(mem != nullptr, nullptr, "failed to new OH_AVMemory");
    mem->isUserCreated = true;

    return mem;
}

uint8_t *OH_AVMemory_GetAddr(struct OH_AVMemory *mem)
{
    FALSE_RETURN_V_MSG_E(mem != nullptr, nullptr, "input mem is nullptr!");
    FALSE_RETURN_V_MSG_E(mem->magic_ == MFMagic::MFMAGIC_SHARED_MEMORY, nullptr, "magic error!");
    FALSE_RETURN_V_MSG_E(mem->memory_ != nullptr, nullptr, "memory is nullptr!");
    return mem->memory_->GetBase();
}

int32_t OH_AVMemory_GetSize(struct OH_AVMemory *mem)
{
    FALSE_RETURN_V_MSG_E(mem != nullptr, -1, "input mem is nullptr!");
    FALSE_RETURN_V_MSG_E(mem->magic_ == MFMagic::MFMAGIC_SHARED_MEMORY, -1, "magic error!");
    FALSE_RETURN_V_MSG_E(mem->memory_ != nullptr, -1, "memory is nullptr!");
    return mem->memory_->GetSize();
}

OH_AVErrCode OH_AVMemory_Destroy(struct OH_AVMemory *mem)
{
    FALSE_RETURN_V_MSG_E(mem != nullptr, AV_ERR_INVALID_VAL, "input mem is nullptr!");
    FALSE_RETURN_V_MSG_E(mem->magic_ == MFMagic::MFMAGIC_SHARED_MEMORY, AV_ERR_INVALID_VAL, "magic error!");
    FALSE_RETURN_V_MSG_E(mem->isUserCreated, AV_ERR_INVALID_VAL, "input mem is not user created!");
    delete mem;
    return AV_ERR_OK;
}