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

#include <mutex>
#include "inner_api/buffer/avbuffer.h"
#include "inner_api/common/log.h"
#include "inner_api/common/status.h"
#include "native_avmagic.h"
#include "native_buffer.h"
#include "surface_buffer.h"
#include <shared_mutex>

typedef struct OH_AVBuffer OH_AVBuffer;

OH_AVBuffer::OH_AVBuffer(const std::shared_ptr<OHOS::Media::AVBuffer> &buf)
    : AVObjectMagic(AVMagic::AVCODEC_MAGIC_AVBUFFER), buffer_(buf)
{
}

bool OH_AVBuffer::IsEqualBuffer(const std::shared_ptr<OHOS::Media::AVBuffer> &buf)
{
    return (buf == buffer_);
}

namespace OHOS {
namespace Media {
#ifdef __cplusplus
extern "C" {
#endif
OH_AVBuffer *OH_AVBuffer_Create(int32_t capacity)
{
    FALSE_RETURN_V_MSG_E(capacity >= 0, nullptr, "capacity %{public}d is error!", capacity);
    auto allocator = AVAllocatorFactory::CreateSharedAllocator(MemoryFlag::MEMORY_READ_WRITE);
    FALSE_RETURN_V_MSG_E(allocator != nullptr, nullptr, "create allocator failed");

    std::shared_ptr<AVBuffer> buffer = AVBuffer::CreateAVBuffer(allocator, capacity);
    FALSE_RETURN_V_MSG_E(buffer->memory_ != nullptr, nullptr, "create OH_AVBuffer failed");
    FALSE_RETURN_V_MSG_E(buffer->memory_->GetAddr() != nullptr, nullptr, "create OH_AVBuffer failed");

    struct OH_AVBuffer *buf = new (std::nothrow) OH_AVBuffer(buffer);
    FALSE_RETURN_V_MSG_E(buf != nullptr, nullptr, "failed to new OH_AVBuffer");
    buf->isUserCreated = true;
    return buf;
}

// OH_AVBuffer *OH_AVBuffer_CreateFromBufferQueue(OH_AVBufferQueue *bufferQueue)
// {
//     FALSE_RETURN_V_MSG_E(bufferQueue != nullptr, nullptr, "input bufferQueue is nullptr!");
//     FALSE_RETURN_V_MSG_E(bufferQueue->magic_ == AVMagic::AVCODEC_MAGIC_AVBUFFER_QUEUE, nullptr, "magic error!");

//     std::shared_ptr<AVBuffer> buffer = bufferQueue->bufferQueue_->RequestBuffer(-1);
//     struct OH_AVBuffer *buf = new (std::nothrow) OH_AVBuffer(buffer);
//     FALSE_RETURN_V_MSG_E(buf != nullptr, nullptr, "failed to new OH_AVBuffer");
//     buf->isUserCreated = true;
//     return buf;
// }

uint8_t *OH_AVBuffer_GetAddr(OH_AVBuffer *buf)
{
    FALSE_RETURN_V_MSG_E(buf != nullptr, nullptr, "input buf is nullptr!");
    FALSE_RETURN_V_MSG_E(buf->magic_ == AVMagic::AVCODEC_MAGIC_AVBUFFER, nullptr, "magic error!");
    FALSE_RETURN_V_MSG_E(buf->buffer_ != nullptr, nullptr, "buffer is nullptr!");
    FALSE_RETURN_V_MSG_E(buf->buffer_->memory_ != nullptr, nullptr, "buffer's memory is nullptr!");
    return buf->buffer_->memory_->GetAddr();
}

int32_t OH_AVBuffer_GetCapacity(OH_AVBuffer *buf)
{
    FALSE_RETURN_V_MSG_E(buf != nullptr, -1, "input buf is nullptr!");
    FALSE_RETURN_V_MSG_E(buf->magic_ == AVMagic::AVCODEC_MAGIC_AVBUFFER, -1, "magic error!");
    FALSE_RETURN_V_MSG_E(buf->buffer_ != nullptr, -1, "buffer is nullptr!");
    FALSE_RETURN_V_MSG_E(buf->buffer_->memory_ != nullptr, -1, "buffer's memory is nullptr!");
    return buf->buffer_->memory_->GetCapacity();
}

OH_AVCodecBufferAttr OH_AVBuffer_GetBufferAttr(OH_AVBuffer *buf)
{
    OH_AVCodecBufferAttr attr;
    FALSE_RETURN_V_MSG_E(buf != nullptr, attr, "input buf is nullptr!");
    FALSE_RETURN_V_MSG_E(buf->magic_ == AVMagic::AVCODEC_MAGIC_AVBUFFER, attr, "magic error!");
    FALSE_RETURN_V_MSG_E(buf->buffer_ != nullptr, attr, "buffer is nullptr!");
    FALSE_RETURN_V_MSG_E(buf->buffer_->memory_ != nullptr, attr, "buffer's memory is nullptr!");
    attr.pts = buf->buffer_->pts_;
    attr.offset = buf->buffer_->memory_->GetOffset();
    attr.size = buf->buffer_->memory_->GetSize();
    attr.flags = static_cast<uint32_t>(buf->buffer_->flag_);
    return attr;
}

OH_AVErrCode OH_AVBuffer_SetBufferAttr(OH_AVBuffer *buf, OH_AVCodecBufferAttr *attr)
{
    FALSE_RETURN_V_MSG_E(buf != nullptr, static_cast<int32_t>(Status::ERROR_INVALID_PARAMETER),
                         "input buf is nullptr!");
    FALSE_RETURN_V_MSG_E(buf->magic_ == AVMagic::AVCODEC_MAGIC_AVBUFFER,
                         static_cast<int32_t>(Status::ERROR_INVALID_PARAMETER), "magic error!");
    FALSE_RETURN_V_MSG_E(buf->buffer_ != nullptr, static_cast<int32_t>(Status::ERROR_INVALID_PARAMETER),
                         "buffer is nullptr!");
    FALSE_RETURN_V_MSG_E(buf->buffer_->memory_ != nullptr, static_cast<int32_t>(Status::ERROR_INVALID_PARAMETER),
                         "buffer's memory is nullptr!");
    buf->buffer_->pts_ = attr->pts;
    buf->buffer_->memory_->SetOffset(attr->offset);
    buf->buffer_->flag_ = static_cast<AVCodecBufferFlag>(attr->flags);
    buf->buffer_->memory_->SetSize(attr->size);
    return static_cast<int32_t>(Status::OK);
}

OH_AVFormat *OH_AVBuffer_GetParameter(OH_AVBuffer *buf)
{
    FALSE_RETURN_V_MSG_E(buf != nullptr, nullptr, "input buf is nullptr!");
    FALSE_RETURN_V_MSG_E(buf->magic_ == AVMagic::AVCODEC_MAGIC_AVBUFFER, nullptr, "magic error!");
    FALSE_RETURN_V_MSG_E(buf->buffer_ != nullptr, nullptr, "buffer is nullptr!");
    FALSE_RETURN_V_MSG_E(buf->buffer_->meta_ != nullptr, nullptr, "buffer's meta is nullptr!");

    OH_AVFormat *avFormat = OH_AVFormat_Create();
    avFormat->format_ = *(buf->buffer_->meta_);

    return avFormat;
}

OH_AVErrCode OH_AVBuffer_SetParameter(OH_AVBuffer *buf, OH_AVFormat *format)
{
    FALSE_RETURN_V_MSG_E(buf != nullptr, static_cast<int32_t>(Status::ERROR_INVALID_PARAMETER),
                         "input buf is nullptr!");
    FALSE_RETURN_V_MSG_E(buf->magic_ == AVMagic::AVCODEC_MAGIC_AVBUFFER,
                         static_cast<int32_t>(Status::ERROR_INVALID_PARAMETER), "magic error!");
    FALSE_RETURN_V_MSG_E(buf->buffer_ != nullptr, static_cast<int32_t>(Status::ERROR_INVALID_PARAMETER),
                         "buffer is nullptr!");
    *(buf->buffer_->meta_) = format->format_;
    return static_cast<int32_t>(Status::OK);
}

OH_NativeBuffer *OH_AVBuffer_GetNativeBuffer(OH_AVBuffer *buf)
{
    FALSE_RETURN_V_MSG_E(buf != nullptr, nullptr, "input buf is nullptr!");
    FALSE_RETURN_V_MSG_E(buf->magic_ == AVMagic::AVCODEC_MAGIC_AVBUFFER, nullptr, "magic error!");
    FALSE_RETURN_V_MSG_E(buf->buffer_ != nullptr, nullptr, "buffer is nullptr!");
    FALSE_RETURN_V_MSG_E(buf->buffer_->memory_ != nullptr, nullptr, "buffer's memory is nullptr!");
    sptr<SurfaceBuffer> surfaceBuffer = buf->buffer_->memory_->GetSurfaceBuffer();
    FALSE_RETURN_V_MSG_E(surfaceBuffer != nullptr, nullptr, "surface buffer is nullptr!");
    return surfaceBuffer->SurfaceBufferToNativeBuffer();
}

OH_AVErrCode OH_AVBuffer_Destroy(struct OH_AVBuffer *buf)
{
    FALSE_RETURN_V_MSG_E(buf != nullptr, static_cast<int32_t>(Status::ERROR_INVALID_PARAMETER),
                         "input buf is nullptr!");
    FALSE_RETURN_V_MSG_E(buf->magic_ == AVMagic::AVCODEC_MAGIC_AVBUFFER,
                         static_cast<int32_t>(Status::ERROR_INVALID_PARAMETER), "magic error!");
    FALSE_RETURN_V_MSG_E(buf->isUserCreated, static_cast<int32_t>(Status::ERROR_INVALID_PARAMETER),
                         "input buf is not user created!");
    delete buf;
    return static_cast<int32_t>(Status::OK);
}

// OH_AVErrCode OH_AVBufferQueue_SetProducerCallback(OH_AVBufferQueue *bufferQueue, OH_AVBufferQueueCallback callback,
//                                                   void *userData)
// {
//     FALSE_RETURN_V_MSG_E(bufferQueue != nullptr, static_cast<int32_t>(Status::ERROR_INVALID_PARAMETER), "input
//     bufferQueue is nullptr!"); FALSE_RETURN_V_MSG_E(bufferQueue->bufferQueue_ != nullptr,
//     static_cast<int32_t>(Status::ERROR_INVALID_PARAMETER), "bufferQueue_ is nullptr!");
//     FALSE_RETURN_V_MSG_E(bufferQueue->magic_ == AVMagic::AVCODEC_MAGIC_AVBUFFER_QUEUE,
//     static_cast<int32_t>(Status::ERROR_INVALID_PARAMETER),
//                              "magic error!");
//     FALSE_RETURN_V_MSG_E(callback.onBufferAvailable != nullptr,
//     static_cast<int32_t>(Status::ERROR_INVALID_PARAMETER),
//                              "Callback onBufferAvailable is nullptr");

//     bufferQueue->callback_ = std::make_shared<NativeAVBufferQueueCallback>(bufferQueue, callback, userData);
//     int32_t ret = bufferQueue->bufferQueue_->SetProducerCallback(bufferQueue->callback_);
//     FALSE_RETURN_V_MSG_E(ret == static_cast<int32_t>(Status::OK),
//     AVCSErrorToOHAVErrCode(static_cast<AVCodecServiceErrCode>(ret)),
//                              "audioDecoder SetCallback failed!");
//     return static_cast<int32_t>(Status::OK);
// }

// OH_AVErrCode OH_AVBufferQueue_PushBuffer(OH_AVBufferQueue *bufferQueue, OH_AVBuffer *buf)
// {
//     FALSE_RETURN_V_MSG_E(bufferQueue != nullptr, static_cast<int32_t>(Status::ERROR_INVALID_PARAMETER), "bufferQueue
//     is nullptr!"); FALSE_RETURN_V_MSG_E(bufferQueue->bufferQueue_ != nullptr,
//     static_cast<int32_t>(Status::ERROR_INVALID_PARAMETER), "bufferQueue_ is nullptr!");
//     FALSE_RETURN_V_MSG_E(bufferQueue->magic_ == AVMagic::AVCODEC_MAGIC_AVBUFFER_QUEUE,
//     static_cast<int32_t>(Status::ERROR_INVALID_PARAMETER),
//                              "magic error!");

//     FALSE_RETURN_V_MSG_E(buf != nullptr, static_cast<int32_t>(Status::ERROR_INVALID_PARAMETER), "input buf is
//     nullptr!"); FALSE_RETURN_V_MSG_E(buf->magic_ == AVMagic::AVCODEC_MAGIC_AVBUFFER,
//     static_cast<int32_t>(Status::ERROR_INVALID_PARAMETER), "magic error!"); FALSE_RETURN_V_MSG_E(buf->buffer_ !=
//     nullptr, static_cast<int32_t>(Status::ERROR_INVALID_PARAMETER), "buffer is nullptr!");
//     FALSE_RETURN_V_MSG_E(buf->buffer_->memory_ != nullptr, static_cast<int32_t>(Status::ERROR_INVALID_PARAMETER),
//     "buffer's memory is nullptr!"); auto buffer = buf->buffer_; FALSE_RETURN_V_MSG_E(buf->isUserCreated,
//     static_cast<int32_t>(Status::ERROR_INVALID_PARAMETER), "input buf is not user created!"); delete buf;

//     int32_t ret = bufferQueue->bufferQueue_->PushBuffer(buffer);
//     FALSE_RETURN_V_MSG_E(ret == static_cast<int32_t>(Status::OK),
//     AVCSErrorToOHAVErrCode(static_cast<AVCodecServiceErrCode>(ret)),
//                              "audioDecoder SetCallback failed!");
//     return static_cast<int32_t>(Status::OK);
// }

// OH_AVBuffer *OH_AVBufferQueue_RequestBuffer(OH_AVBufferQueue *bufferQueue, int32_t capacity)
// {
//     FALSE_RETURN_V_MSG_E(bufferQueue != nullptr, nullptr, "input bufferQueue is nullptr!");
//     FALSE_RETURN_V_MSG_E(bufferQueue->bufferQueue_ != nullptr, nullptr, "bufferQueue_ is nullptr!");
//     FALSE_RETURN_V_MSG_E(bufferQueue->magic_ == AVMagic::AVCODEC_MAGIC_AVBUFFER_QUEUE, nullptr,
//                              "magic error!");
//     FALSE_RETURN_V_MSG_E(capacity > 0, nullptr, "capacity is invalid!");

//     std::shared_ptr<AVBuffer> buffer = bufferQueue->bufferQueue_->RequestBuffer(capacity);
//     struct OH_AVBuffer *buf = new (std::nothrow) OH_AVBuffer(buffer);
//     FALSE_RETURN_V_MSG_E(buf != nullptr, nullptr, "failed to new OH_AVBuffer");
//     buf->isUserCreated = true;
//     return buf;
// }
#ifdef __cplusplus
};
#endif
} // namespace Media
} // namespace OHOS