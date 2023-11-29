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

#include "meta/meta.h"
#include <functional>
#include "common/log.h"

namespace OHOS {
namespace Media {
using namespace Plugin;

#define DEFINE_METADATA_SETTER_GETTER_FUNC(EnumTypeName, ExtTypeName)                       \
static bool Set##EnumTypeName(Meta& meta, const TagType& tag, ExtTypeName& value)           \
{                                                                                           \
    if (__is_enum(EnumTypeName)) {                                                          \
        meta.SetData(tag, EnumTypeName(value));                                             \
    } else {                                                                                \
        meta.SetData(tag, value);                                                           \
    }                                                                                       \
    return true;                                                                            \
}                                                                                           \
                                                                                            \
static bool Get##EnumTypeName(const Meta& meta, const TagType& tag, ExtTypeName& value)     \
{                                                                                           \
    EnumTypeName tmpValue;                                                                  \
    if (meta.GetData(tag, tmpValue)) {                                                      \
        value = static_cast<ExtTypeName>(tmpValue);                                         \
        return true;                                                                        \
    }                                                                                       \
    return false;                                                                           \
}

DEFINE_METADATA_SETTER_GETTER_FUNC(SrcInputType, int32_t)
DEFINE_METADATA_SETTER_GETTER_FUNC(AudioSampleFormat, int32_t)
DEFINE_METADATA_SETTER_GETTER_FUNC(VideoPixelFormat, int32_t)
DEFINE_METADATA_SETTER_GETTER_FUNC(MediaType, int32_t)
DEFINE_METADATA_SETTER_GETTER_FUNC(VideoH264Profile, int32_t)
DEFINE_METADATA_SETTER_GETTER_FUNC(VideoRotation, int32_t)
DEFINE_METADATA_SETTER_GETTER_FUNC(ColorPrimary, int32_t)
DEFINE_METADATA_SETTER_GETTER_FUNC(TransferCharacteristic, int32_t)
DEFINE_METADATA_SETTER_GETTER_FUNC(MatrixCoefficient, int32_t)
DEFINE_METADATA_SETTER_GETTER_FUNC(HEVCProfile, int32_t)
DEFINE_METADATA_SETTER_GETTER_FUNC(HEVCLevel, int32_t)
DEFINE_METADATA_SETTER_GETTER_FUNC(ChromaLocation, int32_t)
DEFINE_METADATA_SETTER_GETTER_FUNC(FileType, int32_t)

DEFINE_METADATA_SETTER_GETTER_FUNC(AudioChannelLayout, int64_t)

#define  DEFINE_METADATA_SETTER_GETTER(tag, EnumType) {tag, std::make_pair(Set##EnumType, Get##EnumType)}

using  MetaSetterFunction = std::function<bool(Meta&, const TagType&, int32_t&)>;
using  MetaGetterFunction = std::function<bool(const Meta&, const TagType&, int32_t&)>;

static std::map<TagType, std::pair<MetaSetterFunction, MetaGetterFunction>> g_metadataGetterSetterMap = {
    DEFINE_METADATA_SETTER_GETTER(Tag::SRC_INPUT_TYPE, SrcInputType),
    DEFINE_METADATA_SETTER_GETTER(Tag::AUDIO_SAMPLE_FORMAT, AudioSampleFormat),
    DEFINE_METADATA_SETTER_GETTER(Tag::VIDEO_PIXEL_FORMAT, VideoPixelFormat),
    DEFINE_METADATA_SETTER_GETTER(Tag::MEDIA_TYPE, MediaType),
    DEFINE_METADATA_SETTER_GETTER(Tag::VIDEO_H264_PROFILE, VideoH264Profile),
    DEFINE_METADATA_SETTER_GETTER(Tag::VIDEO_ROTATION, VideoRotation),
    DEFINE_METADATA_SETTER_GETTER(Tag::VIDEO_COLOR_PRIMARIES, ColorPrimary),
    DEFINE_METADATA_SETTER_GETTER(Tag::VIDEO_COLOR_TRC, TransferCharacteristic),
    DEFINE_METADATA_SETTER_GETTER(Tag::VIDEO_COLOR_MATRIX_COEFF, MatrixCoefficient),
    DEFINE_METADATA_SETTER_GETTER(Tag::VIDEO_H265_PROFILE, HEVCProfile),
    DEFINE_METADATA_SETTER_GETTER(Tag::VIDEO_H265_LEVEL, HEVCLevel),
    DEFINE_METADATA_SETTER_GETTER(Tag::VIDEO_CHROMA_LOCATION, ChromaLocation),
    DEFINE_METADATA_SETTER_GETTER(Tag::MEDIA_FILE_TYPE, FileType)
};

using  MetaSetterInt64Function = std::function<bool(Meta&, const TagType&, int64_t&)>;
using  MetaGetterInt64Function = std::function<bool(const Meta&, const TagType&, int64_t&)>;
static std::map<TagType, std::pair<MetaSetterInt64Function, MetaGetterInt64Function>> g_metadataGetterSetterInt64Map = {
        DEFINE_METADATA_SETTER_GETTER(Tag::AUDIO_CHANNEL_LAYOUT, AudioChannelLayout),
        DEFINE_METADATA_SETTER_GETTER(Tag::AUDIO_OUTPUT_CHANNEL_LAYOUT, AudioChannelLayout)
};

static std::vector<TagType> g_metadataBoolVector = {
    Tag::VIDEO_COLOR_RANGE,
    Tag::VIDEO_IS_HDR_VIVID
};

bool SetMetaData(Meta& meta, const TagType& tag, int32_t value)
{
    auto iter = g_metadataGetterSetterMap.find(tag);
    if (iter == g_metadataGetterSetterMap.end()) {
        if (std::find(g_metadataBoolVector.begin(), g_metadataBoolVector.end(), tag) != g_metadataBoolVector.end()) {
            meta.SetData(tag, value != 0 ? true : false);
            return true;
        }
        meta.SetData(tag, value);
        return true;
    }
    return iter->second.first(meta, tag, value);
}

bool GetMetaData(const Meta& meta, const TagType& tag, int32_t& value)
{
    auto iter = g_metadataGetterSetterMap.find(tag);
    if (iter == g_metadataGetterSetterMap.end()) {
        if (std::find(g_metadataBoolVector.begin(), g_metadataBoolVector.end(), tag) != g_metadataBoolVector.end()) {
            bool valueBool = false;
            FALSE_RETURN_V(meta.GetData(tag, valueBool), false);
            value = valueBool ? 1 : 0;
            return true;
        }
        return meta.GetData(tag, value);
    }
    return iter->second.second(meta, tag, value);
}

bool SetMetaData(Meta& meta, const TagType& tag, int64_t value)
{
    auto iter = g_metadataGetterSetterInt64Map.find(tag);
    if (iter == g_metadataGetterSetterInt64Map.end()) {
        meta.SetData(tag, value);
        return true;
    }
    return iter->second.first(meta, tag, value);
}

bool GetMetaData(const Meta& meta, const TagType& tag, int64_t& value)
{
    auto iter = g_metadataGetterSetterInt64Map.find(tag);
    if (iter == g_metadataGetterSetterInt64Map.end()) {
        return meta.GetData(tag, value);
    }
    return iter->second.second(meta, tag, value);
}

static Any defaultString = std::string();
static Any defaultUInt8 = (uint8_t)0;
static Any defaultInt32 = (int32_t)0;
static Any defaultInt64 = (int64_t)0;
static Any defaultUInt64 = (uint64_t)0;
static Any defaultDouble = (double)0.0;
static Any defaultBool = (bool) false;
static Any defaultSrcInputType = SrcInputType::UNKNOWN;
static Any defaultAudioSampleFormat = AudioSampleFormat::INVALID_WIDTH;
static Any defaultVideoPixelFormat = VideoPixelFormat::UNKNOWN;
static Any defaultMediaType = MediaType::UNKNOWN;
static Any defaultVideoH264Profile = VideoH264Profile::UNKNOWN;
static Any defaultVideoRotation = VideoRotation::VIDEO_ROTATION_0;
static Any defaultColorPrimary = ColorPrimary::COLOR_PRIMARY_BT2020;
static Any defaultTransferCharacteristic = TransferCharacteristic::TRANSFER_CHARACTERISTIC_BT1361;
static Any defaultMatrixCoefficient = MatrixCoefficient::MATRIX_COEFFICIENT_BT2020_CL;
static Any defaultHEVCProfile = HEVCProfile::HEVC_PROFILE_UNKNOW;
static Any defaultHEVCLevel = HEVCLevel::HEVC_LEVEL_UNKNOW;
static Any defaultChromaLocation = ChromaLocation::CHROMA_LOC_BOTTOM;
static Any defaultFileType = FileType::UNKNOW;

static Any defaultAudioChannelLayout = AudioChannelLayout::UNKNOWN_CHANNEL_LAYOUT;
static Any defaultAudioAacProfile = AudioAacProfile::ELD;
static Any defaultAudioAacStreamFormat = AudioAacStreamFormat::ADIF;
static Any defaultVectorUInt8 = std::vector<uint8_t>();
static Any defaultVectorVideoBitStreamFormat = std::vector<VideoBitStreamFormat>();
static std::map<TagType, const Any &> g_metadataDefaultValueMap = {
    {Tag::SRC_INPUT_TYPE, defaultSrcInputType},
    {Tag::AUDIO_SAMPLE_FORMAT, defaultAudioSampleFormat},
    {Tag::VIDEO_PIXEL_FORMAT, defaultVideoPixelFormat},
    {Tag::MEDIA_TYPE, defaultMediaType},
    {Tag::VIDEO_H264_PROFILE, defaultVideoH264Profile},
    {Tag::VIDEO_ROTATION, defaultVideoRotation},
    {Tag::VIDEO_COLOR_PRIMARIES, defaultColorPrimary},
    {Tag::VIDEO_COLOR_TRC, defaultTransferCharacteristic},
    {Tag::VIDEO_COLOR_MATRIX_COEFF, defaultMatrixCoefficient},
    {Tag::VIDEO_H265_PROFILE, defaultHEVCProfile},
    {Tag::VIDEO_H265_LEVEL, defaultHEVCLevel},
    {Tag::VIDEO_CHROMA_LOCATION, defaultChromaLocation},
    {Tag::MEDIA_FILE_TYPE, defaultFileType},
    // Int32
    {Tag::APP_PID, defaultInt32},
    {Tag::APP_TOKEN_ID, defaultInt32},
    {Tag::REQUIRED_IN_BUFFER_CNT, defaultInt32},
    {Tag::REQUIRED_IN_BUFFER_SIZE, defaultInt32},
    {Tag::REQUIRED_OUT_BUFFER_CNT, defaultInt32},
    {Tag::REQUIRED_OUT_BUFFER_SIZE, defaultInt32},
    {Tag::BUFFERING_SIZE, defaultInt32},
    {Tag::WATERLINE_HIGH, defaultInt32},
    {Tag::WATERLINE_LOW, defaultInt32},
    {Tag::AUDIO_CHANNEL_COUNT, defaultInt32},
    {Tag::AUDIO_SAMPLE_RATE, defaultInt32},
    {Tag::AUDIO_SAMPLE_PER_FRAME, defaultInt32},
    {Tag::AUDIO_OUTPUT_CHANNELS, defaultInt32},
    {Tag::AUDIO_MPEG_VERSION, defaultInt32},
    {Tag::AUDIO_MPEG_LAYER, defaultInt32},
    {Tag::AUDIO_AAC_LEVEL, defaultInt32},
    {Tag::AUDIO_MAX_INPUT_SIZE, defaultInt32},
    {Tag::AUDIO_MAX_OUTPUT_SIZE, defaultInt32},
    {Tag::VIDEO_WIDTH, defaultInt32},
    {Tag::VIDEO_HEIGHT, defaultInt32},
    {Tag::VIDEO_DELAY, defaultInt32},
    {Tag::VIDEO_MAX_SURFACE_NUM, defaultInt32},
    {Tag::VIDEO_H264_LEVEL, defaultInt32},
    {Tag::MEDIA_TRACK_COUNT, defaultInt32},
    {Tag::MEDIA_HAS_VIDEO, defaultInt32},
    {Tag::MEDIA_HAS_AUDIO, defaultInt32},
    {Tag::AUDIO_AAC_IS_ADTS, defaultInt32},
    {Tag::AUDIO_COMPRESSION_LEVEL, defaultInt32},
    {Tag::AUDIO_BITS_PER_CODED_SAMPLE, defaultInt32},
    {Tag::MEDIA_TRACK_COUNT, defaultInt32},
    {Tag::MEDIA_HAS_VIDEO, defaultInt32},
    {Tag::MEDIA_HAS_AUDIO, defaultInt32},
    // String
    {Tag::MIME_TYPE, defaultString},
    {Tag::MEDIA_FILE_URI, defaultString},
    {Tag::MEDIA_TITLE, defaultString},
    {Tag::MEDIA_ARTIST, defaultString},
    {Tag::MEDIA_LYRICIST, defaultString},
    {Tag::MEDIA_ALBUM, defaultString},
    {Tag::MEDIA_ALBUM_ARTIST, defaultString},
    {Tag::MEDIA_DATE, defaultString},
    {Tag::MEDIA_COMMENT, defaultString},
    {Tag::MEDIA_GENRE, defaultString},
    {Tag::MEDIA_COPYRIGHT, defaultString},
    {Tag::MEDIA_LANGUAGE, defaultString},
    {Tag::MEDIA_DESCRIPTION, defaultString},
    {Tag::USER_TIME_SYNC_RESULT, defaultString},
    {Tag::USER_AV_SYNC_GROUP_INFO, defaultString},
    {Tag::USER_SHARED_MEMORY_FD, defaultString},
    {Tag::MEDIA_AUTHOR, defaultString},
    {Tag::MEDIA_COMPOSER, defaultString},
    {Tag::MEDIA_LYRICS, defaultString},
    // Double
    {Tag::VIDEO_CAPTURE_RATE, defaultDouble},
    {Tag::VIDEO_FRAME_RATE, defaultDouble},
    // Bool
    {Tag::VIDEO_COLOR_RANGE, defaultBool},
    {Tag::VIDEO_IS_HDR_VIVID, defaultBool},
    // UInt64
    {Tag::MEDIA_FILE_SIZE, defaultUInt64},
    {Tag::MEDIA_POSITION, defaultUInt64},
    // Int64
    {Tag::APP_FULL_TOKEN_ID, defaultInt64},
    {Tag::MEDIA_DURATION, defaultInt64},
    {Tag::MEDIA_BITRATE, defaultInt64},
    {Tag::MEDIA_START_TIME, defaultInt64},
    {Tag::USER_FRAME_PTS, defaultInt64},
    {Tag::USER_PUSH_DATA_TIME, defaultInt64},
    // AudioChannelLayout UINT64_T
    {Tag::AUDIO_CHANNEL_LAYOUT, defaultAudioChannelLayout},
    {Tag::AUDIO_OUTPUT_CHANNEL_LAYOUT, defaultAudioChannelLayout},
    // AudioAacProfile UInt8
    {Tag::AUDIO_AAC_PROFILE, defaultAudioAacProfile},
    // AudioAacStreamFormat UInt8
    {Tag::AUDIO_AAC_STREAM_FORMAT, defaultAudioAacStreamFormat},
    // vector<uint8_t>
    {Tag::MEDIA_CODEC_CONFIG, defaultVectorUInt8},
    {Tag::MEDIA_COVER, defaultVectorUInt8},
    // vector<Plugin::VideoBitStreamFormat>
    {Tag::VIDEO_BIT_STREAM_FORMAT, defaultVectorVideoBitStreamFormat}};

Any GetDefaultAnyValue(const TagType& tag)
{
    auto iter = g_metadataDefaultValueMap.find(tag);
    if (iter == g_metadataDefaultValueMap.end()) {
        return defaultString; //Default String type
    }
    return iter->second;
}

bool Meta::ToParcel(MessageParcel &parcel) const
{
    MessageParcel metaParcel;
    int32_t metaSize = 0;
    bool ret = true;
    for (auto it = begin(); it != end(); ++it) {
        ++metaSize;
        ret &= metaParcel.WriteString(it->first);
        ret &= it->second.ToParcel(metaParcel);
        if (!ret) {
            MEDIA_LOG_E("fail to Marshalling Key: " PUBLIC_LOG_S, it->first.c_str());
            return false;
        }
        MEDIA_LOG_D("success to Marshalling Key: " PUBLIC_LOG_S, it->first.c_str());
    }
    if (ret) {
        ret &= parcel.WriteInt32(metaSize);
        ret &= parcel.Append(metaParcel);
    }
    return ret;
}

bool Meta::FromParcel(MessageParcel &parcel)
{
    map_.clear();
    int32_t size = parcel.ReadInt32();
    for (int32_t index = 0; index < size; index++) {
        std::string key = parcel.ReadString();
        Any value = GetDefaultAnyValue(key); //Init Default Value
        if (value.FromParcel(parcel)) {
            map_[key] = value;
        } else {
            MEDIA_LOG_E("fail to Unmarshalling Key: %{public}s", key.c_str());
            return false;
        }
        MEDIA_LOG_D("success to Unmarshalling Key: %{public}s", key.c_str());
    }
    return true;
}
}
} // namespace OHOS