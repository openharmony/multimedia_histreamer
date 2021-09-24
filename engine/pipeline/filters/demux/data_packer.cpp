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

inline static size_t AudioBufferCapacity(AVBufferPtr& ptr)
{
    return ptr->GetMemory()->GetCapacity();
}

inline static uint8_t* AudioBufferWritableData(AVBufferPtr& ptr, size_t size, size_t position = 0)
{
    return ptr->GetMemory()->GetWritableData(size, position);
}

inline static const uint8_t* AudioBufferReadOnlyData(AVBufferPtr& ptr)
{
    return ptr->GetMemory()->GetReadOnlyData();
}

void DataPacker::PushData(AVBufferPtr bufferPtr)
{
    MEDIA_LOG_D("DataPacker PushData...");
    OSAL::ScopedLock lock(mutex_);
    size_ += AudioBufferSize(bufferPtr);
    if (que_.size() == 1) {
        bufferOffset_ = 0;
        dts_ = bufferPtr->dts;
        pts_ = bufferPtr->pts;
    }
    que_.emplace_back(std::move(bufferPtr));
}

bool DataPacker::IsDataAvailable(uint64_t offset, uint32_t size)
{
    MEDIA_LOG_D("DataPacker IsDataAvailable...");
    OSAL::ScopedLock lock(mutex_);
    auto curOffset = bufferOffset_;
    if (que_.empty() || offset < curOffset) {
        return false;
    }
    int bufCnt = que_.size();
    auto offsetEnd = offset + size;
    auto curOffsetEnd = AudioBufferSize(que_.front());
    if (bufCnt == 1) {
        return offsetEnd <= curOffsetEnd;
    }
    auto preOffsetEnd = curOffsetEnd;
    for (auto i = 1; i < bufCnt; ++i) {
        auto bufferOffset = 0;
        if (preOffsetEnd != bufferOffset) {
            return false;
        }
        curOffsetEnd = bufferOffset + AudioBufferSize(que_[i]);
        if (curOffsetEnd >= offsetEnd) {
            return true;
        } else {
            preOffsetEnd = curOffsetEnd;
        }
    }
    return false;
}

// 在调用当前接口前需要先调用IsDataAvailable()
bool DataPacker::PeekRange(uint64_t offset, uint32_t size, AVBufferPtr& bufferPtr)
{
    MEDIA_LOG_D("DataPacker PeekRange(offset, size) = (%" PRIu64 ", %ul)...", offset, size);
    OSAL::ScopedLock lock(mutex_);
    uint8_t* dstPtr = nullptr;
    if (bufferPtr && AudioBufferCapacity(bufferPtr) < size) {
        return false;
    } else {
        assembler_.resize(size);
        dstPtr = assembler_.data();
    }
    auto offsetEnd = offset + size;
    auto curOffsetEnd = AudioBufferSize(que_[0]);
    if (offsetEnd <= curOffsetEnd) {
        dstPtr = AudioBufferWritableData(bufferPtr, size);
        RETURN_FALSE_IF_NULL(dstPtr);
        LOG_WARN_IF_FAIL(memcpy_s(dstPtr, size, AudioBufferReadOnlyData(que_[0]), size), "failed to memcpy");
    } else {
        auto srcSize = AudioBufferSize(que_[0]) - bufferOffset_;
        dstPtr = AudioBufferWritableData(bufferPtr, srcSize);
        RETURN_FALSE_IF_NULL(dstPtr);
        LOG_WARN_IF_FAIL(memcpy_s(dstPtr, srcSize, AudioBufferReadOnlyData(que_[0]) + bufferOffset_, srcSize),
                         "failed to memcpy");
        dstPtr += srcSize;
        size -= srcSize;
        for (size_t i = 1; i < que_.size(); ++i) {
            curOffsetEnd = AudioBufferSize(que_[i]);
            if (curOffsetEnd >= offsetEnd) {
                dstPtr = AudioBufferWritableData(bufferPtr, size);
                RETURN_FALSE_IF_NULL(dstPtr);
                LOG_WARN_IF_FAIL(memcpy_s(dstPtr, size, AudioBufferReadOnlyData(que_[i]), size), "failed to memcpy");
                break;
            } else {
                srcSize = AudioBufferSize(que_[i]);
                dstPtr = AudioBufferWritableData(bufferPtr, srcSize);
                RETURN_FALSE_IF_NULL(dstPtr);
                LOG_WARN_IF_FAIL(memcpy_s(dstPtr, srcSize, AudioBufferReadOnlyData(que_[i]), srcSize),
                                 "failed to memcpy");
                dstPtr += srcSize;
                size -= srcSize;
            }
        }
    }
    if (!bufferPtr) {
        WrapAssemblerBuffer(offset).swap(bufferPtr);
    }
    bufferPtr->pts = pts_;
    bufferPtr->dts = dts_;
    return true;
}

// 单个buffer不足以存储请求的数据，需要合并多个buffer从而组成目标buffer
bool DataPacker::RepackBuffers(uint64_t offset, uint32_t size, AVBufferPtr& bufferPtr)
{
    if (!PeekRange(offset, size, bufferPtr)) {
        return false;
    }
    auto offsetEnd = offset + size;
    size_ -= size;
    while (!que_.empty()) {
        auto& buf = que_.front();
        auto curOffsetEnd = AudioBufferSize(buf);
        if (curOffsetEnd < offsetEnd) {
            que_.pop_front();
            continue;
        } else if (curOffsetEnd == offsetEnd) {
            que_.pop_front();
            bufferOffset_ = 0;
            if (!que_.empty()) {
                dts_ = que_.front()->dts;
                pts_ = que_.front()->pts;
            }
        } else {
            bufferOffset_ = AudioBufferSize(buf) - (curOffsetEnd - offsetEnd);
            dts_ = buf->dts;
            pts_ = buf->pts;
        }
        break;
    }
    return true;
}

// 在调用当前接口前需要先调用IsDataAvailable()
AVBufferPtr DataPacker::GetRange(uint64_t offset, uint32_t size)
{
    MEDIA_LOG_D("DataPacker GetRange(offset, size) = (%" PRIu64 ", %ul)...", offset, size);
    OSAL::ScopedLock lock(mutex_);
    AVBufferPtr bufferPtr = nullptr;
    if (AudioBufferCapacity(que_[0]) < size) {
        RepackBuffers(offset, size, bufferPtr);
        return bufferPtr;
    }
    size_ -= size;

    auto offsetEnd = offset + size;
    auto curOffsetEnd = AudioBufferSize(que_[0]);
    if (offsetEnd < curOffsetEnd) {
        bufferOffset_ = static_cast<uint32_t>(offset);
        bufferPtr = MakeAliasBuffer(que_[0], bufferOffset_, size);
        bufferOffset_ += size;
    } else if (offsetEnd == curOffsetEnd) {
        bufferPtr = que_.front();
        que_.pop_front();
        bufferOffset_ = 0;
        if (!que_.empty()) {
            dts_ = que_.front()->dts;
            pts_ = que_.front()->pts;
        }
    } else {
        bufferPtr = que_.front();
        que_.pop_front();
        auto remainBytes = AudioBufferSize(bufferPtr) - bufferOffset_;

        auto dataPtr = AudioBufferWritableData(bufferPtr, remainBytes);
        if (dataPtr) {
            LOG_WARN_IF_FAIL(memmove_s(dataPtr, remainBytes, dataPtr + bufferOffset_, remainBytes),
                             "failed to memmove");
            dataPtr = AudioBufferWritableData(bufferPtr, size);
            LOG_WARN_IF_FAIL(memcpy_s(dataPtr + remainBytes, size - remainBytes, AudioBufferReadOnlyData(que_.front()),
                             size - remainBytes), "failed to memcpy");
            bufferOffset_ = size - remainBytes;
            dts_ = que_.front()->dts;
            pts_ = que_.front()->pts;
        } else {
            dataPtr = nullptr;
        }
    }
    return bufferPtr;
}

bool DataPacker::GetRange(uint64_t offset, uint32_t size, AVBufferPtr& bufferPtr)
{
    if (bufferPtr) {
        if (!RepackBuffers(offset, size, bufferPtr)) {
            return false;
        }
    } else {
        GetRange(offset, size).swap(bufferPtr);
    }
    return true;
}

AVBufferPtr DataPacker::MakeAliasBuffer(AVBufferPtr bufferPtr, uint32_t offset, uint32_t size)
{
    MEDIA_LOG_D("DataPacker MakeAliasBuffer(offset, size) = (%uld, %ul)...", offset, size);
    AudioBufferWritableData(bufferPtr, size);
    return bufferPtr;
}

AVBufferPtr DataPacker::WrapAssemblerBuffer(uint64_t offset)
{
    MEDIA_LOG_D("DataPacker WrapAssemblerBuffer, offset = %" PRIu64, offset);
    auto bufferPtr = std::make_shared<AVBuffer>();
    auto dataPtr = std::shared_ptr<uint8_t>(assembler_.data(), [this](void* ptr) { assembler_.resize(0); });
    auto bufferData = bufferPtr->WrapMemoryPtr(dataPtr, assembler_.size(), assembler_.size());
    bufferPtr->dts = dts_;
    bufferPtr->pts = pts_;
    return bufferPtr;
}

void DataPacker::Flush()
{
    MEDIA_LOG_D("DataPacker Flush called.");
    OSAL::ScopedLock lock(mutex_);
    que_.clear();
    size_ = 0;
    bufferOffset_ = 0;
    dts_ = 0;
    pts_ = 0;
}
} // namespace Media
} // namespace OHOS
