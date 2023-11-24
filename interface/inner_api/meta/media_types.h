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

#ifndef MEDIA_FOUNDATION_MEDIA_TYPES_H
#define MEDIA_FOUNDATION_MEDIA_TYPES_H

#include <cstdint>  // NOLINT: used it
#include <vector>

namespace OHOS {
namespace Media {
namespace Plugin {
/**
 * @enum Media Track Type
 *
 * @since 1.0
 * @version 1.0
 */
enum class MediaType : int32_t {
    UNKNOWN = 0,    ///< Usually treated as DATA
    AUDIO,
    VIDEO,
    SUBTITLE,
    ATTACHMENT,     ///< Opaque data information usually sparse
    DATA            ///< Opaque data information usually continuous
};

/**
 * @enum Seekable Status.
 *
 * @since 1.0
 * @version 1.0
 */
enum class Seekable : int32_t {
    INVALID = -1,
    UNSEEKABLE = 0,
    SEEKABLE = 1
};

/**
 * @enum Codec Mode.
 *
 * @since 1.0
 * @version 1.0
 */
enum struct CodecMode {
    HARDWARE, ///<  HARDWARE CODEC
    SOFTWARE, ///<  SOFTWARE CODEC
};

/**
 * @enum Mux file output format.
 *
 * @since 1.0
 * @version 1.0
 */
enum class OutputFormat : uint32_t {
    DEFAULT = 0,
    MPEG_4 = 2,
    M4A = 6,
};

struct Location {
    float latitude = 0;
    float longitude = 0;
};

/**
 * @enum Enumerates types of Seek Mode.
 *
 * @brief Seek modes, Options that SeekTo() behaviour.
 *
 * @since 1.0
 * @version 1.0
 */
enum struct SeekMode : uint32_t {
    SEEK_NEXT_SYNC = 0,     ///> sync to keyframes after the time point.
    SEEK_PREVIOUS_SYNC,     ///> sync to keyframes before the time point.
    SEEK_CLOSEST_SYNC,      ///> sync to closest keyframes.
    SEEK_CLOSEST,           ///> seek to frames closest the time point.
};

/**
 * @enum Buffer flags.
 *
 * @since 1.0
 * @version 1.0
 */
enum class AVBufferFlag : uint32_t {
    NONE = 0,
    /* This signals the end of stream */
    EOS = 1 << 0,
    /* This indicates that the buffer contains the data for a sync frame */
    SYNC_FRAME = 1 << 1,
    /* This indicates that the buffer only contains part of a frame */
    PARTIAL_FRAME = 1 << 2,
    /* This indicated that the buffer contains codec specific data */
    CODEC_DATA = 1 << 3,
};

/**
 * @enum Demux file type.
 *
 * @since 1.0
 * @version 1.0
 */
enum class FileType : int32_t {
    UNKNOW = 0,
    MP4 = 101,
    MPEGTS = 102,
    MKV = 103,
    AMR = 201,
    AAC = 202,
    MP3 = 203,
    FLAC = 204,
    OGG = 205,
    M4A = 206,
    WAV = 207,
};

/**
 * @enum Media protocol type.
 *
 * @since 1.0
 * @version 1.0
 */
enum struct ProtocolType : uint32_t {
    UNKNOWN, ///< Unknown protocol
    FILE,    ///< File protocol, uri prefix: "file://"
    FD,      ///< File descriptor protocol, uri prefix: "fd://"
    STREAM,  ///< Stream protocol, uri prefix: "stream://"
    HTTP,    ///< Http protocol, uri prefix: "http://"
    HTTPS,   ///< Https protocol, uri prefix: "https://"
    HLS,     ///< Http live streaming protocol, uri prefix: "https://" or "https://" or "file://", suffix: ".m3u8"
    DASH,    ///< Dynamic adaptive streaming over Http protocol, uri prefix: "https://" or "https://", suffix: ".mpd"
    RTSP,    ///< Real time streaming protocol, uri prefix: "rtsp://"
    RTP,     ///< Real-time transport protocol, uri prefix: "rtp://"
    RTMP,    ///< RTMP protocol, uri prefix: "rtmp://"
    FTP,     ///< FTP protocol, uri prefix: "ftp://"
    UDP,     ///< User datagram protocol, uri prefix: "udp://"
};

/**
 * @enum Plugin Type.
 *
 * @since 1.0
 * @version 1.0
 */
enum struct PluginType : int32_t {
    INVALID_TYPE = -1, ///< Invalid plugin
    SOURCE = 1,        ///< reference SourcePlugin
    DEMUXER,           ///< reference DemuxerPlugin
    AUDIO_DECODER,     ///< reference CodecPlugin
    AUDIO_ENCODER,     ///< reference CodecPlugin
    VIDEO_DECODER,     ///< reference CodecPlugin
    VIDEO_ENCODER,     ///< reference CodecPlugin
    AUDIO_SINK,        ///< reference AudioSinkPlugin
    VIDEO_SINK,        ///< reference VideoSinkPlugin
    MUXER,             ///< reference MuxerPlugin
    OUTPUT_SINK,       ///< reference OutputSinkPlugin
    GENERIC_PLUGIN,    ///< generic plugin can be used to represent any user extended plugin
};

/**
 * @enum Plugin running state.
 *
 * @since 1.0
 * @version 1.0
 */
enum struct State : int32_t {
    CREATED = 0,     ///< Indicates the status of the plugin when it is constructed.
    ///< The plug-in will not be restored in the entire life cycle.
    INITIALIZED = 1, ///< Plugin global resource initialization completion status.
    PREPARED = 2,    ///< Status of parameters required for plugin running.
    RUNNING = 3,     ///< The system enters the running state after call start().
    PAUSED = 4,      ///< Plugin temporarily stops processing data. This state is optional.
    DESTROYED = -1,  ///< Plugin destruction state. In this state, all resources are released.
    INVALID = -2,    ///< An error occurs in any state and the plugin enters the invalid state.
};

/**
 * The tag content is stored in key-value format.
 */
using CodecConfig = std::vector<uint8_t>;
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif // MEDIA_FOUNDATION_MEDIA_TYPES_H