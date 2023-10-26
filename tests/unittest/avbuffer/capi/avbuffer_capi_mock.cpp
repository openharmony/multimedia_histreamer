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

#include "avbuffer_capi_mock.h"
#include <gtest/gtest.h>
#include "avformat_capi_mock.h"
#include "native_avbuffer.h"
#include "native_averrors.h"
#include "surface_buffer.h"
#include "unittest_log.h"

namespace OHOS {
namespace MediaAVCodec {
// std::mutex AVBufferQueueCapiMock::mutex_;
// std::map<OH_AVBufferQueue *, std::shared_ptr<AVBufferQueueCallbackMock>> AVBufferQueueCapiMock::mockCbMap_;

uint8_t *AVBufferCapiMock::GetAddr()
{
    UNITTEST_CHECK_AND_RETURN_RET_LOG(buffer_ != nullptr, nullptr, "buffer_ is nullptr!");
    return OH_AVBuffer_GetAddr(buffer_);
}

int32_t AVBufferCapiMock::GetCapacity()
{
    UNITTEST_CHECK_AND_RETURN_RET_LOG(buffer_ != nullptr, 0, "buffer_ is nullptr!");
    return OH_AVBuffer_GetCapacity(buffer_);
}

OH_AVCodecBufferAttr AVBufferCapiMock::GetBufferAttr()
{
    OH_AVCodecBufferAttr attr;
    UNITTEST_CHECK_AND_RETURN_RET_LOG(buffer_ != nullptr, attr, "buffer_ is nullptr!");
    return OH_AVBuffer_GetBufferAttr(buffer_);
}
int32_t AVBufferCapiMock::SetBufferAttr(OH_AVCodecBufferAttr &attr)
{
    UNITTEST_CHECK_AND_RETURN_RET_LOG(buffer_ != nullptr, static_cast<int32_t>(Status::ERROR_UNKNOWN),
                                      "buffer_ is nullptr!");
    return OH_AVBuffer_SetBufferAttr(buffer_, &attr);
}

std::shared_ptr<FormatMock> AVBufferCapiMock::GetParameter()
{
    UNITTEST_CHECK_AND_RETURN_RET_LOG(buffer_ != nullptr, nullptr, "buffer_ is nullptr!");
    OH_AVFormat *format = OH_AVBuffer_GetParameter(buffer_);
    EXPECT_NE(format, nullptr);
    auto formatMock = std::make_shared<AVFormatCapiMock>(format);
    return formatMock;
}

int32_t AVBufferCapiMock::SetParameter(const std::shared_ptr<FormatMock> &format)
{
    UNITTEST_CHECK_AND_RETURN_RET_LOG(buffer_ != nullptr, static_cast<int32_t>(Status::ERROR_UNKNOWN),
                                      "buffer_ is nullptr!");
    return OH_AVBuffer_SetParameter(buffer_, std::static_pointer_cast<AVFormatCapiMock>(format)->GetFormat());
}

sptr<SurfaceBuffer> AVBufferCapiMock::GetNativeBuffer()
{
    UNITTEST_CHECK_AND_RETURN_RET_LOG(buffer_ != nullptr, nullptr, "buffer_ is nullptr!");
    OH_NativeBuffer *surfaceBuffer = OH_AVBuffer_GetNativeBuffer(buffer_);
    UNITTEST_CHECK_AND_RETURN_RET_LOG(surfaceBuffer != nullptr, nullptr, "surfaceBuffer is nullptr!");
    return sptr<SurfaceBuffer>(SurfaceBuffer::NativeBufferToSurfaceBuffer(surfaceBuffer));
}

int32_t AVBufferCapiMock::Destroy()
{
    int32_t ret = OH_AVBuffer_Destroy(buffer_);
    UNITTEST_CHECK_AND_RETURN_RET_LOG(ret == static_cast<int32_t>(Status::OK), ret, "OH_AVBuffer_Destroy failed!");
    buffer_ = nullptr;
    return static_cast<int32_t>(Status::OK);
}

OH_AVBuffer *AVBufferCapiMock::GetAVBuffer()
{
    return buffer_;
}

// AVBufferQueueCapiMock::~AVBufferQueueCapiMock()
// {
//     DelCallback(bufferQueue_);
// }

// OH_AVBufferQueue *AVBufferQueueCapiMock::GetAVBufferQueue()
// {
//     return bufferQueue_;
// }

// void AVBufferQueueCapiMock::OnBufferAvailable(OH_AVBufferQueue *bufferQueue, void *userData)
// {
//     (void)userData;
//     std::shared_ptr<AVBufferQueueCallbackMock> mockCb = GetCallback(bufferQueue);
//     if (mockCb != nullptr) {
//         mockCb->OnBufferAvailable();
//     }
// }

// std::shared_ptr<AVBufferQueueCallbackMock> AVBufferQueueCapiMock::GetCallback(OH_AVBufferQueue *bufferQueue)
// {
//     std::lock_guard<std::mutex> lock(mutex_);
//     if (mockCbMap_.find(bufferQueue) != mockCbMap_.end()) {
//         return mockCbMap_.at(bufferQueue);
//     }
//     return nullptr;
// }

// void AVBufferQueueCapiMock::SetProducerCallback(OH_AVBufferQueue *bufferQueue,
//                                                 std::shared_ptr<AVBufferQueueCallbackMock> cb)
// {
//     std::lock_guard<std::mutex> lock(mutex_);
//     mockCbMap_[bufferQueue] = cb;
// }

// void AVBufferQueueCapiMock::DelCallback(OH_AVBufferQueue *bufferQueue)
// {
//     auto it = mockCbMap_.find(bufferQueue);
//     if (it != mockCbMap_.end()) {
//         mockCbMap_.erase(it);
//     }
// }

// int32_t AVBufferQueueCapiMock::SetProducerCallback(std::shared_ptr<AVBufferQueueCallbackMock> cb)
// {
//     if (cb != nullptr && bufferQueue_ != nullptr) {
//         SetProducerCallback(bufferQueue_, cb);
//         struct OH_AVBufferQueueCallback callback;
//         callback.onBufferAvailable = AVBufferQueueCapiMock::OnBufferAvailable;
//         return OH_AVBufferQueue_SetProducerCallback(bufferQueue_, callback, NULL);
//     }
//     return static_cast<int32_t>(Status::ERROR_INVALID_OPERATION);
// }

// int32_t AVBufferQueueCapiMock::PushBuffer(std::shared_ptr<AVBufferMock> &buffer)
// {
//     return OH_AVBufferQueue_PushBuffer(bufferQueue_,
//     std::static_pointer_cast<AVBufferCapiMock>(buffer)->GetAVBuffer());
// }

// std::shared_ptr<AVBufferMock> AVBufferQueueCapiMock::RequestBuffer(int32_t capacity)
// {
//     struct OH_AVBuffer *buf = OH_AVBufferQueue_RequestBuffer(bufferQueue_, capacity);
//     std::shared_ptr<AVBufferMock> buffer = std::make_shared<AVBufferCapiMock>(buf);
//     return buffer;
// }
} // namespace MediaAVCodec
} // namespace OHOS