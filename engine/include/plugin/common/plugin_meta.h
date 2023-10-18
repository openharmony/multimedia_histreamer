/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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

#ifndef HISTREAMER_PLUGIN_TAG_VALUE_MAP_H
#define HISTREAMER_PLUGIN_TAG_VALUE_MAP_H

#include "plugin/common/plugin_audio_tags.h"
#include "plugin/common/plugin_source_tags.h"
#include "plugin/common/plugin_tags.h"
#include "plugin/common/plugin_types.h"
#include "plugin/common/plugin_video_tags.h"
#if !defined(OHOS_LITE) && defined(VIDEO_SUPPORT)
#include "plugin/common/surface_allocator.h"
#endif

namespace OHOS {
namespace Media {
namespace Plugin {
#define DEFINE_INSERT_GET_FUNC(condition, ValueType)             \
        template<Tag tag>                                        \
        inline typename std::enable_if<(condition), bool>::type  \
        Set(ValueType value)                                     \
        {                                                        \
            auto iter = map_.find(tag);                          \
            if (iter != map_.end()) {                            \
                map_.erase(iter++);                              \
            }                                                    \
            map_.insert(std::make_pair(tag, value));             \
            return true;                                         \
        }                                                        \
        template<Tag tag>                                        \
        inline typename std::enable_if<(condition), bool>::type  \
        Get(ValueType& value) const                              \
        {                                                        \
            if (map_.count(tag) == 0) {                          \
                return false;                                    \
            }                                                    \
            return AnyCast<ValueType>(&map_.at(tag), value);     \
        }


using MapIt = std::map<Tag, Any>::const_iterator;
class Meta {
public:
#if !defined(OHOS_LITE) && defined(VIDEO_SUPPORT)
    DEFINE_INSERT_GET_FUNC(tag == Tag::BUFFER_ALLOCATOR or tag == Tag::VIDEO_SURFACE,
                           std::shared_ptr<SurfaceAllocator>);
#endif
    DEFINE_INSERT_GET_FUNC(tag == Tag::SRC_INPUT_TYPE, SrcInputType);
    DEFINE_INSERT_GET_FUNC(tag == Tag::MEDIA_CODEC_CONFIG, std::vector<uint8_t>);
    DEFINE_INSERT_GET_FUNC(
        tag == Tag::AUDIO_CHANNEL_LAYOUT or
        tag == Tag::AUDIO_OUTPUT_CHANNEL_LAYOUT, AudioChannelLayout);
    DEFINE_INSERT_GET_FUNC(tag == Tag::AUDIO_SAMPLE_FORMAT, AudioSampleFormat);
    DEFINE_INSERT_GET_FUNC(tag == Tag::AUDIO_AAC_PROFILE, AudioAacProfile);
    DEFINE_INSERT_GET_FUNC(tag == Tag::AUDIO_AAC_STREAM_FORMAT, AudioAacStreamFormat);
    DEFINE_INSERT_GET_FUNC(tag == Tag::VIDEO_PIXEL_FORMAT, VideoPixelFormat);
    DEFINE_INSERT_GET_FUNC(tag == Tag::MEDIA_SEEKABLE, Seekable);
    DEFINE_INSERT_GET_FUNC(tag == Tag::MEDIA_TYPE, MediaType);
    DEFINE_INSERT_GET_FUNC(tag == Tag::VIDEO_BIT_STREAM_FORMAT, std::vector<VideoBitStreamFormat>);
    DEFINE_INSERT_GET_FUNC(tag == Tag::VIDEO_H264_PROFILE, VideoH264Profile);
    DEFINE_INSERT_GET_FUNC(
        tag == Tag::TRACK_ID or
        tag == Tag::REQUIRED_OUT_BUFFER_CNT or
        tag == Tag::BUFFERING_SIZE or
        tag == Tag::WATERLINE_HIGH or
        tag == Tag::WATERLINE_LOW or
        tag == Tag::AUDIO_CHANNELS or
        tag == Tag::AUDIO_SAMPLE_RATE or
        tag == Tag::AUDIO_SAMPLE_PER_FRAME or
        tag == Tag::AUDIO_OUTPUT_CHANNELS or
        tag == Tag::AUDIO_MPEG_VERSION or
        tag == Tag::AUDIO_MPEG_LAYER or
        tag == Tag::AUDIO_AAC_LEVEL or
        tag == Tag::VIDEO_WIDTH or
        tag == Tag::VIDEO_HEIGHT or
        tag == Tag::VIDEO_FRAME_RATE or
        tag == Tag::VIDEO_MAX_SURFACE_NUM or
        tag == Tag::VIDEO_H264_LEVEL or
        tag == Tag::BITS_PER_CODED_SAMPLE or
        tag == Tag::USER_FRAME_NUMBER, uint32_t);
    DEFINE_INSERT_GET_FUNC(
        tag == Tag::MEDIA_DURATION or
        tag == Tag::MEDIA_BITRATE or
        tag == Tag::MEDIA_START_TIME or
        tag == Tag::USER_FRAME_PTS or
        tag == Tag::USER_PUSH_DATA_TIME, int64_t);

    DEFINE_INSERT_GET_FUNC(
        tag == Tag::MEDIA_FILE_SIZE or
        tag == Tag::MEDIA_POSITION, uint64_t);
    DEFINE_INSERT_GET_FUNC(
        tag == Tag::VIDEO_CAPTURE_RATE, double);
    DEFINE_INSERT_GET_FUNC(
        tag == Tag::MIME or
        tag == Tag::MEDIA_FILE_URI or
        tag == Tag::MEDIA_TITLE or
        tag == Tag::MEDIA_ARTIST or
        tag == Tag::MEDIA_LYRICIST or
        tag == Tag::MEDIA_ALBUM or
        tag == Tag::MEDIA_ALBUM_ARTIST or
        tag == Tag::MEDIA_DATE or
        tag == Tag::MEDIA_COMMENT or
        tag == Tag::MEDIA_GENRE or
        tag == Tag::MEDIA_COPYRIGHT or
        tag == Tag::MEDIA_LANGUAGE or
        tag == Tag::MEDIA_DESCRIPTION or
        tag == Tag::USER_TIME_SYNC_RESULT or
        tag == Tag::USER_AV_SYNC_GROUP_INFO or
        tag == Tag::USER_SHARED_MEMORY_FD or
        tag == Tag::MEDIA_LYRICS, std::string);
    Meta& operator=(const Meta& other)
    {
        map_ = other.map_;
        return *this;
    }

    Meta() {
    };

    Meta(const Meta& other)
    {
        map_ = other.map_;
    }

    ValueType& operator[](const Tag &tag)
    {
        return map_[tag];
    }

    MapIt begin() const // to support for (auto e : Meta), must use begin/end name
    {
        return map_.cbegin();
    }

    MapIt end() const
    {
        return map_.cend();
    }

    void Clear()
    {
        map_.clear();
    }

    MapIt Find(Tag tag) const
    {
        return map_.find(tag);
    }

    bool Empty() const
    {
        return map_.empty();
    }

    template <typename T>
    void SetData(Plugin::Tag id, const T& value)
    {
        map_[id] = value;
    }

    template <typename T>
    bool GetData(Plugin::Tag id, T& value) const
    {
        auto ite = map_.find(id);
        if (ite == map_.end() || !Plugin::Any::IsSameTypeWith<T>(ite->second)) {
            return false;
        }
        value = Plugin::AnyCast<T>(ite->second);
        return true;
    }

    bool GetData(Plugin::Tag id, Plugin::ValueType& value) const
    {
        auto ite = map_.find(id);
        if (ite == map_.end()) {
            return false;
        }
        value = ite->second;
        return true;
    }

    void Remove(Plugin::Tag id)
    {
        auto ite = map_.find(id);
        if (ite != map_.end()) {
            map_.erase(ite);
        }
    }

    void GetKeys(std::vector<Tag>& keys) const
    {
        int cnt = 0;
        keys.resize(map_.size());
        for (const auto& tmp : map_) {
            keys[cnt++] = tmp.first;
        }
    }
private:
    std::map<Tag, Any> map_;
};
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PLUGIN_TAG_VALUE_MAP_H
