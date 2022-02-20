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
#define HST_LOG_TAG "DataPacker"
#define MEDIA_LOG_DEBUG 0

#include "data_packer.h"
#include <cstring>
#include "foundation/log.h"
#include "filters/common/dump_buffer.h"

namespace OHOS {
namespace Media {
#define EXEC_WHEN_GET(isGet, exec)     \
    do {                               \
        if (isGet) {                   \
            exec;                      \
        }                              \
    } while(0)

DataPacker::DataPacker() : mutex_(), que_(), size_(0), mediaOffset_(0), pts_(0), dts_(0)
{
    MEDIA_LOG_I("DataPacker ctor...");
}

DataPacker::~DataPacker()
{
    MEDIA_LOG_I("DataPacker dtor...");
}

inline static size_t AudioBufferSize(AVBufferPtr& ptr)
{
    return ptr->GetMemory()->GetSize();
}

inline static uint8_t* AudioBufferWritableData(AVBufferPtr& ptr, size_t size, size_t position = 0)
{
    return ptr->GetMemory()->GetWritableAddr(size, position);
}

inline static const uint8_t* AudioBufferReadOnlyData(AVBufferPtr& ptr)
{
    return ptr->GetMemory()->GetReadOnlyData();
}

void DataPacker::PushData(AVBufferPtr bufferPtr, uint64_t offset)
{
    MEDIA_LOG_D("DataPacker PushData begin... buffer (offset %" PUBLIC_LOG_U64 ", size %" PUBLIC_LOG_U32 ")",
                offset, AudioBufferSize(bufferPtr));
    DUMP_BUFFER2LOG("DataPacker Push", bufferPtr, offset);
    OSAL::ScopedLock lock(mutex_);
    size_ += AudioBufferSize(bufferPtr);
    if (que_.empty()) {
        mediaOffset_ = offset;
        dts_ = bufferPtr->dts;
        pts_ = bufferPtr->pts;
    }
    que_.emplace_back(std::move(bufferPtr));
    MEDIA_LOG_D("DataPacker PushData end... dataPacker (offset %" PUBLIC_LOG_U64 ", size %" PUBLIC_LOG_U32 ")",
                mediaOffset_, size_.load());
}

bool DataPacker::IsDataAvailable(uint64_t offset, uint32_t size, uint64_t &curOffset)
{
    MEDIA_LOG_D("dataPacker (offset %" PUBLIC_LOG_U64 ", size %" PUBLIC_LOG_U32 "), curOffsetEnd is %" PUBLIC_LOG_U64,
                mediaOffset_, size_.load(), mediaOffset_ + size_.load());
    MEDIA_LOG_D("%" PUBLIC_LOG_S, ToString().c_str());
    OSAL::ScopedLock lock(mutex_);
    auto curOffsetTemp = mediaOffset_;
    if (que_.empty() || offset < curOffsetTemp || offset > curOffsetTemp + size_) { // 原有数据无法命中, 则删除原有数据
        curOffset = offset;
        FlushInternal();
        MEDIA_LOG_D("IsDataAvailable false, offset not in cached data, clear it.");
        return false;
    }
    size_t bufCnt = que_.size();
    uint64_t offsetEnd = offset + size;
    uint64_t curOffsetEnd = mediaOffset_ + AudioBufferSize(que_.front());
    if (bufCnt == 1) {
        curOffset = curOffsetEnd;
        MEDIA_LOG_D("IsDataAvailable bufCnt == 1, result %" PUBLIC_LOG_D32, offsetEnd <= curOffsetEnd);
        return offsetEnd <= curOffsetEnd;
    }
    auto preOffsetEnd = curOffsetEnd;
    for (size_t i = 1; i < bufCnt; ++i) {
        curOffsetEnd = preOffsetEnd + AudioBufferSize(que_[i]);
        if (curOffsetEnd >= offsetEnd) {
            MEDIA_LOG_D("IsDataAvailable true, last buffer index %" PUBLIC_LOG_ZU ", offsetEnd %" PUBLIC_LOG_U64
                ", curOffsetEnd %" PUBLIC_LOG_U64, i, offsetEnd, curOffsetEnd);
            return true;
        } else {
            preOffsetEnd = curOffsetEnd;
        }
    }
    if (preOffsetEnd >= offsetEnd) {
        MEDIA_LOG_D("IsDataAvailable true, use all buffers, last buffer index %" PUBLIC_LOG_ZU ", offsetEnd %"
            PUBLIC_LOG_U64 ", curOffsetEnd %" PUBLIC_LOG_U64, bufCnt - 1, offsetEnd, curOffsetEnd);
        return true;
    }
    curOffset = preOffsetEnd;
    MEDIA_LOG_D("IsDataAvailable false, offsetEnd %" PUBLIC_LOG_U64 ", curOffsetEnd %" PUBLIC_LOG_U64,
                offsetEnd, preOffsetEnd);
    return false;
}

bool DataPacker::PeekRange(uint64_t offset, uint32_t size, AVBufferPtr& bufferPtr)
{
    OSAL::ScopedLock lock(mutex_);
    return PeekRangeInternal(offset, size, bufferPtr, false);
}

// 在调用当前接口前需要先调用IsDataAvailable()
// offset - 要peek的数据起始位置 在media file文件 中的 offset
// size - 要读取的长度
// bufferPtr - 出参
bool DataPacker::PeekRangeInternal(uint64_t offset, uint32_t size, AVBufferPtr &bufferPtr, bool isGet)
{
    MEDIA_LOG_D("PeekRangeInternal (offset, size) = (%" PUBLIC_LOG_U64 ", %" PUBLIC_LOG_U32 ")...", offset, size);
    uint32_t needCopySize = size;
    int32_t startIndex = 0; // The index of buffer that we first use
    int32_t usedCount = 1;
    size_t copySize = 0;
    uint32_t firstBufferOffset = 0;
    uint32_t lastBufferOffsetEnd = 0;
    uint8_t* dstPtr = AudioBufferWritableData(bufferPtr, needCopySize);
    FALSE_RETURN_V(dstPtr != nullptr, false);

    auto offsetEnd = offset + needCopySize;
    auto curOffsetEnd = mediaOffset_ + AudioBufferSize(que_[startIndex]);
    if (offsetEnd <= curOffsetEnd) { // first buffer is enough
        copySize = CopyFirstBuffer(offset, size, startIndex, mediaOffset_, dstPtr, bufferPtr,
                        firstBufferOffset);
        needCopySize -= copySize;
        FALSE_LOG_MSG_E(needCopySize == 0, "First buffer is enough, but copySize is not enough");
        lastBufferOffsetEnd = firstBufferOffset + size;
        EXEC_WHEN_GET(isGet, currentGet = std::make_pair(Position{startIndex, firstBufferOffset, offset},
            Position{startIndex, lastBufferOffsetEnd, offset + size}));
        return true;
    } else { // first buffer not enough
        // Find the first buffer that should copy
        uint64_t prevOffset; // The media offset of the startIndex buffer start byte
        FALSE_RETURN_V(FindFirstBufferToCopy(offset, startIndex, prevOffset), false);
        copySize = CopyFirstBuffer(offset, size, startIndex, prevOffset, dstPtr, bufferPtr, firstBufferOffset);

        needCopySize -= copySize;
        if (needCopySize == 0) { // First buffer is enough
            lastBufferOffsetEnd = firstBufferOffset + copySize;
            EXEC_WHEN_GET(isGet, currentGet = std::make_pair(Position{startIndex, firstBufferOffset, offset},
                                                             Position{startIndex, lastBufferOffsetEnd, offset + size}));
            return true;
        }

        dstPtr += copySize;

        // First buffer is not enough, copy from successive buffers
        usedCount += CopyFromSuccessiveBuffer(prevOffset, offsetEnd, startIndex, dstPtr, needCopySize,
                                              lastBufferOffsetEnd);
    }
    EXEC_WHEN_GET(isGet, currentGet = std::make_pair(Position{startIndex, firstBufferOffset, offset},
        Position{startIndex + usedCount - 1, lastBufferOffsetEnd, offset + size}));

    // Update to the real size, especially at the end.
    bufferPtr->GetMemory()->UpdateDataSize(size - needCopySize);
    return true;
}

// Call IsDataAvailable() first before call GetRange
bool DataPacker::GetRange(uint64_t offset, uint32_t size, AVBufferPtr& bufferPtr)
{
    MEDIA_LOG_D("DataPacker GetRange(offset, size) = (%" PUBLIC_LOG_U64 ", %"
                PUBLIC_LOG_U32 ")...", offset, size);
    DUMP_BUFFER2LOG("GetRange Input", bufferPtr, 0);
    FALSE_RET_V_MSG_E(bufferPtr && (!bufferPtr->IsEmpty()) && bufferPtr->GetMemory()->GetCapacity() >= size, false,
                      "GetRange input bufferPtr empty or capacity not enough.");

    OSAL::ScopedLock lock(mutex_);
    FALSE_RETURN_V(!que_.empty(), false);
    prevGet = currentGet; // store last get position to prevGet

    FALSE_RETURN_V(PeekRangeInternal(offset, size, bufferPtr, true), false);
    RemoveBuffers(offset, size, currentGet.first.index, currentGet.second.index);
    MEDIA_LOG_D("RemoveBuffers called (offset, size, startIndex, endIndex) = (%" PUBLIC_LOG_U64 ", %"
                PUBLIC_LOG_U32 ", %" PUBLIC_LOG_U32 ", %" PUBLIC_LOG_U32 ")", offset, size, startIndex, endIndex);
    MEDIA_LOG_D("%" PUBLIC_LOG_S, ToString().c_str());
    return true;
}

void DataPacker::Flush()
{
    MEDIA_LOG_I("DataPacker Flush called.");
    OSAL::ScopedLock lock(mutex_);
    FlushInternal();
}

void DataPacker::FlushInternal()
{
    MEDIA_LOG_D("DataPacker FlushInternal called.");
    que_.clear();
    size_ = 0;
    mediaOffset_ = 0;
    dts_ = 0;
    pts_ = 0;
}

void DataPacker::RemoveBufferContent(std::shared_ptr<AVBuffer> &buffer, size_t removeSize)
{
    auto memory = buffer->GetMemory();
    auto copySize = memory->GetSize() - removeSize;
    FALSE_LOG_MSG_E(memmove_s(memory->GetWritableAddr(copySize), memory->GetCapacity(),
        memory->GetReadOnlyData(removeSize), copySize) == EOK, "memmove failed.");
}

// Remove items between startIndex and endIndex, startIndex / endIndex are included.
// Currently, not make the remaining buffer continuous.
void DataPacker::RemoveBuffers(uint64_t offset, size_t size, uint32_t startIndex, uint32_t endIndex)
{
    auto beginIt = que_.begin();
    for(uint32_t i = 0; i < startIndex;i++) {
        beginIt++;
    }
    if (startIndex == endIndex) {
        que_.erase(beginIt);
    } else {
        auto endIt = que_.begin();
        for (uint32_t i = 0; i < endIndex; i++) {
            endIt++;
        }
        que_.erase(beginIt, endIt);
    }
    if (que_.empty()) {
        mediaOffset_ = 0;
        pts_ = 0;
        dts_ = 0;
    } else if (startIndex == 0) {
        mediaOffset_ += size;
        pts_ = que_[0]->pts;
        dts_ = que_[0]->dts;
    }
    size_ -= size;
}

bool DataPacker::FindFirstBufferToCopy(uint64_t offset, int32_t &startIndex, uint64_t &prevOffset)
{
    startIndex = 0;
    prevOffset= mediaOffset_;
    do {
        if (offset >= prevOffset && offset - prevOffset < AudioBufferSize(que_[startIndex])) {
            return true;
        }
        prevOffset += AudioBufferSize(que_[startIndex]);
        startIndex++;
    } while (startIndex < que_.size());
    return false;
}

// startOffset - the media offset of the index buffer
// startBufferOffset - the buffer offset that we start copy
size_t DataPacker::CopyFirstBuffer(uint64_t offset, size_t size, int32_t index, uint64_t startOffset, uint8_t *dstPtr,
                                   AVBufferPtr& dstBufferPtr, uint32_t &startBufferOffset) {
    size_t copySize= std::min(AudioBufferSize(que_[index]) - (offset - startOffset), size);
    NZERO_LOG(memcpy_s(dstPtr, copySize,
        AudioBufferReadOnlyData(que_[index]) + offset - startOffset, copySize));

    dstBufferPtr->pts = que_[index]->pts;
    dstBufferPtr->dts = que_[index]->dts;
    startBufferOffset = offset - startOffset;
    return copySize;
}

int32_t DataPacker::CopyFromSuccessiveBuffer(uint64_t prevOffset, uint64_t offsetEnd, int32_t startIndex,
                                          uint8_t *dstPtr,
                                          uint32_t &needCopySize, uint32_t &lastBufferOffsetEnd) {
    size_t copySize;
    int32_t usedCount = 0;
    uint64_t curOffsetEnd;
    prevOffset = prevOffset + AudioBufferSize(que_[startIndex]);
    for (size_t i = startIndex + 1; i < que_.size(); ++i) {
        usedCount++;
        curOffsetEnd = prevOffset + AudioBufferSize(que_[i]);
        if (curOffsetEnd >= offsetEnd) { // This buffer is enough
            NZERO_LOG(memcpy_s(dstPtr, needCopySize, AudioBufferReadOnlyData(que_[i]), needCopySize));
            lastBufferOffsetEnd = needCopySize;
            return usedCount; // Finished copy buffer
        } else {
            copySize = AudioBufferSize(que_[i]);
            NZERO_LOG(memcpy_s(dstPtr, copySize, AudioBufferReadOnlyData(que_[i]), copySize));
            dstPtr += copySize;
            needCopySize -= copySize;
            prevOffset += copySize;
        }
    }
    MEDIA_LOG_W("Processed all cached buffers, still not meet offsetEnd, maybe EOS reached.");
    return usedCount;
}

std::string DataPacker::ToString()
{
    return "DataPacker (offset " + std::to_string(mediaOffset_) + ", size " + std::to_string(size_) +
           ", buffer count " + std::to_string(que_.size()) + ")";
}
} // namespace Media
} // namespace OHOS
