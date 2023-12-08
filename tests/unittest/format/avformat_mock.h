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

#ifndef AVFORMAT_MOCK_H
#define AVFORMAT_MOCK_H

#include <string>
#include "native_avbuffer.h"
#include "nocopyable.h"

namespace OHOS {
namespace Media {
class FormatMock : public NoCopyable {
public:
    virtual ~FormatMock() = default;
    virtual bool PutIntValue(const std::string_view &key, int32_t value) = 0;
    virtual bool GetIntValue(const std::string_view &key, int32_t &value) = 0;
    virtual bool PutLongValue(const std::string_view &key, int64_t value) = 0;
    virtual bool GetLongValue(const std::string_view &key, int64_t &value) = 0;
    virtual bool PutFloatValue(const std::string_view &key, float value) = 0;
    virtual bool GetFloatValue(const std::string_view &key, float &value) = 0;
    virtual bool PutDoubleValue(const std::string_view &key, double value) = 0;
    virtual bool GetDoubleValue(const std::string_view &key, double &value) = 0;
    virtual bool PutStringValue(const std::string_view &key, const std::string_view &value) = 0;
    virtual bool GetStringValue(const std::string_view &key, std::string &value) = 0;
    virtual bool GetBuffer(const std::string_view &key, uint8_t **addr, size_t &size) = 0;
    virtual bool PutBuffer(const std::string_view &key, const uint8_t *addr, size_t size) = 0;
    virtual void InitTrackFormat() = 0;
    virtual void InitAudioTrackFormat(const std::string_view &mimeType, int32_t sampleRate, int32_t channelCount) = 0;
    virtual void InitVideoTrackFormat(const std::string_view &mimeType, int32_t width, int32_t height) = 0;
    virtual const char *DumpInfo() = 0;
    virtual void Destroy() = 0;
};

class __attribute__((visibility("default"))) FormatMockFactory {
public:
    static std::shared_ptr<FormatMock> CreateFormat();
    static std::shared_ptr<FormatMock> CreateAudioFormat(const std::string_view &mimeType, int32_t sampleRate,
                                                         int32_t channelCount);
    static std::shared_ptr<FormatMock> CreateVideoFormat(const std::string_view &mimeType, int32_t width,
                                                         int32_t height);

private:
    FormatMockFactory() = delete;
    ~FormatMockFactory() = delete;
};
} // namespace Media
} // namespace OHOS
#endif // AVFORMAT_MOCK_H