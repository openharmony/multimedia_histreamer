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

#include "native_avbuffer.h"
#include <shared_mutex>
#include "buffer/avbuffer.h"
#include "common/log.h"
#include "common/status.h"
#include "native_mfmagic.h"
#include "surface_buffer.h"

using namespace OHOS;
using namespace OHOS::Media;

OH_AVBuffer::OH_AVBuffer(const std::shared_ptr<OHOS::Media::AVBuffer> &buffer)
    : MFObjectMagic(MFMagic::MFMAGIC_AVBUFFER), buffer_(buffer)
{
}

bool OH_AVBuffer::IsEqualBuffer(const std::shared_ptr<OHOS::Media::AVBuffer> &buffer)
{
    return (buffer == buffer_);
}

OH_AVBuffer *OH_AVBuffer_Create(int32_t capacity)
{
    FALSE_RETURN_V_MSG_E(capacity >= 0, nullptr, "capacity %{public}d is error!", capacity);
    auto allocator = AVAllocatorFactory::CreateSharedAllocator(MemoryFlag::MEMORY_READ_WRITE);
    FALSE_RETURN_V_MSG_E(allocator != nullptr, nullptr, "create allocator failed");

    std::shared_ptr<AVBuffer> buffer = AVBuffer::CreateAVBuffer(allocator, capacity);
    FALSE_RETURN_V_MSG_E(buffer->memory_ != nullptr, nullptr, "create OH_AVBuffer failed");
    FALSE_RETURN_V_MSG_E(buffer->memory_->GetAddr() != nullptr, nullptr, "create OH_AVBuffer failed");

    struct OH_AVBuffer *buf = new (std::nothrow) OH_AVBuffer(buffer);
    FALSE_RETURN_V_MSG_E(buffer != nullptr, nullptr, "failed to new OH_AVBuffer");
    buf->isUserCreated = true;
    return buf;
}

OH_AVErrCode OH_AVBuffer_Destroy(struct OH_AVBuffer *buffer)
{
    FALSE_RETURN_V_MSG_E(buffer != nullptr, AV_ERR_INVALID_STATE, "input buffer is nullptr!");
    FALSE_RETURN_V_MSG_E(buffer->magic_ == MFMagic::MFMAGIC_AVBUFFER, AV_ERR_INVALID_VAL, "magic error!");
    FALSE_RETURN_V_MSG_E(buffer->isUserCreated, AV_ERR_INVALID_VAL, "input buffer is not user created!");
    delete buffer;
    return AV_ERR_OK;
}

OH_AVBufferAttr OH_AVBuffer_GetBufferAttr(OH_AVBuffer *buffer)
{
    OH_AVBufferAttr attr;
    FALSE_RETURN_V_MSG_E(buffer != nullptr, attr, "input buffer is nullptr!");
    FALSE_RETURN_V_MSG_E(buffer->magic_ == MFMagic::MFMAGIC_AVBUFFER, attr, "magic error!");
    FALSE_RETURN_V_MSG_E(buffer->buffer_ != nullptr, attr, "buffer is nullptr!");
    attr.pts = buffer->buffer_->pts_;
    attr.flags = static_cast<uint32_t>(buffer->buffer_->flag_);
    if (buffer->buffer_->memory_ != nullptr) {
        attr.offset = buffer->buffer_->memory_->GetOffset();
        attr.size = buffer->buffer_->memory_->GetSize();
    } else {
        attr.offset = 0;
        attr.size = 0;
    }
    return attr;
}

OH_AVErrCode OH_AVBuffer_SetBufferAttr(OH_AVBuffer *buffer, OH_AVBufferAttr *attr)
{
    FALSE_RETURN_V_MSG_E(buffer != nullptr, AV_ERR_INVALID_VAL, "input buffer is nullptr!");
    FALSE_RETURN_V_MSG_E(buffer->magic_ == MFMagic::MFMAGIC_AVBUFFER, AV_ERR_INVALID_VAL, "magic error!");
    FALSE_RETURN_V_MSG_E(buffer->buffer_ != nullptr, AV_ERR_INVALID_VAL, "buffer is nullptr!");
    buffer->buffer_->pts_ = attr->pts;
    buffer->buffer_->flag_ = attr->flags;
    if (buffer->buffer_->memory_ != nullptr) {
        buffer->buffer_->memory_->SetSize(attr->size);
        buffer->buffer_->memory_->SetOffset(attr->offset);
    }
    return AV_ERR_OK;
}

OH_AVFormat *OH_AVBuffer_GetParameter(OH_AVBuffer *buffer)
{
    FALSE_RETURN_V_MSG_E(buffer != nullptr, nullptr, "input buffer is nullptr!");
    FALSE_RETURN_V_MSG_E(buffer->magic_ == MFMagic::MFMAGIC_AVBUFFER, nullptr, "magic error!");
    FALSE_RETURN_V_MSG_E(buffer->buffer_ != nullptr, nullptr, "buffer is nullptr!");
    FALSE_RETURN_V_MSG_E(buffer->buffer_->meta_ != nullptr, nullptr, "buffer's meta is nullptr!");

    return nullptr;
}

OH_AVErrCode OH_AVBuffer_SetParameter(OH_AVBuffer *buffer, OH_AVFormat *format)
{
    FALSE_RETURN_V_MSG_E(buffer != nullptr, AV_ERR_INVALID_VAL, "input buffer is nullptr!");
    FALSE_RETURN_V_MSG_E(buffer->magic_ == MFMagic::MFMAGIC_AVBUFFER, AV_ERR_INVALID_VAL, "magic error!");
    FALSE_RETURN_V_MSG_E(buffer->buffer_ != nullptr, AV_ERR_INVALID_VAL, "buffer is nullptr!");

    return AV_ERR_OK;
}

uint8_t *OH_AVBuffer_GetAddr(OH_AVBuffer *buffer)
{
    FALSE_RETURN_V_MSG_E(buffer != nullptr, nullptr, "input buffer is nullptr!");
    FALSE_RETURN_V_MSG_E(buffer->magic_ == MFMagic::MFMAGIC_AVBUFFER, nullptr, "magic error!");
    FALSE_RETURN_V_MSG_E(buffer->buffer_ != nullptr, nullptr, "buffer is nullptr!");
    FALSE_RETURN_V_MSG_E(buffer->buffer_->memory_ != nullptr, nullptr, "buffer's memory is nullptr!");
    return buffer->buffer_->memory_->GetAddr();
}

int32_t OH_AVBuffer_GetCapacity(OH_AVBuffer *buffer)
{
    FALSE_RETURN_V_MSG_E(buffer != nullptr, -1, "input buffer is nullptr!");
    FALSE_RETURN_V_MSG_E(buffer->magic_ == MFMagic::MFMAGIC_AVBUFFER, -1, "magic error!");
    FALSE_RETURN_V_MSG_E(buffer->buffer_ != nullptr, -1, "buffer is nullptr!");
    FALSE_RETURN_V_MSG_E(buffer->buffer_->memory_ != nullptr, -1, "buffer's memory is nullptr!");
    return buffer->buffer_->memory_->GetCapacity();
}

OH_NativeBuffer *OH_AVBuffer_GetNativeBuffer(OH_AVBuffer *buffer)
{
    FALSE_RETURN_V_MSG_E(buffer != nullptr, nullptr, "input buffer is nullptr!");
    FALSE_RETURN_V_MSG_E(buffer->magic_ == MFMagic::MFMAGIC_AVBUFFER, nullptr, "magic error!");
    FALSE_RETURN_V_MSG_E(buffer->buffer_ != nullptr, nullptr, "buffer is nullptr!");
    FALSE_RETURN_V_MSG_E(buffer->buffer_->memory_ != nullptr, nullptr, "buffer's memory is nullptr!");
    sptr<SurfaceBuffer> surfaceBuffer = buffer->buffer_->memory_->GetSurfaceBuffer();
    FALSE_RETURN_V_MSG_E(surfaceBuffer != nullptr, nullptr, "surfaceBuffer is nullptr!");
    return surfaceBuffer->SurfaceBufferToNativeBufffer();
}