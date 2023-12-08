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

#include "avbuffer_mock.h"
#include "buffer/avbuffer.h"

namespace OHOS {
namespace Media {
class AVBufferInnerMock : public AVBufferMock {
public:
    explicit AVBufferInnerMock(const std::shared_ptr<AVBuffer> &buffer) : buffer_(buffer) {}
    ~AVBufferInnerMock() = default;
    uint8_t *GetAddr() override;
    int32_t GetCapacity() override;
    int32_t GetBufferAttr(OH_AVCodecBufferAttr &attr) override;
    int32_t SetBufferAttr(const OH_AVCodecBufferAttr &attr) override;
    std::shared_ptr<FormatMock> GetParameter() override;
    int32_t SetParameter(const std::shared_ptr<FormatMock> &format) override;
    sptr<SurfaceBuffer> GetNativeBuffer() override;
    int32_t Destroy() override;
    std::shared_ptr<AVBuffer> &GetAVBuffer();

private:
    std::shared_ptr<AVBuffer> buffer_;
};
} // namespace Media
} // namespace OHOS
#endif // AVBUFFER_NATIVE_MOCK_H