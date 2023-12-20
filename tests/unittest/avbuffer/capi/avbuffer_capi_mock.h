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
namespace Media {
class AVBufferCapiMock : public AVBufferMock {
public:
    explicit AVBufferCapiMock(OH_AVBuffer *buffer) : buffer_(buffer) {}
    ~AVBufferCapiMock() = default;
    uint8_t *GetAddr() override;
    int32_t GetCapacity() override;
    int32_t GetBufferAttr(OH_AVCodecBufferAttr &attr) override;
    int32_t SetBufferAttr(const OH_AVCodecBufferAttr &attr) override;
    std::shared_ptr<FormatMock> GetParameter() override;
    int32_t SetParameter(const std::shared_ptr<FormatMock> &format) override;
    sptr<SurfaceBuffer> GetNativeBuffer() override;
    int32_t Destroy() override;
    OH_AVBuffer *GetAVBuffer();

private:
    OH_AVBuffer *buffer_;
};
} // namespace Media
} // namespace OHOS
#endif // AVBUFFER_CAPI_MOCK_H