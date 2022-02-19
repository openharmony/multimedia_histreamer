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

    void PushData(AVBufferPtr bufferPtr, uint64_t offset);

    bool IsDataAvailable(uint64_t offset, uint32_t size, uint64_t &curOffset);

    bool PeekRange(uint64_t offset, uint32_t size, AVBufferPtr &bufferPtr);

    bool GetRange(uint64_t offset, uint32_t size, AVBufferPtr &bufferPtr);

    void Flush();

private:
    struct Position {
        int32_t index; // Buffer index, -1 means this Position is invalid
        uint32_t bufferOffset; // Offset in the buffer
        uint64_t mediaOffset;  // Offset in the media file
    };

    // first - start position;
    // second - end position, not include bufferOffset byte.
    using PositionPair = std::pair<Position, Position>;

    void RemoveBufferContent(std::shared_ptr<AVBuffer> &buffer, size_t removeSize);

    void RemoveBuffers(uint64_t offset, size_t size, uint32_t startIndex, uint32_t endIndex);

    bool PeekRangeInternal(uint64_t offset, uint32_t size, AVBufferPtr &bufferPtr, bool isGet);

    void FlushInternal();

    bool FindFirstBufferToCopy(uint64_t offset, int32_t &startIndex, uint64_t &prevOffset);

    std::string ToString();

    OSAL::Mutex mutex_;
    std::deque<AVBufferPtr> que_;
    std::atomic<uint32_t> size_;
    uint64_t mediaOffset_; // The media file offset of the first byte in data packer
    uint64_t pts_;
    uint64_t dts_;

    // The position in prev GetRange / current GetRange
    PositionPair prevGet {{-1, 0, 0}, {-1, 0, 0}};
    PositionPair currentGet {{-1, 0, 0}, {-1, 0, 0}};
};
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_DATA_PACKER_H
