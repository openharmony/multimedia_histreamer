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

#ifndef AVFORMAT_CAPI_MOCK_H
#define AVFORMAT_CAPI_MOCK_H

#include "avformat_mock.h"
#include "native_avformat.h"

namespace OHOS {
namespace Media {
class AVFormatCapiMock : public FormatMock {
public:
    explicit AVFormatCapiMock(OH_AVFormat *format) : format_(format) {}
    AVFormatCapiMock();
    ~AVFormatCapiMock();
    bool PutIntValue(const std::string_view &key, int32_t value) override;
    bool GetIntValue(const std::string_view &key, int32_t &value) override;
    bool PutStringValue(const std::string_view &key, const std::string_view &value) override;
    bool GetStringValue(const std::string_view &key, std::string &value) override;
    void Destroy() override;
    bool PutLongValue(const std::string_view &key, int64_t value) override;
    bool GetLongValue(const std::string_view &key, int64_t &value) override;
    bool PutFloatValue(const std::string_view &key, float value) override;
    bool GetFloatValue(const std::string_view &key, float &value) override;
    bool PutDoubleValue(const std::string_view &key, double value) override;
    bool GetDoubleValue(const std::string_view &key, double &value) override;
    bool GetBuffer(const std::string_view &key, uint8_t **addr, size_t &size) override;
    bool PutBuffer(const std::string_view &key, const uint8_t *addr, size_t size) override;
    void InitTrackFormat() override;
    void InitAudioTrackFormat(const std::string_view &mimeType, int32_t sampleRate, int32_t channelCount) override;
    void InitVideoTrackFormat(const std::string_view &mimeType, int32_t width, int32_t height) override;
    const char *DumpInfo() override;
    bool AVFormat_Copy(struct OH_AVFormat *to, struct OH_AVFormat *from);
    OH_AVFormat *GetFormat();

private:
    OH_AVFormat *format_;
};
}  // namespace Media
}  // namespace OHOS
#endif // AVFORMAT_CAPI_MOCK_H