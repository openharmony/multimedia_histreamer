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

#ifndef AVBUFFER_CAPI_MOCK_H
#define AVBUFFER_CAPI_MOCK_H

#include <map>
#include <mutex>
#include "avbuffer_mock.h"
#include "native_avbuffer.h"

namespace OHOS {
namespace MediaAVCodec {
class AVBufferCapiMock : public AVBufferMock {
public:
    explicit AVBufferCapiMock(OH_AVBuffer *buffer) : buffer_(buffer) {}
    ~AVBufferCapiMock() = default;
    uint8_t *GetAddr() override;
    int32_t GetCapacity() override;
    OH_AVBufferAttr GetBufferAttr() override;
    int32_t SetBufferAttr(OH_AVBufferAttr &attr) override;
    std::shared_ptr<FormatMock> GetParameter() override;
    int32_t SetParameter(const std::shared_ptr<FormatMock> &format) override;
    sptr<SurfaceBuffer> GetNativeBuffer() override;
    int32_t Destroy() override;
    OH_AVBuffer *GetAVBuffer();

private:
    OH_AVBuffer *buffer_;
};

// class AVBufferQueueCapiMock : public AVBufferQueueMock {
// public:
//     explicit AVBufferQueueCapiMock(OH_AVBufferQueue *bufferQueue) : bufferQueue_(bufferQueue) {}
//     ~AVBufferQueueCapiMock() override;
//     int32_t SetProducerCallback(std::shared_ptr<AVBufferQueueCallbackMock> callback) override;
//     int32_t PushBuffer(std::shared_ptr<AVBufferMock> &buffer) override;
//     std::shared_ptr<AVBufferMock> RequestBuffer(int32_t capacity) override;

//     OH_AVBufferQueue *GetAVBufferQueue();

// private:
//     static void OnBufferAvailable(OH_AVBufferQueue *codec, void *userData);
//     static void SetProducerCallback(OH_AVBufferQueue *codec, std::shared_ptr<AVBufferQueueCallbackMock> cb);
//     static void DelCallback(OH_AVBufferQueue *codec);
//     static std::shared_ptr<AVBufferQueueCallbackMock> GetCallback(OH_AVBufferQueue *codec);
//     OH_AVBufferQueue *bufferQueue_;
//     static std::mutex mutex_;
//     static std::map<OH_AVBufferQueue *, std::shared_ptr<AVBufferQueueCallbackMock>> mockCbMap_;
// };
} // namespace MediaAVCodec
} // namespace OHOS
#endif // AVBUFFER_CAPI_MOCK_H