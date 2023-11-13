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

#ifndef AVBUFFER_INNER_MOCK_H
#define AVBUFFER_INNER_MOCK_H

#include "buffer/avbuffer.h"
#include "avbuffer_mock.h"


namespace OHOS {
namespace MediaAVCodec {
class AVBufferInnerMock : public AVBufferMock {
public:
    explicit AVBufferInnerMock(const std::shared_ptr<AVBuffer> &buffer) : buffer_(buffer) {}
    ~AVBufferInnerMock() = default;
    uint8_t *GetAddr() override;
    int32_t GetCapacity() override;
    OH_AVBufferAttr GetBufferAttr() override;
    int32_t SetBufferAttr(OH_AVBufferAttr &attr) override;
    std::shared_ptr<FormatMock> GetParameter() override;
    int32_t SetParameter(const std::shared_ptr<FormatMock> &format) override;
    sptr<SurfaceBuffer> GetNativeBuffer() override;
    int32_t Destroy() override;
    std::shared_ptr<AVBuffer> &GetAVBuffer();
private:
    std::shared_ptr<AVBuffer> buffer_;
};

// class AVBufferQueueInnerMock : public AVBufferQueueMock {
// public:
//     explicit AVBufferQueueInnerMock(std::shared_ptr<AVBufferQueue> bufferQueue) : bufferQueue_(bufferQueue) {}
//     ~AVBufferQueueInnerMock() = default;
//     int32_t SetProducerCallback(std::shared_ptr<AVBufferQueueCallbackMock> callback) override;
//     int32_t PushBuffer(std::shared_ptr<AVBufferMock> &buffer) override;
//     std::shared_ptr<AVBufferMock> RequestBuffer(int32_t capacity) override;
//     std::shared_ptr<AVBufferQueue> GetAVBufferQueue();

// private:
//     std::shared_ptr<AVBufferQueue> bufferQueue_ = nullptr;
// };

// class AVBufferQueueCallbackInnerMock : public AVBufferQueueCallback {
// public:
//     AVBufferQueueCallbackInnerMock(std::shared_ptr<AVBufferQueueCallbackMock> cb, std::weak_ptr<AVBufferQueue> bufferQueue);
//     ~AVBufferQueueCallbackInnerMock() = default;
//     void OnBufferAvailable() override;

// private:
//     std::shared_ptr<AVBufferQueueCallbackMock> mockCb_ = nullptr;
//     std::weak_ptr<AVBufferQueue> bufferQueue_;
// };
}  // namespace MediaAVCodec
}  // namespace OHOS
#endif // AVBUFFER_NATIVE_MOCK_H