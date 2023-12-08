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

#ifndef NATIVE_MFMAGIC_H
#define NATIVE_MFMAGIC_H

#include <refbase.h>
#include "buffer/avbuffer.h"
#include "buffer/avsharedmemory.h"
#include "meta/format.h"

#define MF_MAGIC(a, b, c, d) (((a) << 24) + ((b) << 16) + ((c) << 8) + ((d) << 0))

enum class MFMagic {
    MFMAGIC_AVBUFFER = MF_MAGIC('B', 'B', 'U', 'F'),
    MFMAGIC_FORMAT = MF_MAGIC('F', 'R', 'M', 'T'),
    MFMAGIC_SHARED_MEMORY = MF_MAGIC('S', 'M', 'E', 'M'),
};

struct MFObjectMagic : public OHOS::RefBase {
    explicit MFObjectMagic(enum MFMagic m) : magic_(m) {}
    virtual ~MFObjectMagic() = default;
    enum MFMagic magic_;
};

struct OH_AVBuffer : public MFObjectMagic {
    explicit OH_AVBuffer(const std::shared_ptr<OHOS::Media::AVBuffer> &buf);
    virtual ~OH_AVBuffer() = default;
    bool IsEqualBuffer(const std::shared_ptr<OHOS::Media::AVBuffer> &buf);
    std::shared_ptr<OHOS::Media::AVBuffer> buffer_;
    bool isUserCreated = false;
};

struct OH_AVMemory : public MFObjectMagic {
    explicit OH_AVMemory(const std::shared_ptr<OHOS::Media::AVSharedMemory> &mem);
    ~OH_AVMemory() override;
    bool IsEqualMemory(const std::shared_ptr<OHOS::Media::AVSharedMemory> &mem);
    const std::shared_ptr<OHOS::Media::AVSharedMemory> memory_;
    bool isUserCreated = false;
};

struct OH_AVFormat : public MFObjectMagic {
    OH_AVFormat();
    explicit OH_AVFormat(const OHOS::Media::Format &fmt);
    ~OH_AVFormat() override;
    OHOS::Media::Format format_;
    char *outString_ = nullptr;
    char *dumpInfo_ = nullptr;
};
#endif // NATIVE_MFMAGIC_H