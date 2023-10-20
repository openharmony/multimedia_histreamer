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
#include <shared_mutex>
#include "avbuffer.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "native_avcodec_base.h"
#include "native_avmagic.h"
#include "native_buffer.h"
#include "surface_buffer.h"


namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "OH_AVBuffer"};
}
typedef struct OH_AVBuffer OH_AVBuffer;
// typedef struct OH_AVBufferQueue OH_AVBufferQueue;
using namespace OHOS::MediaAVCodec;

OH_AVBuffer::OH_AVBuffer(const std::shared_ptr<AVBuffer> &buf)
    : AVObjectMagic(AVMagic::AVCODEC_MAGIC_AVBUFFER), buffer_(buf)
{
}

bool OH_AVBuffer::IsEqualBuffer(const std::shared_ptr<AVBuffer> &buf)
{
    return (buf == buffer_);
}

// OH_AVBufferQueue::OH_AVBufferQueue(const std::shared_ptr<AVBufferQueue> &bufferQueue)
//     : AVObjectMagic(AVMagic::AVCODEC_MAGIC_AVBUFFER_QUEUE), bufferQueue_(bufferQueue)
// {
// }

// class NativeAVBufferQueueCallback : public AVBufferQueueCallback {
// public:
//     NativeAVBufferQueueCallback(OH_AVBufferQueue *bufferQueue, OH_AVBufferQueueCallback callback, void *userData)
//         : bufferQueue_(bufferQueue), callback_(callback), userData_(userData)
//     {
//     }
//     virtual ~NativeAVBufferQueueCallback() = default;

//     void OnBufferAvailable() override
//     {
//         std::unique_lock<std::shared_mutex> lock(mutex_);
//         if (bufferQueue_ != nullptr && callback_.onBufferAvailable != nullptr) {
//             callback_.onBufferAvailable(bufferQueue_, userData_);
//         }
//     }

//     void StopCallback()
//     {
//         std::unique_lock<std::shared_mutex> lock(mutex_);
//         bufferQueue_ = nullptr;
//     }

// private:
//     struct OH_AVBufferQueue *bufferQueue_;
//     struct OH_AVBufferQueueCallback callback_;
//     void *userData_;
//     std::shared_mutex mutex_;
// };

namespace OHOS {
namespace MediaAVCodec {
#ifdef __cplusplus
extern "C" {
#endif
OH_AVBuffer *OH_AVBuffer_Create(int32_t capacity)
{
    CHECK_AND_RETURN_RET_LOG(capacity >= 0, nullptr, "capacity %{public}d is error!", capacity);
    auto allocator = AVAllocatorFactory::CreateSharedAllocator(MemoryFlag::MEMORY_READ_WRITE);
    CHECK_AND_RETURN_RET_LOG(allocator != nullptr, nullptr, "create allocator failed");

    std::shared_ptr<AVBuffer> buffer = AVBuffer::CreateAVBuffer(allocator, capacity);
    CHECK_AND_RETURN_RET_LOG(buffer->memory_ != nullptr, nullptr, "create OH_AVBuffer failed");
    CHECK_AND_RETURN_RET_LOG(buffer->memory_->GetAddr() != nullptr, nullptr, "create OH_AVBuffer failed");

    struct OH_AVBuffer *buf = new (std::nothrow) OH_AVBuffer(buffer);
    CHECK_AND_RETURN_RET_LOG(buf != nullptr, nullptr, "failed to new OH_AVBuffer");
    buf->isUserCreated = true;
    return buf;
}

// OH_AVBuffer *OH_AVBuffer_CreateFromBufferQueue(OH_AVBufferQueue *bufferQueue)
// {
//     CHECK_AND_RETURN_RET_LOG(bufferQueue != nullptr, nullptr, "input bufferQueue is nullptr!");
//     CHECK_AND_RETURN_RET_LOG(bufferQueue->magic_ == AVMagic::AVCODEC_MAGIC_AVBUFFER_QUEUE, nullptr, "magic error!");

//     std::shared_ptr<AVBuffer> buffer = bufferQueue->bufferQueue_->RequestBuffer(-1);
//     struct OH_AVBuffer *buf = new (std::nothrow) OH_AVBuffer(buffer);
//     CHECK_AND_RETURN_RET_LOG(buf != nullptr, nullptr, "failed to new OH_AVBuffer");
//     buf->isUserCreated = true;
//     return buf;
// }

uint8_t *OH_AVBuffer_GetAddr(OH_AVBuffer *buf)
{
    CHECK_AND_RETURN_RET_LOG(buf != nullptr, nullptr, "input buf is nullptr!");
    CHECK_AND_RETURN_RET_LOG(buf->magic_ == AVMagic::AVCODEC_MAGIC_AVBUFFER, nullptr, "magic error!");
    CHECK_AND_RETURN_RET_LOG(buf->buffer_ != nullptr, nullptr, "buffer is nullptr!");
    CHECK_AND_RETURN_RET_LOG(buf->buffer_->memory_ != nullptr, nullptr, "buffer's memory is nullptr!");
    return buf->buffer_->memory_->GetAddr();
}

int32_t OH_AVBuffer_GetCapacity(OH_AVBuffer *buf)
{
    CHECK_AND_RETURN_RET_LOG(buf != nullptr, -1, "input buf is nullptr!");
    CHECK_AND_RETURN_RET_LOG(buf->magic_ == AVMagic::AVCODEC_MAGIC_AVBUFFER, -1, "magic error!");
    CHECK_AND_RETURN_RET_LOG(buf->buffer_ != nullptr, -1, "buffer is nullptr!");
    CHECK_AND_RETURN_RET_LOG(buf->buffer_->memory_ != nullptr, -1, "buffer's memory is nullptr!");
    return buf->buffer_->memory_->GetCapacity();
}

OH_AVCodecBufferAttr OH_AVBuffer_GetBufferAttr(OH_AVBuffer *buf)
{
    OH_AVCodecBufferAttr attr;
    CHECK_AND_RETURN_RET_LOG(buf != nullptr, attr, "input buf is nullptr!");
    CHECK_AND_RETURN_RET_LOG(buf->magic_ == AVMagic::AVCODEC_MAGIC_AVBUFFER, attr, "magic error!");
    CHECK_AND_RETURN_RET_LOG(buf->buffer_ != nullptr, attr, "buffer is nullptr!");
    CHECK_AND_RETURN_RET_LOG(buf->buffer_->memory_ != nullptr, attr, "buffer's memory is nullptr!");
    attr.pts = buf->buffer_->pts_;
    attr.offset = buf->buffer_->memory_->GetOffset();
    attr.size = buf->buffer_->memory_->GetSize();
    attr.flags = static_cast<uint32_t>(buf->buffer_->flag_);
    return attr;
}

OH_AVErrCode OH_AVBuffer_SetBufferAttr(OH_AVBuffer *buf, OH_AVCodecBufferAttr *attr)
{
    CHECK_AND_RETURN_RET_LOG(buf != nullptr, AV_ERR_INVALID_VAL, "input buf is nullptr!");
    CHECK_AND_RETURN_RET_LOG(buf->magic_ == AVMagic::AVCODEC_MAGIC_AVBUFFER, AV_ERR_INVALID_VAL, "magic error!");
    CHECK_AND_RETURN_RET_LOG(buf->buffer_ != nullptr, AV_ERR_INVALID_VAL, "buffer is nullptr!");
    CHECK_AND_RETURN_RET_LOG(buf->buffer_->memory_ != nullptr, AV_ERR_INVALID_VAL, "buffer's memory is nullptr!");
    buf->buffer_->pts_ = attr->pts;
    buf->buffer_->memory_->SetOffset(attr->offset);
    buf->buffer_->flag_ = static_cast<AVCodecBufferFlag>(attr->flags);
    buf->buffer_->memory_->SetSize(attr->size);
    return AV_ERR_OK;
}

OH_AVFormat *OH_AVBuffer_GetParameter(OH_AVBuffer *buf)
{
    CHECK_AND_RETURN_RET_LOG(buf != nullptr, nullptr, "input buf is nullptr!");
    CHECK_AND_RETURN_RET_LOG(buf->magic_ == AVMagic::AVCODEC_MAGIC_AVBUFFER, nullptr, "magic error!");
    CHECK_AND_RETURN_RET_LOG(buf->buffer_ != nullptr, nullptr, "buffer is nullptr!");
    CHECK_AND_RETURN_RET_LOG(buf->buffer_->meta_ != nullptr, nullptr, "buffer's meta is nullptr!");

    OH_AVFormat *avFormat = OH_AVFormat_Create();
    avFormat->format_ = *(buf->buffer_->meta_);

    return avFormat;
}

OH_AVErrCode OH_AVBuffer_SetParameter(OH_AVBuffer *buf, OH_AVFormat *format)
{
    CHECK_AND_RETURN_RET_LOG(buf != nullptr, AV_ERR_INVALID_VAL, "input buf is nullptr!");
    CHECK_AND_RETURN_RET_LOG(buf->magic_ == AVMagic::AVCODEC_MAGIC_AVBUFFER, AV_ERR_INVALID_VAL, "magic error!");
    CHECK_AND_RETURN_RET_LOG(buf->buffer_ != nullptr, AV_ERR_INVALID_VAL, "buffer is nullptr!");
    *(buf->buffer_->meta_ )= format->format_;
    return AV_ERR_OK;
}

OH_NativeBuffer *OH_AVBuffer_GetNativeBuffer(OH_AVBuffer *buf)
{
    CHECK_AND_RETURN_RET_LOG(buf != nullptr, nullptr, "input buf is nullptr!");
    CHECK_AND_RETURN_RET_LOG(buf->magic_ == AVMagic::AVCODEC_MAGIC_AVBUFFER, nullptr, "magic error!");
    CHECK_AND_RETURN_RET_LOG(buf->buffer_ != nullptr, nullptr, "buffer is nullptr!");
    CHECK_AND_RETURN_RET_LOG(buf->buffer_->memory_ != nullptr, nullptr, "buffer's memory is nullptr!");
    sptr<SurfaceBuffer> surfaceBuffer = buf->buffer_->memory_->GetSurfaceBuffer();
    CHECK_AND_RETURN_RET_LOG(surfaceBuffer != nullptr, nullptr, "surface buffer is nullptr!");
    return surfaceBuffer->SurfaceBufferToNativeBuffer();
}

OH_AVErrCode OH_AVBuffer_Destroy(struct OH_AVBuffer *buf)
{
    CHECK_AND_RETURN_RET_LOG(buf != nullptr, AV_ERR_INVALID_VAL, "input buf is nullptr!");
    CHECK_AND_RETURN_RET_LOG(buf->magic_ == AVMagic::AVCODEC_MAGIC_AVBUFFER, AV_ERR_INVALID_VAL, "magic error!");
    CHECK_AND_RETURN_RET_LOG(buf->isUserCreated, AV_ERR_INVALID_VAL, "input buf is not user created!");
    delete buf;
    return AV_ERR_OK;
}

// OH_AVErrCode OH_AVBufferQueue_SetProducerCallback(OH_AVBufferQueue *bufferQueue, OH_AVBufferQueueCallback callback,
//                                                   void *userData)
// {
//     CHECK_AND_RETURN_RET_LOG(bufferQueue != nullptr, AV_ERR_INVALID_VAL, "input bufferQueue is nullptr!");
//     CHECK_AND_RETURN_RET_LOG(bufferQueue->bufferQueue_ != nullptr, AV_ERR_INVALID_VAL, "bufferQueue_ is nullptr!");
//     CHECK_AND_RETURN_RET_LOG(bufferQueue->magic_ == AVMagic::AVCODEC_MAGIC_AVBUFFER_QUEUE, AV_ERR_INVALID_VAL,
//                              "magic error!");
//     CHECK_AND_RETURN_RET_LOG(callback.onBufferAvailable != nullptr, AV_ERR_INVALID_VAL,
//                              "Callback onBufferAvailable is nullptr");

//     bufferQueue->callback_ = std::make_shared<NativeAVBufferQueueCallback>(bufferQueue, callback, userData);
//     int32_t ret = bufferQueue->bufferQueue_->SetProducerCallback(bufferQueue->callback_);
//     CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCSErrorToOHAVErrCode(static_cast<AVCodecServiceErrCode>(ret)),
//                              "audioDecoder SetCallback failed!");
//     return AV_ERR_OK;
// }

// OH_AVErrCode OH_AVBufferQueue_PushBuffer(OH_AVBufferQueue *bufferQueue, OH_AVBuffer *buf)
// {
//     CHECK_AND_RETURN_RET_LOG(bufferQueue != nullptr, AV_ERR_INVALID_VAL, "bufferQueue is nullptr!");
//     CHECK_AND_RETURN_RET_LOG(bufferQueue->bufferQueue_ != nullptr, AV_ERR_INVALID_VAL, "bufferQueue_ is nullptr!");
//     CHECK_AND_RETURN_RET_LOG(bufferQueue->magic_ == AVMagic::AVCODEC_MAGIC_AVBUFFER_QUEUE, AV_ERR_INVALID_VAL,
//                              "magic error!");

//     CHECK_AND_RETURN_RET_LOG(buf != nullptr, AV_ERR_INVALID_VAL, "input buf is nullptr!");
//     CHECK_AND_RETURN_RET_LOG(buf->magic_ == AVMagic::AVCODEC_MAGIC_AVBUFFER, AV_ERR_INVALID_VAL, "magic error!");
//     CHECK_AND_RETURN_RET_LOG(buf->buffer_ != nullptr, AV_ERR_INVALID_VAL, "buffer is nullptr!");
//     CHECK_AND_RETURN_RET_LOG(buf->buffer_->memory_ != nullptr, AV_ERR_INVALID_VAL, "buffer's memory is nullptr!");
//     auto buffer = buf->buffer_;
//     CHECK_AND_RETURN_RET_LOG(buf->isUserCreated, AV_ERR_INVALID_VAL, "input buf is not user created!");
//     delete buf;

//     int32_t ret = bufferQueue->bufferQueue_->PushBuffer(buffer);
//     CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCSErrorToOHAVErrCode(static_cast<AVCodecServiceErrCode>(ret)),
//                              "audioDecoder SetCallback failed!");
//     return AV_ERR_OK;
// }

// OH_AVBuffer *OH_AVBufferQueue_RequestBuffer(OH_AVBufferQueue *bufferQueue, int32_t capacity)
// {
//     CHECK_AND_RETURN_RET_LOG(bufferQueue != nullptr, nullptr, "input bufferQueue is nullptr!");
//     CHECK_AND_RETURN_RET_LOG(bufferQueue->bufferQueue_ != nullptr, nullptr, "bufferQueue_ is nullptr!");
//     CHECK_AND_RETURN_RET_LOG(bufferQueue->magic_ == AVMagic::AVCODEC_MAGIC_AVBUFFER_QUEUE, nullptr,
//                              "magic error!");
//     CHECK_AND_RETURN_RET_LOG(capacity > 0, nullptr, "capacity is invalid!");

//     std::shared_ptr<AVBuffer> buffer = bufferQueue->bufferQueue_->RequestBuffer(capacity);
//     struct OH_AVBuffer *buf = new (std::nothrow) OH_AVBuffer(buffer);
//     CHECK_AND_RETURN_RET_LOG(buf != nullptr, nullptr, "failed to new OH_AVBuffer");
//     buf->isUserCreated = true;
//     return buf;
// }
#ifdef __cplusplus
};
#endif
} // namespace MediaAVCodec
} // namespace OHOS