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
#include "meta/meta_key.h"
#include "securec.h"
namespace OHOS {
namespace Media {
bool AVFormatInnerMock::PutIntValue(const std::string_view &key, int32_t value)
{
    return format_.PutIntValue(key, value);
}

bool AVFormatInnerMock::GetIntValue(const std::string_view &key, int32_t &value)
{
    return format_.GetIntValue(key, value);
}

bool AVFormatInnerMock::PutStringValue(const std::string_view &key, const std::string_view &value)
{
    return format_.PutStringValue(key, value);
}

bool AVFormatInnerMock::GetStringValue(const std::string_view &key, std::string &value)
{
    return format_.GetStringValue(key, value);
}

void AVFormatInnerMock::Destroy()
{
    if (dumpInfo_ != nullptr) {
        free(dumpInfo_);
        dumpInfo_ = nullptr;
    }
    return;
}

Format &AVFormatInnerMock::GetFormat()
{
    return format_;
}

bool AVFormatInnerMock::PutLongValue(const std::string_view &key, int64_t value)
{
    return format_.PutLongValue(key, value);
}

bool AVFormatInnerMock::GetLongValue(const std::string_view &key, int64_t &value)
{
    return format_.GetLongValue(key, value);
}

bool AVFormatInnerMock::PutFloatValue(const std::string_view &key, float value)
{
    return format_.PutFloatValue(key, value);
}

bool AVFormatInnerMock::GetFloatValue(const std::string_view &key, float &value)
{
    return format_.GetFloatValue(key, value);
}

bool AVFormatInnerMock::PutDoubleValue(const std::string_view &key, double value)
{
    return format_.PutDoubleValue(key, value);
}

bool AVFormatInnerMock::GetDoubleValue(const std::string_view &key, double &value)
{
    return format_.GetDoubleValue(key, value);
}

bool AVFormatInnerMock::GetBuffer(const std::string_view &key, uint8_t **addr, size_t &size)
{
    return format_.GetBuffer(key, addr, size);
}

bool AVFormatInnerMock::PutBuffer(const std::string_view &key, const uint8_t *addr, size_t size)
{
    return format_.PutBuffer(key, addr, size);
}

void AVFormatInnerMock::InitAudioTrackFormat(const std::string_view &mimeType, int32_t sampleRate, int32_t channelCount)
{
    format_.PutStringValue(Tag::MIME_TYPE, mimeType);
    format_.PutIntValue(Tag::AUDIO_SAMPLE_RATE, sampleRate);
    format_.PutIntValue(Tag::AUDIO_CHANNEL_COUNT, channelCount);
}

void AVFormatInnerMock::InitVideoTrackFormat(const std::string_view &mimeType, int32_t width, int32_t height)
{
    format_.PutStringValue(Tag::MIME_TYPE, mimeType);
    format_.PutIntValue(Tag::VIDEO_WIDTH, width);
    format_.PutIntValue(Tag::VIDEO_HEIGHT, height);
}

const char *AVFormatInnerMock::DumpInfo()
{
    if (dumpInfo_ != nullptr) {
        free(dumpInfo_);
        dumpInfo_ = nullptr;
    }
    std::string info = format_.Stringify();
    if (info.empty()) {
        return nullptr;
    }
    constexpr uint32_t maxDumpLength = 1024;
    uint32_t bufLength = info.size() > maxDumpLength ? maxDumpLength : info.size();
    dumpInfo_ = static_cast<char *>(malloc((bufLength + 1) * sizeof(char)));
    if (dumpInfo_ == nullptr) {
        return nullptr;
    }
    if (strcpy_s(dumpInfo_, bufLength + 1, info.c_str()) != 0) {
        free(dumpInfo_);
        dumpInfo_ = nullptr;
    }
    return dumpInfo_;
}
} // namespace Media
} // namespace OHOS