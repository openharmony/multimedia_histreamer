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
#include "data_packer.h"
#include <cstring>
#include "foundation/log.h"
#include "plugin/common/plugin_buffer.h"

namespace OHOS {
namespace Media {
#define LOG_WARN_IF_FAIL(errorNo, errorMsg)                                                                            \
    do {                                                                                                               \
        if ((errorNo) != EOK) {                                                                                        \
            MEDIA_LOG_W(errorMsg);                                                                                     \
        }                                                                                                              \
    } while (0)

#define RETURN_FALSE_IF_NULL(ptr)                                                                                      \
    do {                                                                                                               \
        if ((ptr) == nullptr) {                                                                                        \
            return false;                                                                                              \
        }                                                                                                              \
    } while (0)

DataPacker::DataPacker() : mutex_(), que_(), assembler_(), size_(0), bufferOffset_(0), pts_(0), dts_(0)
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
    MEDIA_LOG_D("DataPacker PushData begin... buffer (size %" PUBLIC_LOG_U32 ", offset %" PUBLIC_LOG_U64 ")",
                AudioBufferSize(bufferPtr), offset);
    OSAL::ScopedLock lock(mutex_);
    size_ += AudioBufferSize(bufferPtr);
    if (que_.empty()) {
        bufferOffset_ = offset;
        dts_ = bufferPtr->dts;
        pts_ = bufferPtr->pts;
    }
    que_.emplace_back(std::move(bufferPtr));
    MEDIA_LOG_D("DataPacker PushData end... dataPacker (size %" PUBLIC_LOG_U32, "  offset %" PUBLIC_LOG_U64,
                size_.load(), bufferOffset_);
}

bool DataPacker::IsDataAvailable(uint64_t offset, uint32_t size, uint64_t &curOffset)
{
    OSAL::ScopedLock lock(mutex_);
    auto curOffsetTemp = bufferOffset_;
    if (que_.empty() || offset < curOffsetTemp || offset >= curOffsetTemp + size_) { // 原有数据无法命中, 则删除原有数据
        curOffset = offset;
        FlushInternal();
        MEDIA_LOG_D("IsDataAvailable false, offset < bufferOffset_");
        return false;
    }
    int bufCnt = que_.size();
    auto offsetEnd = offset + size;
    auto curOffsetEnd = bufferOffset_ + AudioBufferSize(que_.front());
    if (bufCnt == 1) {
        curOffset = curOffsetEnd;
        MEDIA_LOG_D("IsDataAvailable bufCnt == 1, result %" PUBLIC_LOG_D32, offsetEnd <= curOffsetEnd);
        return offsetEnd <= curOffsetEnd;
    }
    auto preOffsetEnd = curOffsetEnd;
    for (auto i = 1; i < bufCnt; ++i) {
        curOffsetEnd = preOffsetEnd + AudioBufferSize(que_[i]);
        if (curOffsetEnd >= offsetEnd) {
            MEDIA_LOG_D("IsDataAvailable true, last buffer index %" PUBLIC_LOG_U32 ", offsetEnd %" PUBLIC_LOG_U64
                ", curOffsetEnd %" PUBLIC_LOG_U64, i, offsetEnd, curOffsetEnd);
            return true;
        } else {
            preOffsetEnd = curOffsetEnd;
        }
    }
    if (preOffsetEnd >= offsetEnd) {
        MEDIA_LOG_D("IsDataAvailable true, use all buffers, last buffer index %" PUBLIC_LOG_U32 ", offsetEnd %"
            PUBLIC_LOG_U64 ", curOffsetEnd %" PUBLIC_LOG_U64, bufCnt - 1, offsetEnd, curOffsetEnd);
        return true;
    }
    curOffset = preOffsetEnd;
    MEDIA_LOG_D("IsDataAvailable false, offsetEnd %" PUBLIC_LOG_U64 ", curOffsetEnd %" PUBLIC_LOG_U64,
                offsetEnd, curOffsetEnd);
    return false;
}

bool DataPacker::PeekRange(uint64_t offset, uint32_t size, AVBufferPtr& bufferPtr)
{
    OSAL::ScopedLock lock(mutex_);
    return PeekRangeInternal(offset, size, bufferPtr);
}

// 在调用当前接口前需要先调用IsDataAvailable()
// offset - 要peek的数据起始位置 在media file文件 中的 offset
// size - 要读取的长度
// bufferPtr - 出参
bool DataPacker::PeekRangeInternal(uint64_t offset, uint32_t size, AVBufferPtr& bufferPtr)
{
    MEDIA_LOG_D("DataPacker PeekRange(offset, size) = (%" PUBLIC_LOG PRIu64 ", %"
                PUBLIC_LOG PRIu32 ")...", offset, size);
    FALSE_RETURN_V(bufferPtr != nullptr, false);
    assembler_.resize(size);
    uint8_t* dstPtr = assembler_.data();
    auto offsetEnd = offset + size;
    auto curOffsetEnd = bufferOffset_ + AudioBufferSize(que_[0]);
    if (offsetEnd <= curOffsetEnd) { // 0号buffer够用
        dstPtr = AudioBufferWritableData(bufferPtr, size);
        RETURN_FALSE_IF_NULL(dstPtr);
        LOG_WARN_IF_FAIL(memcpy_s(dstPtr, size,
            AudioBufferReadOnlyData(que_[0]) + offset - bufferOffset_, size), "failed to memcpy");
    } else { // 0号buffer不够用
        // 拷贝0号buffer需要的内容
        auto srcSize = AudioBufferSize(que_[0]) - (offset - bufferOffset_);
        dstPtr = AudioBufferWritableData(bufferPtr, srcSize);
        RETURN_FALSE_IF_NULL(dstPtr);
        LOG_WARN_IF_FAIL(memcpy_s(dstPtr, srcSize,
            AudioBufferReadOnlyData(que_[0]) + offset - bufferOffset_, srcSize),
            "failed to memcpy");

        // 数据不足的部分，从1/2/...号buffer拷贝
        auto prevMediaOffsetEnd = bufferOffset_ + AudioBufferSize(que_[0]);
        dstPtr += srcSize;
        size -= srcSize;
        for (size_t i = 1; i < que_.size(); ++i) {
            curOffsetEnd = prevMediaOffsetEnd + AudioBufferSize(que_[i]);
            if (curOffsetEnd >= offsetEnd) {
                LOG_WARN_IF_FAIL(memcpy_s(dstPtr, size, AudioBufferReadOnlyData(que_[i]), size),
                                 "failed to memcpy");
                break;
            } else {
                srcSize = AudioBufferSize(que_[i]);
                LOG_WARN_IF_FAIL(memcpy_s(dstPtr, srcSize, AudioBufferReadOnlyData(que_[i]), srcSize),
                                 "failed to memcpy");
                dstPtr += srcSize;
                size -= srcSize;
                prevMediaOffsetEnd += srcSize;
            }
        }
    }
    WrapAssemblerBuffer(offset).swap(bufferPtr);
    return true;
}

// 单个buffer不足以存储请求的数据，需要合并多个buffer从而组成目标buffer
bool DataPacker::RepackBuffers(uint64_t offset, uint32_t size, AVBufferPtr& bufferPtr)
{
    FALSE_RETURN_V(PeekRangeInternal(offset, size, bufferPtr), false);

    // 删除已经拿走的数据
    auto offsetEnd = offset + size;
    size_ -= size;
    while (!que_.empty()) {
        auto& buf = que_.front();
        auto curOffsetEnd = bufferOffset_ + AudioBufferSize(buf);
        if (curOffsetEnd < offsetEnd) {
            que_.pop_front();
            bufferOffset_ = curOffsetEnd;
            continue;
        } else if (curOffsetEnd == offsetEnd) {
            que_.pop_front();
            bufferOffset_ = curOffsetEnd;
            if (!que_.empty()) {
                dts_ = que_.front()->dts;
                pts_ = que_.front()->pts;
            }
        } else {
            auto removeSize = AudioBufferSize(que_.front()) - (curOffsetEnd - offsetEnd);
            RemoveBufferContent(que_.front(), removeSize);
            bufferOffset_ = offsetEnd;
            dts_ = buf->dts;
            pts_ = buf->pts;
        }
        break;
    }
    return true;
}

// 在调用当前接口前需要先调用IsDataAvailable()
bool DataPacker::GetRange(uint64_t offset, uint32_t size, AVBufferPtr& bufferPtr)
{
    MEDIA_LOG_D("DataPacker GetRange(offset, size) = (%" PUBLIC_LOG PRIu64 ", %"
                PUBLIC_LOG PRIu32 ")...", offset, size);
    OSAL::ScopedLock lock(mutex_);
    if (!bufferPtr) {
        bufferPtr = std::make_shared<AVBuffer>();
    }
    if (RepackBuffers(offset, size, bufferPtr)) {
        return true;
    }
    MEDIA_LOG_E("GetRange peek data failed, have you called IsDataAvailable() first?");
    return false;
}

AVBufferPtr DataPacker::WrapAssemblerBuffer(uint64_t offset)
{
    MEDIA_LOG_D("DataPacker WrapAssemblerBuffer, offset = %" PUBLIC_LOG PRIu64, offset);
    (void)offset;
    auto bufferPtr = std::make_shared<AVBuffer>();
    auto dataPtr = std::shared_ptr<uint8_t>(assembler_.data(), [this](void* ptr) { assembler_.resize(0); });
    auto bufferData = bufferPtr->WrapMemoryPtr(dataPtr, assembler_.size(), assembler_.size());
    bufferPtr->dts = dts_;
    bufferPtr->pts = pts_;
    return bufferPtr;
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
    bufferOffset_ = 0;
    dts_ = 0;
    pts_ = 0;
}

void DataPacker::RemoveBufferContent(std::shared_ptr<AVBuffer> &buffer, size_t removeSize) {
    auto memory = buffer->GetMemory();
    auto copySize = memory->GetSize() - removeSize;
    memmove_s(memory->GetWritableAddr(copySize), memory->GetCapacity(),
              memory->GetReadOnlyData(removeSize), copySize);
}
} // namespace Media
} // namespace OHOS
