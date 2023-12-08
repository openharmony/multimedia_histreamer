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

#ifndef AVBUFFER_MOCK_H
#define AVBUFFER_MOCK_H

#include <string>
#include "avformat_mock.h"
#include "native_avbuffer.h"
#include "nocopyable.h"
#include "surface.h"

namespace OHOS {
namespace Media {
class AVBufferMock : public NoCopyable {
public:
    virtual ~AVBufferMock() = default;
    virtual uint8_t *GetAddr() = 0;
    virtual int32_t GetCapacity() = 0;
    virtual int32_t GetBufferAttr(OH_AVCodecBufferAttr &attr) = 0;
    virtual int32_t SetBufferAttr(const OH_AVCodecBufferAttr &attr) = 0;
    virtual std::shared_ptr<FormatMock> GetParameter() = 0;
    virtual int32_t SetParameter(const std::shared_ptr<FormatMock> &format) = 0;
    virtual sptr<SurfaceBuffer> GetNativeBuffer() = 0;
    virtual int32_t Destroy() = 0;
};

class __attribute__((visibility("default"))) AVBufferMockFactory {
public:
    static std::shared_ptr<AVBufferMock> CreateAVBuffer(const int32_t &capacity);

private:
    AVBufferMockFactory() = delete;
    ~AVBufferMockFactory() = delete;
};
} // namespace Media
} // namespace OHOS
#endif // AVBUFFER_MOCK_H