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

#include "native_avformat.h"

#include "common/log.h"
#include "common/native_mfmagic.h"
#include "common/status.h"
#include "meta/meta_key.h"
#include "securec.h"


namespace {
constexpr uint32_t MAX_STRING_LENGTH = 256;
constexpr uint32_t MAX_DUMP_LENGTH = 1024;
} // namespace

using namespace OHOS::Media;

OH_AVFormat::OH_AVFormat() : MFObjectMagic(MFMagic::MFMAGIC_FORMAT) {}

OH_AVFormat::OH_AVFormat(const Format &fmt) : MFObjectMagic(MFMagic::MFMAGIC_FORMAT), format_(fmt) {}

OH_AVFormat::~OH_AVFormat()
{
    if (outString_ != nullptr) {
        free(outString_);
        outString_ = nullptr;
    }
    if (dumpInfo_ != nullptr) {
        free(dumpInfo_);
        dumpInfo_ = nullptr;
    }
}

struct OH_AVFormat *OH_AVFormat_Create(void)
{
    return new (std::nothrow) OH_AVFormat();
}

struct OH_AVFormat *OH_AVFormat_CreateAudioFormat(const char *mimeType, int32_t sampleRate, int32_t channelCount)
{
    FALSE_RETURN_V_MSG_E(mimeType != nullptr, nullptr, "mimeType is nullptr!");
    OH_AVFormat *audioFormat = new (std::nothrow) OH_AVFormat();
    FALSE_RETURN_V_MSG_E(audioFormat != nullptr, nullptr, "new format is nullptr!");
    audioFormat->format_.PutStringValue(Tag::MIME_TYPE, mimeType);
    audioFormat->format_.PutIntValue(Tag::AUDIO_SAMPLE_RATE, sampleRate);
    audioFormat->format_.PutIntValue(Tag::AUDIO_CHANNEL_COUNT, channelCount);
    return audioFormat;
}

struct OH_AVFormat *OH_AVFormat_CreateVideoFormat(const char *mimeType, int32_t width, int32_t height)
{
    FALSE_RETURN_V_MSG_E(mimeType != nullptr, nullptr, "mimeType is nullptr!");
    OH_AVFormat *videoFormat = new (std::nothrow) OH_AVFormat();
    FALSE_RETURN_V_MSG_E(videoFormat != nullptr, nullptr, "new format is nullptr!");
    videoFormat->format_.PutStringValue(Tag::MIME_TYPE, mimeType);
    videoFormat->format_.PutIntValue(Tag::VIDEO_WIDTH, width);
    videoFormat->format_.PutIntValue(Tag::VIDEO_HEIGHT, height);
    return videoFormat;
}

void OH_AVFormat_Destroy(struct OH_AVFormat *format)
{
    delete format;
}

bool OH_AVFormat_Copy(struct OH_AVFormat *to, struct OH_AVFormat *from)
{
    FALSE_RETURN_V_MSG_E(to != nullptr, false, "to format is nullptr!");
    FALSE_RETURN_V_MSG_E(to->magic_ == MFMagic::MFMAGIC_FORMAT, false, "magic error!");
    FALSE_RETURN_V_MSG_E(from != nullptr, false, "from format is nullptr!");
    FALSE_RETURN_V_MSG_E(from->magic_ == MFMagic::MFMAGIC_FORMAT, false, "magic error!");

    to->format_ = from->format_;
    return true;
}

bool OH_AVFormat_SetIntValue(struct OH_AVFormat *format, const char *key, int32_t value)
{
    FALSE_RETURN_V_MSG_E(format != nullptr, false, "input format is nullptr!");
    FALSE_RETURN_V_MSG_E(format->magic_ == MFMagic::MFMAGIC_FORMAT, false, "magic error!");
    FALSE_RETURN_V_MSG_E(key != nullptr, false, "key is nullptr!");

    return format->format_.PutIntValue(key, value);
}

bool OH_AVFormat_SetLongValue(struct OH_AVFormat *format, const char *key, int64_t value)
{
    FALSE_RETURN_V_MSG_E(format != nullptr, false, "input format is nullptr!");
    FALSE_RETURN_V_MSG_E(format->magic_ == MFMagic::MFMAGIC_FORMAT, false, "magic error!");
    FALSE_RETURN_V_MSG_E(key != nullptr, false, "key is nullptr!");

    return format->format_.PutLongValue(key, value);
}

bool OH_AVFormat_SetFloatValue(struct OH_AVFormat *format, const char *key, float value)
{
    FALSE_RETURN_V_MSG_E(format != nullptr, false, "input format is nullptr!");
    FALSE_RETURN_V_MSG_E(format->magic_ == MFMagic::MFMAGIC_FORMAT, false, "magic error!");
    FALSE_RETURN_V_MSG_E(key != nullptr, false, "key is nullptr!");

    return format->format_.PutFloatValue(key, value);
}

bool OH_AVFormat_SetDoubleValue(struct OH_AVFormat *format, const char *key, double value)
{
    FALSE_RETURN_V_MSG_E(format != nullptr, false, "input format is nullptr!");
    FALSE_RETURN_V_MSG_E(format->magic_ == MFMagic::MFMAGIC_FORMAT, false, "magic error!");
    FALSE_RETURN_V_MSG_E(key != nullptr, false, "key is nullptr!");

    return format->format_.PutDoubleValue(key, value);
}

bool OH_AVFormat_SetStringValue(struct OH_AVFormat *format, const char *key, const char *value)
{
    FALSE_RETURN_V_MSG_E(format != nullptr, false, "input format is nullptr!");
    FALSE_RETURN_V_MSG_E(format->magic_ == MFMagic::MFMAGIC_FORMAT, false, "magic error!");
    FALSE_RETURN_V_MSG_E(key != nullptr, false, "key is nullptr!");
    FALSE_RETURN_V_MSG_E(value != nullptr, false, "value is nullptr!");

    return format->format_.PutStringValue(key, value);
}

bool OH_AVFormat_SetBuffer(struct OH_AVFormat *format, const char *key, const uint8_t *addr, size_t size)
{
    FALSE_RETURN_V_MSG_E(format != nullptr, false, "input format is nullptr!");
    FALSE_RETURN_V_MSG_E(format->magic_ == MFMagic::MFMAGIC_FORMAT, false, "magic error!");
    FALSE_RETURN_V_MSG_E(key != nullptr, false, "key is nullptr!");
    FALSE_RETURN_V_MSG_E(addr != nullptr, false, "addr is nullptr!");
    FALSE_RETURN_V_MSG_E(size != 0, false, "size is zero!");

    return format->format_.PutBuffer(key, addr, size);
}

bool OH_AVFormat_GetIntValue(struct OH_AVFormat *format, const char *key, int32_t *out)
{
    FALSE_RETURN_V_MSG_E(format != nullptr, false, "input format is nullptr!");
    FALSE_RETURN_V_MSG_E(format->magic_ == MFMagic::MFMAGIC_FORMAT, false, "magic error!");
    FALSE_RETURN_V_MSG_E(key != nullptr, false, "key is nullptr!");
    FALSE_RETURN_V_MSG_E(out != nullptr, false, "out is nullptr!");

    return format->format_.GetIntValue(key, *out);
}

bool OH_AVFormat_GetLongValue(struct OH_AVFormat *format, const char *key, int64_t *out)
{
    FALSE_RETURN_V_MSG_E(format != nullptr, false, "input format is nullptr!");
    FALSE_RETURN_V_MSG_E(format->magic_ == MFMagic::MFMAGIC_FORMAT, false, "magic error!");
    FALSE_RETURN_V_MSG_E(key != nullptr, false, "key is nullptr!");
    FALSE_RETURN_V_MSG_E(out != nullptr, false, "out is nullptr!");

    return format->format_.GetLongValue(key, *out);
}

bool OH_AVFormat_GetFloatValue(struct OH_AVFormat *format, const char *key, float *out)
{
    FALSE_RETURN_V_MSG_E(format != nullptr, false, "input format is nullptr!");
    FALSE_RETURN_V_MSG_E(format->magic_ == MFMagic::MFMAGIC_FORMAT, false, "magic error!");
    FALSE_RETURN_V_MSG_E(key != nullptr, false, "key is nullptr!");
    FALSE_RETURN_V_MSG_E(out != nullptr, false, "out is nullptr!");

    return format->format_.GetFloatValue(key, *out);
}

bool OH_AVFormat_GetDoubleValue(struct OH_AVFormat *format, const char *key, double *out)
{
    FALSE_RETURN_V_MSG_E(format != nullptr, false, "input format is nullptr!");
    FALSE_RETURN_V_MSG_E(format->magic_ == MFMagic::MFMAGIC_FORMAT, false, "magic error!");
    FALSE_RETURN_V_MSG_E(key != nullptr, false, "key is nullptr!");
    FALSE_RETURN_V_MSG_E(out != nullptr, false, "out is nullptr!");

    return format->format_.GetDoubleValue(key, *out);
}

bool OH_AVFormat_GetStringValue(struct OH_AVFormat *format, const char *key, const char **out)
{
    FALSE_RETURN_V_MSG_E(format != nullptr, false, "input format is nullptr!");
    FALSE_RETURN_V_MSG_E(format->magic_ == MFMagic::MFMAGIC_FORMAT, false, "magic error!");
    FALSE_RETURN_V_MSG_E(key != nullptr, false, "key is nullptr!");
    FALSE_RETURN_V_MSG_E(out != nullptr, false, "out is nullptr!");

    if (format->outString_ != nullptr) {
        free(format->outString_);
        format->outString_ = nullptr;
    }

    std::string str;
    bool ret = format->format_.GetStringValue(key, str);
    if (!ret) {
        return false;
    }
    uint32_t bufLength = str.size() > MAX_STRING_LENGTH ? MAX_STRING_LENGTH : str.size();

    format->outString_ = static_cast<char *>(malloc((bufLength + 1) * sizeof(char)));
    FALSE_RETURN_V_MSG_E(format->outString_ != nullptr, false, "malloc out string nullptr!");

    if (strcpy_s(format->outString_, bufLength + 1, str.c_str()) != EOK) {
        MEDIA_LOG_E("Failed to strcpy_s");
        free(format->outString_);
        format->outString_ = nullptr;
        return false;
    }

    *out = format->outString_;
    return true;
}

bool OH_AVFormat_GetBuffer(struct OH_AVFormat *format, const char *key, uint8_t **addr, size_t *size)
{
    FALSE_RETURN_V_MSG_E(format != nullptr, false, "input format is nullptr!");
    FALSE_RETURN_V_MSG_E(format->magic_ == MFMagic::MFMAGIC_FORMAT, false, "magic error!");
    FALSE_RETURN_V_MSG_E(key != nullptr, false, "key is nullptr!");
    FALSE_RETURN_V_MSG_E(addr != nullptr, false, "addr is nullptr!");
    FALSE_RETURN_V_MSG_E(size != nullptr, false, "size is nullptr!");

    return format->format_.GetBuffer(key, addr, *size);
}

const char *OH_AVFormat_DumpInfo(struct OH_AVFormat *format)
{
    FALSE_RETURN_V_MSG_E(format != nullptr, nullptr, "input format is nullptr!");
    if (format->dumpInfo_ != nullptr) {
        free(format->dumpInfo_);
        format->dumpInfo_ = nullptr;
    }
    std::string info = format->format_.Stringify();
    if (info.empty()) {
        return nullptr;
    }
    uint32_t bufLength = info.size() > MAX_DUMP_LENGTH ? MAX_DUMP_LENGTH : info.size();
    format->dumpInfo_ = static_cast<char *>(malloc((bufLength + 1) * sizeof(char)));
    FALSE_RETURN_V_MSG_E(format->dumpInfo_ != nullptr, nullptr, "malloc dump info nullptr!");
    if (strcpy_s(format->dumpInfo_, bufLength + 1, info.c_str()) != EOK) {
        MEDIA_LOG_E("Failed to strcpy_s");
        free(format->dumpInfo_);
        format->dumpInfo_ = nullptr;
    }
    return format->dumpInfo_;
}
