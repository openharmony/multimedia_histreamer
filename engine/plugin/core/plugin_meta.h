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

#ifndef HISTREAMER_PLUGIN_META_H
#define HISTREAMER_PLUGIN_META_H

#include "common/plugin_tags.h"
#include "common/plugin_types.h"

namespace OHOS {
namespace Media {
namespace Plugin {
template <typename E>
constexpr typename std::underlying_type<E>::type to_underlying(E e) noexcept
{
    return static_cast<typename std::underlying_type<E>::type>(e);
}

/**
 * Meta ID is used to describe the metadata of media files or data streams.
 * All Meta ID must come from Tag.
 *
 * For details about the definition and usage, to see enum Tag in file plugin_tags.h.
 */
enum struct MetaID : uint32_t {
    MIME = to_underlying(Tag::MIME),
    STREAM_INDEX = to_underlying(Tag::STREAM_INDEX),
    MEDIA_CODEC_CONFIG = to_underlying(Tag::MEDIA_CODEC_CONFIG),

    AUDIO_CHANNELS = to_underlying(Tag::AUDIO_CHANNELS),
    AUDIO_SAMPLE_RATE = to_underlying(Tag::AUDIO_SAMPLE_RATE),
    AUDIO_SAMPLE_FORMAT = to_underlying(Tag::AUDIO_SAMPLE_FORMAT),
    AUDIO_SAMPLE_PRE_FRAME = to_underlying(Tag::AUDIO_SAMPLE_PRE_FRAME),
    AUDIO_CHANNEL_LAYOUT = to_underlying(Tag::AUDIO_CHANNEL_LAYOUT),

    MEDIA_TITLE = to_underlying(Tag::MEDIA_TITLE),
    MEDIA_ARTIST = to_underlying(Tag::MEDIA_ARTIST),
    MEDIA_LYRICIST = to_underlying(Tag::MEDIA_LYRICIST),
    MEDIA_ALBUM = to_underlying(Tag::MEDIA_ALBUM),
    MEDIA_ALBUM_ARTIST = to_underlying(Tag::MEDIA_ALBUM_ARTIST),
    MEDIA_DATE = to_underlying(Tag::MEDIA_DATE),
    MEDIA_COMMENT = to_underlying(Tag::MEDIA_COMMENT),
    MEDIA_GENRE = to_underlying(Tag::MEDIA_GENRE),
    MEDIA_DESCRIPTION = to_underlying(Tag::MEDIA_DESCRIPTION),
    MEDIA_COPYRIGHT = to_underlying(Tag::MEDIA_COPYRIGHT),
    MEDIA_LANGUAGE = to_underlying(Tag::MEDIA_LANGUAGE),
    MEDIA_LYRICS = to_underlying(Tag::MEDIA_LYRICS),
    MEDIA_DURATION = to_underlying(Tag::MEDIA_DURATION),
    MEDIA_BITRATE = to_underlying(Tag::MEDIA_BITRATE),
    MEDIA_FILE_EXTENSION = to_underlying(Tag::MEDIA_FILE_EXTENSION),
    MEDIA_FILE_SIZE = to_underlying(Tag::MEDIA_FILE_SIZE),

    AUDIO_MPEG_VERSION = to_underlying(Tag::AUDIO_MPEG_VERSION),
    AUDIO_MPEG_LAYER = to_underlying(Tag::AUDIO_MPEG_LAYER),
    AUDIO_AAC_PROFILE = to_underlying(Tag::AUDIO_AAC_PROFILE),
    AUDIO_AAC_LEVEL = to_underlying(Tag::AUDIO_AAC_LEVEL),
    AUDIO_AAC_STREAM_FORMAT = to_underlying(Tag::AUDIO_AAC_STREAM_FORMAT),

    VIDEO_WIDTH = to_underlying(Tag::VIDEO_WIDTH),
    VIDEO_HEIGHT = to_underlying(Tag::VIDEO_HEIGHT),
    VIDEO_PIXEL_FORMAT = to_underlying(Tag::VIDEO_PIXEL_FORMAT),
};
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PLUGIN_META_H
