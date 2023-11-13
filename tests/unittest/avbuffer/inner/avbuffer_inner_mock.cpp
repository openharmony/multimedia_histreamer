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

#include "avbuffer_inner_mock.h"
#include "avformat_inner_mock.h"
#include "media_description.h"
#include "native_mferrors.h"
#include "securec.h"
#include "unittest_log.h"

namespace OHOS {
namespace MediaAVCodec {
uint8_t *AVBufferInnerMock::GetAddr()
{
    UNITTEST_CHECK_AND_RETURN_RET_LOG(buffer_ != nullptr, nullptr, "buffer_ is nullptr!");
    UNITTEST_CHECK_AND_RETURN_RET_LOG(buffer_->memory_ != nullptr, nullptr, "buffer_->memory_ is nullptr!");
    return buffer_->memory_->GetAddr();
}

int32_t AVBufferInnerMock::GetCapacity()
{
    UNITTEST_CHECK_AND_RETURN_RET_LOG(buffer_ != nullptr, -1, "buffer_ is nullptr!");
    UNITTEST_CHECK_AND_RETURN_RET_LOG(buffer_->memory_ != nullptr, -1, "buffer_->memory_ is nullptr!");
    return buffer_->memory_->GetCapacity();
}

OH_AVBufferAttr AVBufferInnerMock::GetBufferAttr()
{
    OH_AVBufferAttr attr;
    UNITTEST_CHECK_AND_RETURN_RET_LOG(buffer_ != nullptr, attr, "buffer_ is nullptr!");
    attr.pts = buffer_->pts_;
    attr.offset = buffer_->memory_->GetOffset();
    attr.size = buffer_->memory_->GetSize();
    attr.flags = static_cast<uint32_t>(buffer_->flag_);
    return attr;
}
int32_t AVBufferInnerMock::SetBufferAttr(OH_AVBufferAttr &attr)
{
    UNITTEST_CHECK_AND_RETURN_RET_LOG(buffer_ != nullptr, static_cast<int32_t>(Status::ERROR_UNKNOWN),
                                      "buffer_ is nullptr!");
    buffer_->pts_ = attr.pts;
    buffer_->memory_->SetOffset(attr.offset);
    buffer_->flag_ = static_cast<AVCodecBufferFlag>(attr.flags);
    return buffer_->memory_->SetSize(attr.size);
}

std::shared_ptr<FormatMock> AVBufferInnerMock::GetParameter()
{
    UNITTEST_CHECK_AND_RETURN_RET_LOG(buffer_ != nullptr, nullptr, "buffer_ is nullptr!");
    auto formatMock = std::make_shared<AVFormatInnerMock>(*(buffer_->meta_));
    return formatMock;
}

int32_t AVBufferInnerMock::SetParameter(const std::shared_ptr<FormatMock> &format)
{
    UNITTEST_CHECK_AND_RETURN_RET_LOG(buffer_ != nullptr, static_cast<int32_t>(Status::ERROR_UNKNOWN),
                                      "buffer_ is nullptr!");
    *(buffer_->meta_) = std::static_pointer_cast<AVFormatInnerMock>(format)->GetFormat();
    return static_cast<int32_t>(Status::OK);
}

sptr<SurfaceBuffer> AVBufferInnerMock::GetNativeBuffer()
{
    UNITTEST_CHECK_AND_RETURN_RET_LOG(buffer_ != nullptr, nullptr, "buffer_ is nullptr!");
    UNITTEST_CHECK_AND_RETURN_RET_LOG(buffer_->memory_ != nullptr, nullptr, "buffer_->memory_ is nullptr!");
    return buffer_->memory_->GetSurfaceBuffer();
}

int32_t AVBufferInnerMock::Destroy()
{
    buffer_ = nullptr;
    return static_cast<int32_t>(Status::OK);
}

std::shared_ptr<AVBuffer> &AVBufferInnerMock::GetAVBuffer()
{
    return buffer_;
}

// AVBufferQueueCallbackInnerMock::AVBufferQueueCallbackInnerMock(std::shared_ptr<AVBufferQueueCallbackMock> cb,
//                                                              std::weak_ptr<AVBufferQueue> bufferQueue)
//     : mockCb_(cb), bufferQueue_(bufferQueue)
// {
// }

// void AVBufferQueueCallbackInnerMock::OnBufferAvailable()
// {
//     if (mockCb_ != nullptr) {
//         mockCb_->OnBufferAvailable();
//     }
// }

// int32_t AVBufferQueueInnerMock::SetProducerCallback(std::shared_ptr<AVBufferQueueCallbackMock> cb)
// {
//     if (cb != nullptr) {
//         auto callback = std::make_shared<AVBufferQueueCallbackInnerMock>(cb, bufferQueue_);
//         if (bufferQueue_ != nullptr) {
//             return bufferQueue_->SetProducerCallback(callback);
//         }
//     }
//     return static_cast<int32_t>(Status::ERROR_UNKNOWN);
// }

// int32_t AVBufferQueueInnerMock::PushBuffer(std::shared_ptr<AVBufferMock> &buffer)
// {
//     UNITTEST_CHECK_AND_RETURN_RET_LOG(bufferQueue_ != nullptr, static_cast<int32_t>(Status::ERROR_INVALID_PARAMETER),
//     "bufferQueue_ is nullptr!"); return
//     bufferQueue_->PushBuffer(std::static_pointer_cast<AVBufferInnerMock>(buffer)->GetAVBuffer());
// }

// std::shared_ptr<AVBufferMock> AVBufferQueueInnerMock::RequestBuffer(int32_t capacity)
// {
//     UNITTEST_CHECK_AND_RETURN_RET_LOG(bufferQueue_ != nullptr, nullptr, "bufferQueue_ is nullptr!");
//     std::shared_ptr<AVBuffer> buf = bufferQueue_->RequestBuffer(capacity);
//     std::shared_ptr<AVBufferMock> buffer = std::make_shared<AVBufferInnerMock>(buf);
//     return buffer;
// }

// std::shared_ptr<AVBufferQueue> AVBufferQueueInnerMock::GetAVBufferQueue()
// {
//     return bufferQueue_;
// }
} // namespace MediaAVCodec
} // namespace OHOS