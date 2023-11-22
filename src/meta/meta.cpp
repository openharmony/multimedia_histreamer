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

namespace OHOS {
namespace Media {
using namespace Plugin;

#define DEFINE_METADATA_SETTER_GETTER_FUNC(EnumType)                                        \
static bool Set_##EnumType(Meta& meta, const TagType& tag, int32_t& value)            \
{                                                                                           \
    meta.SetData(tag, EnumType(value));                                                     \
    return true;                                                                            \
}                                                                                           \
                                                                                            \
static bool Get_##EnumType(const Meta& meta, const TagType& tag, int32_t& value)            \
{                                                                                           \
    EnumType tmpValue;                                                                      \
    if (meta.GetData(tag, tmpValue)) {                                                      \
        value = static_cast<int32_t>(tmpValue);                                               \
        return true;                                                                        \
    }                                                                                       \
    return false;                                                                           \
}

DEFINE_METADATA_SETTER_GETTER_FUNC(SrcInputType)
DEFINE_METADATA_SETTER_GETTER_FUNC(AudioSampleFormat)
DEFINE_METADATA_SETTER_GETTER_FUNC(VideoPixelFormat)
DEFINE_METADATA_SETTER_GETTER_FUNC(MediaType)
DEFINE_METADATA_SETTER_GETTER_FUNC(VideoH264Profile)
DEFINE_METADATA_SETTER_GETTER_FUNC(VideoRotation)
DEFINE_METADATA_SETTER_GETTER_FUNC(ColorPrimary)
DEFINE_METADATA_SETTER_GETTER_FUNC(TransferCharacteristic)
DEFINE_METADATA_SETTER_GETTER_FUNC(MatrixCoefficient)
DEFINE_METADATA_SETTER_GETTER_FUNC(HEVCProfile)
DEFINE_METADATA_SETTER_GETTER_FUNC(HEVCLevel)
DEFINE_METADATA_SETTER_GETTER_FUNC(ChromaLocation)
DEFINE_METADATA_SETTER_GETTER_FUNC(FileType)

#define  DEFINE_METADATA_SETTER_GETTER(tag, EnumType) {tag, std::make_pair(Set_##EnumType, Get_##EnumType)}

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


const Any g_defaultString = std::string();
const Any g_defaultUInt8 = (uint8_t)0;
const Any g_defaultInt32 = (int32_t)0;
const Any g_defaultInt64 = (int64_t)0;
const Any g_defaultUInt64 = (uint64_t)0;
const Any g_defaultDouble = (double)0.0;
const Any g_defaultBool = (bool) false;
const Any g_defaultSrcInputType = SrcInputType::UNKNOWN;
const Any g_defaultAudioSampleFormat = AudioSampleFormat::INVALID_WIDTH;
const Any g_defaultVideoPixelFormat = VideoPixelFormat::UNKNOWN;
const Any g_defaultMediaType = MediaType::UNKNOWN;
const Any g_defaultVideoH264Profile = VideoH264Profile::UNKNOWN;
const Any g_defaultVideoRotation = VideoRotation::VIDEO_ROTATION_0;
const Any g_defaultColorPrimary = ColorPrimary::COLOR_PRIMARY_BT2020;
const Any g_defaultTransferCharacteristic = TransferCharacteristic::TRANSFER_CHARACTERISTIC_BT1361;
const Any g_defaultMatrixCoefficient = MatrixCoefficient::MATRIX_COEFFICIENT_BT2020_CL;
const Any g_defaultHEVCProfile = HEVCProfile::HEVC_PROFILE_UNKNOW;
const Any g_defaultHEVCLevel = HEVCLevel::HEVC_LEVEL_UNKNOW;
const Any g_defaultChromaLocation = ChromaLocation::CHROMA_LOC_BOTTOM;
const Any g_defaultFileType = FileType::UNKNOW;

const Any g_defaultAudioChannelLayout = AudioChannelLayout::UNKNOWN_CHANNEL_LAYOUT;
const Any g_defaultAudioAacProfile = AudioAacProfile::ELD;
const Any g_defaultAudioAacStreamFormat = AudioAacStreamFormat::ADIF;
const Any g_defaultVectorUInt8 = std::vector<uint8_t>();
const Any g_defaultVectorVideoBitStreamFormat = std::vector<VideoBitStreamFormat>();

const std::map<TagType,  const Any&> g_metadataDefaultValueMap = {
        {Tag::SRC_INPUT_TYPE, g_defaultSrcInputType},
        {Tag::AUDIO_SAMPLE_FORMAT, g_defaultAudioSampleFormat},
        {Tag::VIDEO_PIXEL_FORMAT, g_defaultVideoPixelFormat},
        {Tag::MEDIA_TYPE, g_defaultMediaType},
        {Tag::VIDEO_H264_PROFILE, g_defaultVideoH264Profile},
        {Tag::VIDEO_ROTATION, g_defaultVideoRotation},
        {Tag::VIDEO_COLOR_PRIMARIES, g_defaultColorPrimary},
        {Tag::VIDEO_COLOR_TRC, g_defaultTransferCharacteristic},
        {Tag::VIDEO_COLOR_MATRIX_COEFF, g_defaultMatrixCoefficient},
        {Tag::VIDEO_H265_PROFILE, g_defaultHEVCProfile},
        {Tag::VIDEO_H265_LEVEL, g_defaultHEVCLevel},
        {Tag::VIDEO_CHROMA_LOCATION, g_defaultChromaLocation},
        {Tag::MEDIA_FILE_TYPE, g_defaultFileType},
        //Int32
        {Tag::APP_PID, g_defaultInt32},
        {Tag::APP_TOKEN_ID, g_defaultInt32},
        {Tag::REQUIRED_IN_BUFFER_CNT, g_defaultInt32},
        {Tag::REQUIRED_IN_BUFFER_SIZE, g_defaultInt32},
        {Tag::REQUIRED_OUT_BUFFER_CNT, g_defaultInt32},
        {Tag::REQUIRED_OUT_BUFFER_SIZE, g_defaultInt32},
        {Tag::BUFFERING_SIZE, g_defaultInt32},
        {Tag::WATERLINE_HIGH, g_defaultInt32},
        {Tag::WATERLINE_LOW, g_defaultInt32},
        {Tag::AUDIO_CHANNEL_COUNT, g_defaultInt32},
        {Tag::AUDIO_SAMPLE_RATE, g_defaultInt32},
        {Tag::AUDIO_SAMPLE_PER_FRAME, g_defaultInt32},
        {Tag::AUDIO_OUTPUT_CHANNELS, g_defaultInt32},
        {Tag::AUDIO_MPEG_VERSION, g_defaultInt32},
        {Tag::AUDIO_MPEG_LAYER, g_defaultInt32},
        {Tag::AUDIO_AAC_LEVEL, g_defaultInt32},
        {Tag::AUDIO_MAX_INPUT_SIZE, g_defaultInt32},
        {Tag::AUDIO_MAX_OUTPUT_SIZE, g_defaultInt32},
        {Tag::VIDEO_WIDTH, g_defaultInt32},
        {Tag::VIDEO_HEIGHT, g_defaultInt32},
        {Tag::VIDEO_FRAME_RATE, g_defaultInt32},
        {Tag::VIDEO_DELAY, g_defaultInt32},
        {Tag::VIDEO_MAX_SURFACE_NUM, g_defaultInt32},
        {Tag::VIDEO_H264_LEVEL, g_defaultInt32},
        {Tag::MEDIA_TRACK_COUNT, g_defaultInt32},
        {Tag::MEDIA_HAS_VIDEO, g_defaultInt32},
        {Tag::MEDIA_HAS_AUDIO, g_defaultInt32},
        {Tag::AUDIO_AAC_IS_ADTS, g_defaultInt32},
        {Tag::AUDIO_COMPRESSION_LEVEL, g_defaultInt32},
        {Tag::BITS_PER_CODED_SAMPLE, g_defaultInt32},
        {Tag::MEDIA_TRACK_COUNT, g_defaultInt32},
        {Tag::MEDIA_HAS_VIDEO, g_defaultInt32},
        {Tag::MEDIA_HAS_AUDIO, g_defaultInt32},
        //String
        {Tag::MIME_TYPE, g_defaultString},
        {Tag::MEDIA_FILE_URI, g_defaultString},
        {Tag::MEDIA_TITLE, g_defaultString},
        {Tag::MEDIA_ARTIST, g_defaultString},
        {Tag::MEDIA_LYRICIST, g_defaultString},
        {Tag::MEDIA_ALBUM, g_defaultString},
        {Tag::MEDIA_ALBUM_ARTIST, g_defaultString},
        {Tag::MEDIA_DATE, g_defaultString},
        {Tag::MEDIA_COMMENT, g_defaultString},
        {Tag::MEDIA_GENRE, g_defaultString},
        {Tag::MEDIA_COPYRIGHT, g_defaultString},
        {Tag::MEDIA_LANGUAGE, g_defaultString},
        {Tag::MEDIA_DESCRIPTION, g_defaultString},
        {Tag::USER_TIME_SYNC_RESULT, g_defaultString},
        {Tag::USER_AV_SYNC_GROUP_INFO, g_defaultString},
        {Tag::USER_SHARED_MEMORY_FD, g_defaultString},
        {Tag::MEDIA_AUTHOR, g_defaultString},
        {Tag::MEDIA_COMPOSER, g_defaultString},
        {Tag::MEDIA_LYRICS, g_defaultString},
        //Double
        {Tag::VIDEO_CAPTURE_RATE, g_defaultDouble},
        //Bool
        {Tag::VIDEO_COLOR_RANGE, g_defaultBool},
        {Tag::VIDEO_IS_HDR_VIVID, g_defaultBool},
        //UInt64
        {Tag::MEDIA_FILE_SIZE, g_defaultUInt64},
        {Tag::MEDIA_POSITION, g_defaultUInt64},
        //Int64
        {Tag::APP_FULL_TOKEN_ID, g_defaultInt64},
        {Tag::MEDIA_DURATION, g_defaultInt64},
        {Tag::MEDIA_BITRATE, g_defaultInt64},
        {Tag::MEDIA_START_TIME, g_defaultInt64},
        {Tag::USER_FRAME_PTS, g_defaultInt64},
        {Tag::USER_PUSH_DATA_TIME, g_defaultInt64},
        //AudioChannelLayout UINT64_T
        {Tag::AUDIO_CHANNEL_LAYOUT, g_defaultAudioChannelLayout},
        {Tag::AUDIO_OUTPUT_CHANNEL_LAYOUT, g_defaultAudioChannelLayout},
        //AudioAacProfile UInt8
        {Tag::AUDIO_AAC_PROFILE, g_defaultAudioAacProfile},
        //AudioAacStreamFormat UInt8
        {Tag::AUDIO_AAC_STREAM_FORMAT, g_defaultAudioAacStreamFormat},
        //vector<uint8_t>
        {Tag::MEDIA_CODEC_CONFIG, g_defaultVectorUInt8},
        {Tag::MEDIA_COVER, g_defaultVectorUInt8},
        //vector<Plugin::VideoBitStreamFormat>
        {Tag::VIDEO_BIT_STREAM_FORMAT, g_defaultVectorVideoBitStreamFormat}
};


bool SetMetaData(Meta& meta, const TagType& tag, int32_t& value) {
    auto iter = g_metadataGetterSetterMap.find(tag);
    if (iter == g_metadataGetterSetterMap.end()) {
        return false;
    }
    return iter->second.first(meta, tag, value);
}

bool GetMetaData(const Meta& meta, const TagType& tag, int32_t& value) {
    auto iter = g_metadataGetterSetterMap.find(tag);
    if (iter == g_metadataGetterSetterMap.end()) {
        return false;
    }
    return iter->second.second(meta, tag, value);
}

Any GetDefaultAnyValue(const TagType& tag) {
    auto iter = g_metadataDefaultValueMap.find(tag);
    if (iter == g_metadataDefaultValueMap.end()) {
        return g_defaultString; //Default String type
    }
    return iter->second;
}
}
} // namespace OHOS