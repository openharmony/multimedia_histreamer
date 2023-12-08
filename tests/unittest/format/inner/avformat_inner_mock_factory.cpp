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

#include "avformat_inner_mock.h"

namespace OHOS {
namespace Media {
std::shared_ptr<FormatMock> FormatMockFactory::CreateFormat()
{
    return std::make_shared<AVFormatInnerMock>();
}

std::shared_ptr<FormatMock> FormatMockFactory::CreateAudioFormat(
    const std::string_view &mimeType, int32_t sampleRate, int32_t channelCount)
{
    std::shared_ptr<FormatMock> format = std::make_shared<AVFormatInnerMock>();
    format->InitAudioTrackFormat(mimeType, sampleRate, channelCount);
    return format;
}

std::shared_ptr<FormatMock> FormatMockFactory::CreateVideoFormat(
    const std::string_view &mimeType, int32_t width, int32_t height)
{
    std::shared_ptr<FormatMock> format = std::make_shared<AVFormatInnerMock>();
    format->InitVideoTrackFormat(mimeType, width, height);
    return format;
}
} // namespace Media
} // namespace OHOS