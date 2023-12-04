/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef MEDIA_FOUNDATION_META_H
#define MEDIA_FOUNDATION_META_H
#include <map>
#include "meta/meta_key.h"
#include "meta/audio_types.h"
#include "meta/media_types.h"
#include "meta/video_types.h"
#include "meta/source_types.h"
#include "meta/mime_type.h"
#include "meta/any.h" // NOLINT

namespace OHOS {
namespace Media {
/**
 * @brief GetDefaultAnyValue used for Application to get Any type default value from Meta Object.
 * @implNote In order to get default Any value from Meta Object.
 * We use metadataDefaultValueMap to get the right getDefaultVal function.
 * @return Returns Any type default value, returns <b>Any(string())</b> if no match.
 * @example GetDefaultAnyValue("media.file.type");
 */
extern Any GetDefaultAnyValue(const TagType& tag);

#define DECLARE_INFO_CLASS                                   \
    template<TagTypeCharSeq tagCharSeq, class Enable = void> \
    class ValueInfo {                                        \
        public:                                              \
            typedef int32_t type;                            \
    };

#define DEFINE_INSERT_GET_FUNC(condition, Any, eValueType)   \
    template<TagTypeCharSeq tagCharSeq>                      \
    inline typename std::enable_if<(condition), bool>::type  \
    Set(Any value)                                           \
    {                                                        \
        TagType tag = tagCharSeq;                            \
        auto iter = map_.find(tag);                          \
        if (iter != map_.end()) {                            \
            map_.erase(iter++);                              \
        }                                                    \
        map_.insert(std::make_pair(tag, value));             \
        return true;                                         \
    }                                                        \
                                                             \
    template<TagTypeCharSeq tagCharSeq>                      \
    inline typename std::enable_if<(condition), bool>::type  \
    Get(Any& value) const                                    \
    {                                                        \
        TagType tag = tagCharSeq;                            \
        if (map_.count(tag) == 0) {                          \
            return false;                                    \
        }                                                    \
        return AnyCast<Any>(&map_.at(tag), value);           \
    }                                                        \
                                                             \
    template<TagTypeCharSeq tagCharSeq>                      \
    class ValueInfo<tagCharSeq, typename std::enable_if<condition>::type> \
    {                                                        \
        public:                                              \
            typedef Any type;                                \
    };                                                       \
    template<TagTypeCharSeq tagCharSeq>                      \
    inline typename std::enable_if<condition, ValueType>::type \
    GetValueType()                                           \
    {                                                        \
        return eValueType;                                   \
    }

using MapIt = std::map<TagType, Any>::const_iterator;
class Meta {
public:
    enum struct ValueType : int32_t {
        BOOL,
        UINT8_T,
        INT32_T,
        UINT32_T,
        INT64_T,
        UINT64_T,
        DOUBLE,
        VECTOR_UINT8,
        VECTOR_UINT32,
        STRING
    };

    DECLARE_INFO_CLASS

    DEFINE_INSERT_GET_FUNC(tagCharSeq == Tag::SRC_INPUT_TYPE, Plugin::SrcInputType, ValueType::INT32_T);
    DEFINE_INSERT_GET_FUNC(tagCharSeq == Tag::MEDIA_CODEC_CONFIG or
                           tagCharSeq == Tag::MEDIA_COVER or
                           tagCharSeq == Tag::AUDIO_VORBIS_IDENTIFICATION_HEADER or
                           tagCharSeq == Tag::AUDIO_VORBIS_SETUP_HEADER,
                           std::vector<uint8_t>, ValueType::VECTOR_UINT8);
    DEFINE_INSERT_GET_FUNC(tagCharSeq == Tag::AUDIO_CHANNEL_LAYOUT or
                           tagCharSeq == Tag::AUDIO_OUTPUT_CHANNEL_LAYOUT,
                           Plugin::AudioChannelLayout, ValueType::INT64_T);
    DEFINE_INSERT_GET_FUNC(tagCharSeq == Tag::AUDIO_SAMPLE_FORMAT, Plugin::AudioSampleFormat,
                           ValueType::INT32_T);
    DEFINE_INSERT_GET_FUNC(tagCharSeq == Tag::AUDIO_AAC_PROFILE, Plugin::AudioAacProfile, ValueType::UINT8_T);
    DEFINE_INSERT_GET_FUNC(tagCharSeq == Tag::AUDIO_AAC_STREAM_FORMAT, Plugin::AudioAacStreamFormat,
                           ValueType::UINT8_T);
    DEFINE_INSERT_GET_FUNC(tagCharSeq == Tag::VIDEO_PIXEL_FORMAT, Plugin::VideoPixelFormat,
                           ValueType::INT32_T);
//    DEFINE_INSERT_GET_FUNC(tagCharSeq == Tag::MEDIA_SEEKABLE), Plugin::Seekable);
    DEFINE_INSERT_GET_FUNC(tagCharSeq == Tag::MEDIA_TYPE, Plugin::MediaType, ValueType::INT32_T);
    DEFINE_INSERT_GET_FUNC(tagCharSeq == Tag::VIDEO_BIT_STREAM_FORMAT, std::vector<Plugin::VideoBitStreamFormat>,
                           ValueType::VECTOR_UINT32);
    DEFINE_INSERT_GET_FUNC(tagCharSeq == Tag::VIDEO_H264_PROFILE, Plugin::VideoH264Profile,
                           ValueType::INT32_T);
    DEFINE_INSERT_GET_FUNC(tagCharSeq == Tag::VIDEO_ROTATION, Plugin::VideoRotation, ValueType::INT32_T);
    DEFINE_INSERT_GET_FUNC(tagCharSeq == Tag::VIDEO_COLOR_PRIMARIES, Plugin::ColorPrimary,
                           ValueType::INT32_T);
    DEFINE_INSERT_GET_FUNC(tagCharSeq == Tag::VIDEO_COLOR_TRC, Plugin::TransferCharacteristic,
                           ValueType::INT32_T);
    DEFINE_INSERT_GET_FUNC(tagCharSeq == Tag::VIDEO_COLOR_MATRIX_COEFF, Plugin::MatrixCoefficient,
                           ValueType::INT32_T);
    DEFINE_INSERT_GET_FUNC(tagCharSeq == Tag::VIDEO_ENCODE_BITRATE_MODE, Plugin::VideoEncodeBitrateMode, ValueType::INT32_T);
    DEFINE_INSERT_GET_FUNC(tagCharSeq == Tag::VIDEO_COLOR_RANGE or 
                           tagCharSeq == Tag::VIDEO_REQUEST_I_FRAME or
                           tagCharSeq == Tag::VIDEO_IS_HDR_VIVID or
                           tagCharSeq == Tag::MEDIA_HAS_VIDEO or
                           tagCharSeq == Tag::MEDIA_HAS_AUDIO or
                           tagCharSeq == Tag::MEDIA_END_OF_STREAM, bool, ValueType::BOOL);
    DEFINE_INSERT_GET_FUNC(tagCharSeq == Tag::VIDEO_H265_PROFILE, Plugin::HEVCProfile, ValueType::INT32_T);
    DEFINE_INSERT_GET_FUNC(tagCharSeq == Tag::VIDEO_H265_LEVEL, Plugin::HEVCLevel, ValueType::INT32_T);
    DEFINE_INSERT_GET_FUNC(tagCharSeq == Tag::VIDEO_CHROMA_LOCATION, Plugin::ChromaLocation, ValueType::INT32_T);
    DEFINE_INSERT_GET_FUNC(tagCharSeq == Tag::APP_UID or
        tagCharSeq == Tag::APP_PID or
        tagCharSeq == Tag::APP_TOKEN_ID or
        tagCharSeq == Tag::REQUIRED_IN_BUFFER_CNT or
        tagCharSeq == Tag::REQUIRED_IN_BUFFER_SIZE or
        tagCharSeq == Tag::REQUIRED_OUT_BUFFER_CNT or
        tagCharSeq == Tag::REQUIRED_OUT_BUFFER_SIZE or
        tagCharSeq == Tag::BUFFERING_SIZE or
        tagCharSeq == Tag::WATERLINE_HIGH or
        tagCharSeq == Tag::WATERLINE_LOW or
        tagCharSeq == Tag::AUDIO_CHANNEL_COUNT or
        tagCharSeq == Tag::AUDIO_SAMPLE_RATE or
        tagCharSeq == Tag::AUDIO_SAMPLE_PER_FRAME or
        tagCharSeq == Tag::AUDIO_OUTPUT_CHANNELS or
        tagCharSeq == Tag::AUDIO_MPEG_VERSION or
        tagCharSeq == Tag::AUDIO_MPEG_LAYER or
        tagCharSeq == Tag::AUDIO_AAC_LEVEL or
        tagCharSeq == Tag::AUDIO_MAX_INPUT_SIZE or
        tagCharSeq == Tag::AUDIO_MAX_OUTPUT_SIZE or
        tagCharSeq == Tag::VIDEO_WIDTH or
        tagCharSeq == Tag::VIDEO_HEIGHT or
        tagCharSeq == Tag::VIDEO_DELAY or
        tagCharSeq == Tag::VIDEO_MAX_SURFACE_NUM or
        tagCharSeq == Tag::VIDEO_H264_LEVEL or
        tagCharSeq == Tag::MEDIA_TRACK_COUNT or
        tagCharSeq == Tag::AUDIO_AAC_IS_ADTS or
        tagCharSeq == Tag::AUDIO_COMPRESSION_LEVEL or
        tagCharSeq == Tag::AUDIO_BITS_PER_CODED_SAMPLE or
        tagCharSeq == Tag::REGULAR_TRACK_ID or
        tagCharSeq == Tag::VIDEO_SCALE_TYPE or 
        tagCharSeq == Tag::VIDEO_I_FRAME_INTERVAL or
        tagCharSeq == Tag::MEDIA_PROFILE or
        tagCharSeq == Tag::VIDEO_ENCODE_QUALITY or
        tagCharSeq == Tag::AUDIO_AAC_SBR or
        tagCharSeq == Tag::AUDIO_FLAC_COMPLIANCE_LEVEL or
        tagCharSeq == Tag::MEDIA_LEVEL, int32_t, ValueType::INT32_T);

    DEFINE_INSERT_GET_FUNC(
        tagCharSeq == Tag::APP_FULL_TOKEN_ID or
        tagCharSeq == Tag::MEDIA_DURATION or
        tagCharSeq == Tag::MEDIA_BITRATE or
        tagCharSeq == Tag::MEDIA_START_TIME or
        tagCharSeq == Tag::USER_FRAME_PTS or
        tagCharSeq == Tag::USER_PUSH_DATA_TIME or
        tagCharSeq == Tag::MEDIA_FILE_SIZE or
        tagCharSeq == Tag::MEDIA_POSITION or
        tagCharSeq == Tag::MEDIA_TIME_STAMP, int64_t, ValueType::INT64_T);

    DEFINE_INSERT_GET_FUNC(tagCharSeq == Tag::VIDEO_FRAME_RATE or
        tagCharSeq == Tag::VIDEO_CAPTURE_RATE, double, ValueType::DOUBLE);
    DEFINE_INSERT_GET_FUNC(tagCharSeq == Tag::MEDIA_FILE_TYPE, Plugin::FileType, ValueType::INT32_T);
    DEFINE_INSERT_GET_FUNC(
        tagCharSeq == Tag::MIME_TYPE or
        tagCharSeq == Tag::MEDIA_FILE_URI or
        tagCharSeq == Tag::MEDIA_TITLE or
        tagCharSeq == Tag::MEDIA_ARTIST or
        tagCharSeq == Tag::MEDIA_LYRICIST or
        tagCharSeq == Tag::MEDIA_ALBUM or
        tagCharSeq == Tag::MEDIA_ALBUM_ARTIST or
        tagCharSeq == Tag::MEDIA_DATE or
        tagCharSeq == Tag::MEDIA_COMMENT or
        tagCharSeq == Tag::MEDIA_GENRE or
        tagCharSeq == Tag::MEDIA_COPYRIGHT or
        tagCharSeq == Tag::MEDIA_LANGUAGE or
        tagCharSeq == Tag::MEDIA_DESCRIPTION or
        tagCharSeq == Tag::USER_TIME_SYNC_RESULT or
        tagCharSeq == Tag::USER_AV_SYNC_GROUP_INFO or
        tagCharSeq == Tag::USER_SHARED_MEMORY_FD or
        tagCharSeq == Tag::MEDIA_AUTHOR or
        tagCharSeq == Tag::MEDIA_COMPOSER or
        tagCharSeq == Tag::MEDIA_LYRICS or
        tagCharSeq == Tag::MEDIA_CODEC_NAME, std::string, ValueType::STRING);

    Meta &operator=(const Meta &other)
    {
        map_ = other.map_;
        return *this;
    }

    Meta &operator=(Meta &&other)
    {
        swap(map_, other.map_);
        return *this;
    }

    Meta() {
    };

    Meta(const Meta &other)
    {
        map_ = other.map_;
    }

    Meta(Meta &&other)
    {
        swap(map_, other.map_);
    }

    Any& operator[](const TagType& tag)
    {
        return map_[tag];
    }

    Any& operator[](TagTypeCharSeq tag)
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

    MapIt Find(TagType tag) const
    {
        return map_.find(tag);
    }

    MapIt Find(TagTypeCharSeq tag) const
    {
        return map_.find(tag);
    }

    bool Empty() const
    {
        return map_.empty();
    }

    template <typename T>
    void SetData(const TagType& tag, const T& value)
    {
        map_[tag] = value;
    }

    template <typename T>
    void SetData(TagTypeCharSeq tag, const T& value)
    {
        map_[tag] = value;
    }

    template <int N>
    void SetData(const TagType &tag, char const (&value)[N])
    {
        std::string strValue = value;
        map_[tag] = std::move(strValue);
    }

    template <int N>
    void SetData(TagTypeCharSeq tag, char const (&value)[N])
    {
        std::string strValue = value;
        map_[tag] = std::move(strValue);
    }

    template <typename T>
    bool GetData(const TagType& tag, T &value) const
    {
        auto iter = map_.find(tag);
        if (iter == map_.end() || !Any::IsSameTypeWith<T>(iter->second)) {
            return false;
        }
        value = AnyCast<T>(iter->second);
        return true;
    }

    template <typename T>
    bool GetData(TagTypeCharSeq tag, T &value) const
    {
        auto iter = map_.find(tag);
        if (iter == map_.end() || !Any::IsSameTypeWith<T>(iter->second)) {
            return false;
        }
        value = AnyCast<T>(iter->second);
        return true;
    }

    void Remove(const TagType& tag)
    {
        auto iter = map_.find(tag);
        if (iter != map_.end()) {
            map_.erase(iter);
        }
    }

    void Remove(TagTypeCharSeq tag)
    {
        auto iter = map_.find(tag);
        if (iter != map_.end()) {
            map_.erase(iter);
        }
    }

    void GetKeys(std::vector<TagType>& keys) const
    {
        int cnt = 0;
        keys.resize(map_.size());
        for (const auto& tmp : map_) {
            keys[cnt++] = tmp.first;
        }
    }

    bool ToParcel(MessageParcel &parcel) const;
    bool FromParcel(MessageParcel &parcel);

private:
    std::map<TagType, Any> map_;
};

/**
 * @brief SetMetaData only used for Application interface OH_AVFormat to set Enum value into Meta Object.
 * @implNote In order to set value(int32_t type) to Meta Object, should convert int32_t value to correct EnumType then
 * save to Any object. We use metadataGetterSetterMap to get the right setter function.
 * @return Returns operator status, <b>True</b> if Set Success.
 * returns <b>False</b> otherwise.
 * @example OHOS::Media::SetMetaData(meta, "audio.aac.profile", value);
 */
bool SetMetaData(Meta& meta, const TagType& tag, int32_t value);

/**
 * @brief GetMetaData only used for Application interface OH_AVFormat to get Enum value from Meta Object.
 * @implNote In order to get value(Enum type) from Meta Object, should use correct Enum type to get value from Any
 * object. We use metadataGetterSetterMap to get the right getter function.
 * @return Returns operator status, <b>True</b> if Get Success.
 * returns <b>False</b> otherwise.
 * @example OHOS::Media::GetMetaData(meta, "audio.aac.profile", value);
 */
bool GetMetaData(const Meta& meta, const TagType& tag, int32_t& value);
/**
 * @brief SetMetaData only used for Application interface OH_AVFormat to set Enum value into Meta Object.
 * @implNote In order to set value(int64_t type) to Meta Object, should convert int32_t value to correct EnumType then
 * save to Any object. We use metadataGetterSetterMap to get the right setter function.
 * @return Returns operator status, <b>True</b> if Set Success.
 * returns <b>False</b> otherwise.
 * @example OHOS::Media::SetMetaData(meta, "audio.aac.profile", value);
 */
bool SetMetaData(Meta& meta, const TagType& tag, int64_t value);

/**
 * @brief GetMetaData only used for Application interface OH_AVFormat to get Enum value from Meta Object.
 * @implNote In order to get value(Enum type) from Meta Object, should use correct Enum type to get value from Any
 * object. We use metadataGetterSetterMap to get the right getter function.
 * @return Returns operator status, <b>True</b> if Get Success.
 * returns <b>False</b> otherwise.
 * @example OHOS::Media::GetMetaData(meta, "audio.aac.profile", value);
 */
bool GetMetaData(const Meta& meta, const TagType& tag, int64_t& value);
} // namespace Media
} // namespace OHOS
#endif // MEDIA_FOUNDATION_META_H