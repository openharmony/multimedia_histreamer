/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef RECORDER_PARAM_H
#define RECORDER_PARAM_H

#include <cstdint>
#include <string>
#include "recorder.h"

namespace OHOS {
namespace Media {
/*
 * Declare the type enum value valid range for different kind of RecorderParam.
 * The "Public" means externally visible.
 * The "Private" means internally visible.
 */
enum RecorderParamSectionStart {
    PUBLIC_PARAM_SECTION_START = 0,
    PRIVATE_PARAM_SECTION_START = 0x10000,
};

enum RecorderPublicParamType : uint32_t {
    PUBLIC_PARAM_TYPE_BEGIN = PUBLIC_PARAM_SECTION_START,
    // video begin
    VID_PUBLIC_PARAM_BEGIN,
    VID_ENC_FMT,
    VID_RECTANGLE,
    VID_BITRATE,
    VID_FRAMERATE,
    VID_CAPTURERATE,
    VID_PUBLIC_PARAM_END,
    VID_ORIENTATION_HINT,
    // audio begin
    AUD_PUBLIC_PARAM_BEGIN,
    AUD_ENC_FMT,
    AUD_SAMPLERATE,
    AUD_CHANNEL,
    AUD_BITRATE,
    AUD_PUBIC_PARAM_END,
    // output begin,
    MAX_DURATION,
    MAX_SIZE,
    OUT_PATH,
    OUT_FD,
    NEXT_OUT_FD, // reserved.
    GEO_LOCATION,

    PUBLIC_PARAM_TYPE_END,
};

/*
 * Recorder parameter base structure, inherite to it to extend the new parameter.
 */
struct RecorderParam {
    explicit RecorderParam(uint32_t t) : type(t) {}
    RecorderParam() = delete;
    virtual ~RecorderParam() = default;
    bool IsVideoParam() const
    {
        return (type > VID_PUBLIC_PARAM_BEGIN) && (type < VID_PUBLIC_PARAM_END);
    }

    bool IsAudioParam() const
    {
        return (type > AUD_PUBLIC_PARAM_BEGIN) && (type < AUD_PUBIC_PARAM_END);
    }

    uint32_t type;
};

struct VidEnc : public RecorderParam {
    explicit VidEnc(VideoCodecFormat fmt) : RecorderParam(RecorderPublicParamType::VID_ENC_FMT), encFmt(fmt) {}
    VideoCodecFormat encFmt;
};

struct VidRectangle : public RecorderParam {
    VidRectangle(int32_t w, int32_t h) : RecorderParam(RecorderPublicParamType::VID_RECTANGLE), width(w), height(h) {}
    int32_t width;
    int32_t height;
};

struct VidBitRate : public RecorderParam {
    explicit VidBitRate(int32_t br) : RecorderParam(RecorderPublicParamType::VID_BITRATE), bitRate(br) {}
    int32_t bitRate;
};

struct VidFrameRate : public RecorderParam {
    explicit VidFrameRate(int32_t r) : RecorderParam(RecorderPublicParamType::VID_FRAMERATE), frameRate(r) {}
    int32_t frameRate;
};

struct CaptureRate : public RecorderParam {
    explicit CaptureRate(double cr) : RecorderParam(RecorderPublicParamType::VID_CAPTURERATE), capRate(cr) {}
    double capRate;
};

struct AudEnc : public RecorderParam {
    explicit AudEnc(AudioCodecFormat fmt) : RecorderParam(RecorderPublicParamType::AUD_ENC_FMT), encFmt(fmt) {}
    AudioCodecFormat encFmt;
};

struct AudSampleRate : public RecorderParam {
    explicit AudSampleRate(int32_t sr) : RecorderParam(RecorderPublicParamType::AUD_SAMPLERATE), sampleRate(sr) {}
    int32_t sampleRate;
};

struct AudChannel : public RecorderParam {
    explicit AudChannel(int32_t num) : RecorderParam(RecorderPublicParamType::AUD_CHANNEL), channel(num) {}
    int32_t channel;
};

struct AudBitRate : public RecorderParam {
    explicit AudBitRate(int32_t br) : RecorderParam(RecorderPublicParamType::AUD_BITRATE), bitRate(br) {}
    int32_t bitRate;
};

struct MaxDuration : public RecorderParam {
    explicit MaxDuration(int32_t maxDur) : RecorderParam(RecorderPublicParamType::MAX_DURATION), duration(maxDur) {}
    int32_t duration;
};

struct MaxFileSize : public RecorderParam {
    explicit MaxFileSize(int64_t maxSize) : RecorderParam(RecorderPublicParamType::MAX_SIZE), size(maxSize) {}
    int64_t size;
};

struct GeoLocation : public RecorderParam {
    explicit GeoLocation(float lat, float lng)
        : RecorderParam(RecorderPublicParamType::GEO_LOCATION), latitude(lat), longitude(lng) {}
    float latitude;
    float longitude;
};

struct RotationAngle : public RecorderParam {
    explicit RotationAngle(int32_t angle)
        : RecorderParam(RecorderPublicParamType::VID_ORIENTATION_HINT), rotation(angle) {}
    int32_t rotation;
};

struct OutFilePath : public RecorderParam {
    explicit OutFilePath(const std::string &filePath)
        : RecorderParam(RecorderPublicParamType::OUT_PATH), path(filePath) {}
    std::string path;
};

struct OutFd : public RecorderParam {
    explicit OutFd(int32_t outFd) : RecorderParam(RecorderPublicParamType::OUT_FD), fd(outFd) {}
    int32_t fd;
};

struct NextOutFd : public RecorderParam {
    explicit NextOutFd(int32_t nextOutFd) : RecorderParam(RecorderPublicParamType::NEXT_OUT_FD), fd(nextOutFd) {}
    int32_t fd;
};
} // namespace Media
} // namespace OHOS
#endif
