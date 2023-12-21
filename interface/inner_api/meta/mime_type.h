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

#ifndef MEDIA_FOUNDATION_MIME_TYPE_H
#define MEDIA_FOUNDATION_MIME_TYPE_H

namespace OHOS {
namespace Media {
namespace Plugins {
class MimeType {
public:
    static constexpr const char VIDEO_H263[] = "video/h263";
    static constexpr const char VIDEO_AVC[] = "video/avc";
    static constexpr const char VIDEO_MPEG2[] = "video/mpeg2";
    static constexpr const char VIDEO_HEVC[] = "video/hevc";
    static constexpr const char VIDEO_MPEG4[] = "video/mp4v-es";
    static constexpr const char VIDEO_VP8[] = "video/x-vnd.on2.vp8";
    static constexpr const char VIDEO_VP9[] = "video/x-vnd.on2.vp9";

    static constexpr const char AUDIO_AMR_NB[] = "audio/3gpp";
    static constexpr const char AUDIO_AMR_WB[] = "audio/amr-wb";
    static constexpr const char AUDIO_MPEG[] = "audio/mpeg";
    static constexpr const char AUDIO_AAC[] = "audio/mp4a-latm";
    static constexpr const char AUDIO_VORBIS[] = "audio/vorbis";
    static constexpr const char AUDIO_OPUS[] = "audio/opus";
    static constexpr const char AUDIO_FLAC[] = "audio/flac";
    static constexpr const char AUDIO_RAW[] = "audio/raw";
    static constexpr const char AUDIO_AVS3DA[] = "audio/av3a";

    static constexpr const char IMAGE_JPG[] = "image/jpeg";
    static constexpr const char IMAGE_PNG[] = "image/png";
    static constexpr const char IMAGE_BMP[] = "image/bmp";

    static constexpr const char MEDIA_MP4[] = "media/mp4";
    static constexpr const char MEDIA_M4A[] = "media/m4a";
};
} // namespace Plugins
} // namespace Media
} // namespace OHOS
#endif // MEDIA_FOUNDATION_MIME_TYPE_H