/*
 * Copyright (c) 2021-2021 Huawei Device Co., Ltd.
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

#ifndef HISTREAMER_DATA_PACKER_H
#define HISTREAMER_DATA_PACKER_H

#include <atomic>
#include <deque>
#include <vector>

#include "osal/thread/mutex.h"
#include "osal/thread/scoped_lock.h"
#include "utils/type_define.h"

namespace OHOS {
namespace Media {
class DataPacker {
public:
    DataPacker();

    ~DataPacker();

    DataPacker(const DataPacker& other) = delete;

    DataPacker& operator=(const DataPacker& other) = delete;

    void PushData(AVBufferPtr bufferPtr);

    bool IsDataAvailable(uint64_t offset, uint32_t size);

    bool PeekRange(uint64_t offset, uint32_t size, AVBufferPtr &bufferPtr);

    bool GetRange(uint64_t offset, uint32_t size, AVBufferPtr &bufferPtr);

    AVBufferPtr GetRange(uint64_t offset, uint32_t size);

    void Flush();

private:
    static AVBufferPtr MakeAliasBuffer(AVBufferPtr bufferPtr, uint32_t offset, uint32_t size);

    AVBufferPtr WrapAssemblerBuffer(uint64_t offset);

    bool RepackBuffers(uint64_t offset, uint32_t size, AVBufferPtr &bufferPtr);

    OSAL::Mutex mutex_;
    std::deque<AVBufferPtr> que_;
    std::vector<uint8_t> assembler_;
    std::atomic<uint32_t> size_;
    uint32_t bufferOffset_;
    uint64_t pts_;
    uint64_t dts_;
};
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_DATA_PACKER_H
