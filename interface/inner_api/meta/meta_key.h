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

#ifndef MEDIA_FOUNDATION_META_KEY_H
#define MEDIA_FOUNDATION_META_KEY_H

#include "any.h"

namespace OHOS {
namespace Media {
class Tag {
public:
    /* -------------------- regular tag -------------------- */
    static constexpr const char REGULAR_TRACK_ID[] = "regular.trace.id";                           ///< track id
    static constexpr const char REQUIRED_IN_BUFFER_CNT[] = "regular.required.in.buffer.cnt";       ///< required buffer count of plugin; read only tag
    static constexpr const char REQUIRED_IN_BUFFER_SIZE[] = "regular.required.in.buffer.size";     ///< required buffer size of plugin; read only tag
    static constexpr const char REQUIRED_OUT_BUFFER_CNT[] = "regular.required.out.buffer.cnt";     ///< required buffer count of plugin; read only tag
    static constexpr const char REQUIRED_OUT_BUFFER_SIZE[] = "regular.required.out.buffer.size";   ///< required buffer size of plugin; read only tag
    static constexpr const char BUFFER_ALLOCATOR[] = "regular.buffer.allocator";                   ///< Allocator, allocator to alloc buffers
    static constexpr const char BUFFERING_SIZE[] = "regular.buffering.size";                       ///< download buffer size
    static constexpr const char WATERLINE_HIGH[] = "regular.waterline.high";                       ///< high waterline
    static constexpr const char WATERLINE_LOW[] = "regular.waterline.low";                         ///< low waterline
    static constexpr const char SRC_INPUT_TYPE[] = "regular.src.input.type";                       ///< SrcInputType
    static constexpr const char BITS_PER_CODED_SAMPLE[] = "regular.bits.per.coded.sample";         ///< bits per coded sample
    static constexpr const char APP_TOKEN_ID[] = "regular.app.token.id";                           ///< app token id
    static constexpr const char APP_FULL_TOKEN_ID[] = "regular.app.full.token.id";                 ///< app full token id
    static constexpr const char APP_UID[] = "regular.app.uid";                                     ///< app uid
    static constexpr const char APP_PID[] = "regular.app.pid";                                     ///< app pid
    static constexpr const char AUDIO_RENDER_INFO[] = "regular.audio.render.info";                 ///< AudioRenderInfo, audio render info
    static constexpr const char AUDIO_INTERRUPT_MODE[] = "regular.audio.interrupt.mode";           ///< AudioInterruptMode, audio interrupt mode
    static constexpr const char VIDEO_SCALE_TYPE[] = "regular.video.scale.type";                   ///< VideoScaleType, video scale type
    static constexpr const char INPUT_MEMORY_TYPE[] = "regular.input.memory.type";                 ///< MemoryType
    static constexpr const char OUTPUT_MEMORY_TYPE[] = "regular.output.memory.type";               ///< MemoryType

    /* -------------------- media tag -------------------- */
    static constexpr const char MIME_TYPE[] = "mime.type";                           ///< @see MimeType
    static constexpr const char MEDIA_TITLE[] = "media.title";                       ///< title
    static constexpr const char MEDIA_ARTIST[] = "media.artist";                     ///< artist
    static constexpr const char MEDIA_LYRICIST[] = "media.lyricist";                 ///< lyricist
    static constexpr const char MEDIA_ALBUM[] = "media.album";                       ///< album
    static constexpr const char MEDIA_ALBUM_ARTIST[] = "media.album.artist";         ///< album artist
    static constexpr const char MEDIA_DATE[] = "media.date";                         ///< media date, format：YYYY-MM-DD
    static constexpr const char MEDIA_COMMENT[] = "media.comment";                   ///< comment
    static constexpr const char MEDIA_GENRE[] = "media.genre";                       ///< genre
    static constexpr const char MEDIA_COPYRIGHT[] = "media.copyright";               ///< copyright
    static constexpr const char MEDIA_LANGUAGE[] = "media.language";                 ///< language
    static constexpr const char MEDIA_DESCRIPTION[] = "media.description";           ///< description
    static constexpr const char MEDIA_LYRICS[] = "media.lyrics";                     ///< lyrics
    static constexpr const char MEDIA_AUTHOR[] = "media.author";                     ///< authoe
    static constexpr const char MEDIA_COMPOSER[] = "media.composer";                 ///< composer
    static constexpr const char MEDIA_DURATION[] = "media.duration";                 ///< duration based on {@link HST_TIME_BASE}
    static constexpr const char MEDIA_FILE_SIZE[] = "media.file.size";               ///< file size
    static constexpr const char MEDIA_BITRATE[] = "media.bitrate";                   ///< bite rate
    static constexpr const char MEDIA_FILE_URI[] = "media.file.uri";                 ///< file uri
    static constexpr const char MEDIA_CODEC_CONFIG[] = "media.codec.config";         ///< codec config. e.g. AudioSpecificConfig for mp4
    static constexpr const char MEDIA_CODEC_MODE[] = "media.codec.mode";             ///< codec mode.
    static constexpr const char MEDIA_POSITION[] = "media.position";                 ///< The byte position within media stream/file
    static constexpr const char MEDIA_START_TIME[] = "media.start.time";             ///< The start time of one track
    static constexpr const char MEDIA_SEEKABLE[] = "media.seekable";                 ///< enum Seekable: Seekable status of the media
    static constexpr const char MEDIA_PLAYBACK_SPEED[] = "media.playback.speed";     ///< double, playback speed
    static constexpr const char MEDIA_TYPE[] = "media.type";                         ///< enum MediaType: Audio Video Subtitle...
    static constexpr const char MEDIA_TRACK_COUNT[] = "media.track.conut";           ///< track count in file
    static constexpr const char MEDIA_FILE_TYPE[] = "media.file.type";               ///< @see FileType, track type
    static constexpr const char MEDIA_STREAM_TYPE[] = "media.stream.type";           ///< stream type of track data
    static constexpr const char MEDIA_HAS_VIDEO[] = "media.has.video";               ///< has video track in file
    static constexpr const char MEDIA_HAS_AUDIO[] = "media.has.audio";               ///< has audio track in file
    static constexpr const char MEDIA_COVER[] = "media.cover";                       ///< cover in file
    static constexpr const char MEDIA_PROTOCOL_TYPE[] = "media.protocol.type";       ///< Source protocol type

    /* -------------------- audio universal tag -------------------- */
    static constexpr const char AUDIO_CHANNEL_COUNT[] = "audio.channel.count";                  ///< audio channel count
    static constexpr const char AUDIO_CHANNEL_LAYOUT[] = "audio.channel.layout";                ///< @see AudioChannelLayout, stream channel layout
    static constexpr const char AUDIO_SAMPLE_RATE[] = "audio.sample.rate";                      ///< sample rate
    static constexpr const char AUDIO_SAMPLE_FORMAT[] = "audio.sample.format";                  ///< @see AudioSampleFormat
    static constexpr const char AUDIO_SAMPLE_PER_FRAME[] = "audio.sample.per.frame";            ///< sample per frame
    static constexpr const char AUDIO_OUTPUT_CHANNELS[] = "audio.output.channels";              ///< sink output channel num
    static constexpr const char AUDIO_OUTPUT_CHANNEL_LAYOUT[] = "audio.output.channel.layout";  ///< @see AudioChannelLayout, sink output channel layout
    static constexpr const char AUDIO_COMPRESSION_LEVEL[] = "audio.compresion.level";           ///< compression level
    static constexpr const char AUDIO_MAX_INPUT_SIZE[] = "audio.max.input.size";                ///< max input size
    static constexpr const char AUDIO_MAX_OUTPUT_SIZE[] = "audio.max.output.size";              ///< max output size

    /* -------------------- audio specific tag -------------------- */
    static constexpr const char AUDIO_MPEG_VERSION[] = "audio.mpeg.version";  ///< mpeg version
    static constexpr const char AUDIO_MPEG_LAYER[] = "audio.mpeg.layer";      ///< mpeg layer

    static constexpr const char AUDIO_AAC_PROFILE[] = "audio.aac.profile";               ///< @see AudioAacProfile
    static constexpr const char AUDIO_AAC_LEVEL[] = "audio.aac.level";                   ///< acc level
    static constexpr const char AUDIO_AAC_STREAM_FORMAT[] = "audio.aac.stream.format";   ///< @see AudioAacStreamFormat
    static constexpr const char AUDIO_AAC_IS_ADTS[] = "audio.aac.is.adts";               ///< acc format is adts

    /* -------------------- video universal tag -------------------- */
    static constexpr const char VIDEO_WIDTH[] = "width";                                ///< video width
    static constexpr const char VIDEO_HEIGHT[] = "height";                              ///< video height
    static constexpr const char VIDEO_PIXEL_FORMAT[] = "pixel_format";                  ///< @see VideoPixelFormat
    static constexpr const char VIDEO_FRAME_RATE[] = "frame_rate";                      ///< video frame rate
    static constexpr const char VIDEO_SURFACE[] = "video.surface";                      ///< @see class Surface
    static constexpr const char VIDEO_MAX_SURFACE_NUM[] = "video.max.surface_num";      ///< max video surface num
    static constexpr const char VIDEO_CAPTURE_RATE[] = "video.capture.rate";            ///< double, video capture rate
    static constexpr const char VIDEO_BIT_STREAM_FORMAT[] = "video.bit.stream.format";  ///< @see VideoBitStreamFormat
    static constexpr const char VIDEO_ROTATION[] = "video.rotation";                    ///< @see VideoRotation
    static constexpr const char VIDEO_COLOR_PRIMARIES[] = "video.color.primaries";      ///< @see ColorPrimary
    static constexpr const char VIDEO_COLOR_TRC[] = "video.color.transfer.characteristics";    ///< @see TransferCharacteristic
    static constexpr const char VIDEO_COLOR_MATRIX_COEFF[] = "video.color.matrix.coefficient"; ///< @see MatrixCoefficient
    static constexpr const char VIDEO_COLOR_RANGE[] = "video.color.range";              ///< bool, video color range
    static constexpr const char VIDEO_IS_HDR_VIVID[] = "video.is.hdr.vivid";            ///< bool, video is hdr vivid
    static constexpr const char VIDEO_DELAY[] = "video.delay";                          ///< video delay

    /* -------------------- video specific tag -------------------- */
    static constexpr const char VIDEO_H264_PROFILE[] = "video.h264.profile";                          ///< @see VideoH264Profile
    static constexpr const char VIDEO_H264_LEVEL[] = "video.h264.level";                              ///< h264 level
    static constexpr const char VIDEO_H265_PROFILE[] = "video.h265.profile";                          ///< @see HEVCProfile
    static constexpr const char VIDEO_H265_LEVEL[] = "video.h265.level";                              ///< @see HEVCLevel
    static constexpr const char VIDEO_CHROMA_LOCATION[] = "video.chroma.location";                    ///< @see ChromaLocation

    /* -------------------- user specific tag -------------------- */
    static constexpr const char USER_FRAME_PTS[] = "user.frame.pts";                     ///< The user frame pts
    static constexpr const char USER_TIME_SYNC_RESULT[] = "user.time.sync.result";       ///< std::string : The time sync result
    static constexpr const char USER_AV_SYNC_GROUP_INFO[] = "user.av.sync.group.info";   ///< std::string : The av sync group info
    static constexpr const char USER_SHARED_MEMORY_FD[] = "user.shared.memory.fd";       ///< std::string : The shared memory fd
    static constexpr const char USER_PUSH_DATA_TIME[] = "user.push.data.time";           ///< The user push data time
};

using TagTypeCharSeq = const char*;
using TagType = std::string;
} // namespace Media
} // namespace OHOS
#endif // MEDIA_FOUNDATION_META_KEY_H
