/*
 * Copyright (c) 2021-2021 Huawei Device Co., Ltd.
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

#ifndef HISTREAMER_PLUGIN_COMMON_TAGS_H
#define HISTREAMER_PLUGIN_COMMON_TAGS_H

#include <map> // NOLINT
#include <string>
#include "any.h" // NOLINT

namespace OHOS {
namespace Media {
namespace Plugin {
enum struct TagSection : uint8_t {
    REGULAR = 1,
    MEDIA = 2,
    AUDIO_UNIVERSAL = 3,
    AUDIO_SPECIFIC = 4,
    VIDEO_UNIVERSAL = 5,
    VIDEO_SPECIFIC = 6,
    MAX_SECTION = 64
};

enum struct AudioFormat : uint8_t {
    MPEG = 1,
    AAC,
};

enum struct VideoFormat : uint8_t {
    H264 = 1,
};

#define MAKE_AUDIO_SPECIFIC_START(format) (SECTION_AUDIO_SPECIFIC_START | (static_cast<uint8_t>(format) << 8U))

/**
 * @brief Tag is a key-value pair used to settings or transfer information.
 *
 * The "key" member：An uint32_t index, defined as an enumerated type.
 *   Tag Index consisting of the following fragments:
 *   - reserved field
 *   - vendor extensions
 *   - section (regular, audio, video ...)
 *   - addition index
 *
 *   layout:
 *          +----------+---------+--------+----------------+
 *          | reserved | section | vendor | addition index |
 *          +----------+---------+--------+----------------+
 *    bit:   31 ... 22  21 ... 16    15    14 ............ 0
 *
 * The "value" member: Different tag have different value types,
 *                     which is defined in the plug-in interface.
 *
 * @since 1.0
 * @version 1.0
 */
enum struct Tag : uint32_t {
    INVALID = 0,
    SECTION_REGULAR_START = static_cast<uint8_t>(TagSection::REGULAR) << 16U,                 // regular tag
    SECTION_MEDIA_START = static_cast<uint8_t>(TagSection::MEDIA) << 16U,                     // media tag
    SECTION_AUDIO_UNIVERSAL_START = static_cast<uint8_t>(TagSection::AUDIO_UNIVERSAL) << 16U, // audio universal tag
    SECTION_AUDIO_SPECIFIC_START = static_cast<uint8_t>(TagSection::AUDIO_SPECIFIC) << 16U,   // audio specific tag
    SECTION_VIDEO_UNIVERSAL_START = static_cast<uint8_t>(TagSection::VIDEO_UNIVERSAL) << 16U, // video universal tag
    SECTION_VIDEO_SPECIFIC_START = static_cast<uint8_t>(TagSection::VIDEO_SPECIFIC) << 16U,   // video specific tag

    /* -------------------- regular tag -------------------- */
    MIME = SECTION_REGULAR_START + 1, // string
    STREAM_INDEX,                     // uint32_t
    REQUIRED_OUT_BUFFER_CNT,          // uint32_t required buffer count of plugin; read only tag
    PARAMETER_STRUCT,                 // ParameterStruct

    /* -------------------- media tag -------------------- */
    MEDIA_TITLE = SECTION_MEDIA_START + 1, // string
    MEDIA_ARTIST,                          // string
    MEDIA_LYRICIST,                        // string
    MEDIA_ALBUM,                           // string
    MEDIA_ALBUM_ARTIST,                    // string
    MEDIA_DATE,                            // string, format：YYYY-MM-DD
    MEDIA_COMMENT,                         // string
    MEDIA_GENRE,                           // string
    MEDIA_COPYRIGHT,                       // string
    MEDIA_LANGUAGE,                        // string
    MEDIA_DESCRIPTION,                     // string
    MEDIA_LYRICS,                          // string
    MEDIA_DURATION,                        // uint64_t
    MEDIA_FILE_SIZE,                       // uint64_t
    MEDIA_BITRATE,                         // int64_t
    MEDIA_FILE_EXTENSION,                  // string
    MEDIA_CODEC_CONFIG,                    // vector<uint8>, e.g. AudioSpecificConfig for mp4
    MEDIA_POSITION, ///< uint64_t : The byte position within media stream/file

    /* -------------------- audio universal tag -------------------- */
    AUDIO_CHANNELS = SECTION_AUDIO_UNIVERSAL_START + 1, // uint32_t
    AUDIO_CHANNEL_LAYOUT,                               // AudioChannelLayout
    AUDIO_SAMPLE_RATE,                                  // uint32_t
    AUDIO_SAMPLE_FORMAT,                                // AudioSampleFormat
    AUDIO_SAMPLE_PER_FRAME,                             // uint32_t

    /* -------------------- audio specific tag -------------------- */
    AUDIO_SPECIFIC_MPEG_START = MAKE_AUDIO_SPECIFIC_START(AudioFormat::MPEG),
    AUDIO_MPEG_VERSION, // uint32_t
    AUDIO_MPEG_LAYER,   // uint32_t

    AUDIO_SPECIFIC_AAC_START = MAKE_AUDIO_SPECIFIC_START(AudioFormat::AAC),
    AUDIO_AAC_PROFILE,       // AudioAacProfile
    AUDIO_AAC_LEVEL,         // uint32_t
    AUDIO_AAC_STREAM_FORMAT, // AudioAacStreamFormat

    /* -------------------- video universal tag -------------------- */
    VIDEO_WIDTH = SECTION_VIDEO_UNIVERSAL_START + 1, // uint32_t
    VIDEO_HEIGHT,                                    // uint32_t
    VIDEO_PIXEL_FORMAT,                              // uint32_t
};

using ValueType = Any;

/**
 * The tag content is stored in key-value format.
 */
using TagMap = std::map<Tag, ValueType>;

/**
 * @enum Direction
 *
 * @since 1.0
 * @version 1.0
 */
enum struct Direction : uint8_t {
    IN = 1<<0U, ///< in direction
    OUT = 1<<1U, ///< out direction
};

/**
 * @brief parameter struct, which can be used in PluginBase.SetParameter for complex parameter setting.
 *
 * @since 1.0
 * @version 1.0
 */
struct ParameterStruct {
    uint32_t direction {static_cast<uint8_t>(Direction::IN) |
        static_cast<uint8_t>(Direction::OUT)}; ///< direction of parameter, default is in and out
    int32_t streamIndex {
        -1}; ///< indicates stream that will be effected by this parameter, -1 means that all stream will be effected
    Tag tagId; ///< parameter tag id
    ValueType value; ///< value of the parameter
};
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PLUGIN_COMMON_TAGS_H
