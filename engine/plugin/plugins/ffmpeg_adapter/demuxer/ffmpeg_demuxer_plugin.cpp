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

#define LOG_TAG "FFmpegDemuxerPlugin"

#include "ffmpeg_demuxer_plugin.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <new>
#include "plugin/core/plugin_manager.h"
#include "ffmpeg_track_meta.h"
#include "foundation/log.h"
#include "utils/memory_helper.h"
#include "osal/thread/scoped_lock.h"
#include "plugin/common/plugin_buffer.h"
#include "plugins/ffmpeg_adapter/utils/ffmpeg_utils.h"

#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(58, 78, 0) and LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(58, 64, 100)
#include "libavformat/internal.h"
#endif

namespace OHOS {
namespace Media {
namespace Plugin {
namespace {
const std::map<std::string, MetaID> TAG_MAP = {
    {"title", MetaID::MEDIA_TITLE},
    {"artist", MetaID::MEDIA_ARTIST},
    {"lyricist", MetaID::MEDIA_LYRICIST},
    {"album", MetaID::MEDIA_ALBUM},
    {"album-artist", MetaID::MEDIA_ALBUM_ARTIST},
    {"date", MetaID::MEDIA_DATE},
    {"comment", MetaID::MEDIA_COMMENT},
    {"genre", MetaID::MEDIA_GENRE},
    {"copyright", MetaID::MEDIA_COPYRIGHT},
    {"language", MetaID::MEDIA_LANGUAGE},
    {"description", MetaID::MEDIA_DESCRIPTION},
    {"lyrics", MetaID::MEDIA_LYRICS},
};

std::map<std::string, std::shared_ptr<AVInputFormat>> g_pluginInputFormat;

int Sniff(const std::string& name, std::shared_ptr<DataSource> dataSource);

Status RegisterPlugins(const std::shared_ptr<Register>& reg);

int ConvertSeekModeToFFmpeg(SeekMode mode);
} // namespace

void* FFmpegDemuxerPlugin::DemuxerPluginAllocator::Alloc(size_t size)
{
    if (size == 0) {
        return nullptr;
    }
    return static_cast<void*>(new (std::nothrow) uint8_t[size]);
}

void FFmpegDemuxerPlugin::DemuxerPluginAllocator::Free(void* ptr) // NOLINT: void*
{
    if (ptr) {
        auto data = static_cast<uint8_t*>(ptr);
        delete[] data;
    }
}

FFmpegDemuxerPlugin::FFmpegDemuxerPlugin(std::string name)
    : DemuxerPlugin(std::move(name)),
      ioContext_(),
      callback_(nullptr),
      pluginImpl_(nullptr),
      formatContext_(nullptr),
      allocator_(std::make_shared<DemuxerPluginAllocator>()),
      mediaInfo_(nullptr),
      selectedTrackIds_()
{
    MEDIA_LOG_I("ctor called, plugin name: %s", pluginName_.c_str());
}

FFmpegDemuxerPlugin::~FFmpegDemuxerPlugin()
{
    MEDIA_LOG_I("dtor called.");
    pluginImpl_ = nullptr;
}

Status FFmpegDemuxerPlugin::Init()
{
    MEDIA_LOG_I("Init called.");
    Reset();
    pluginImpl_ = g_pluginInputFormat[pluginName_];

    return pluginImpl_ ? Status::OK : Status::ERROR_UNSUPPORTED_FORMAT;
}

Status FFmpegDemuxerPlugin::Deinit()
{
    return Status::OK;
}

Status FFmpegDemuxerPlugin::Prepare()
{
    InitAVFormatContext();
    if (formatContext_ == nullptr) {
        MEDIA_LOG_E("prepare failed due to formatContext init error");
        return Status::ERROR_UNKNOWN;
    }
    return Status::OK;
}

Status FFmpegDemuxerPlugin::Reset()
{
    mediaInfo_.reset();
    ioContext_.offset = 0;
    ioContext_.eos = false;
    selectedTrackIds_.clear();
    return Status::OK;
}

Status FFmpegDemuxerPlugin::Start()
{
    return Status::OK;
}

Status FFmpegDemuxerPlugin::Stop()
{
    return Status::OK;
}

/**
 * IsParameterSupported no need supported by demuxer
 * @return return false always.
 */
bool FFmpegDemuxerPlugin::IsParameterSupported(Tag tag)
{
    (void)tag;
    return false;
}

/**
 * GetParameter no need supported by demuxer
 * @return return ERROR_UNIMPLEMENTED always.
 */
Status FFmpegDemuxerPlugin::GetParameter(Tag tag, ValueType& value)
{
    (void)tag;
    (void)value;
    return Status::ERROR_UNIMPLEMENTED;
}

/**
 * SetParameter no need supported by demuxer
 * @return return ERROR_UNIMPLEMENTED always.
 */
Status FFmpegDemuxerPlugin::SetParameter(Tag tag, const ValueType& value)
{
    (void)tag;
    (void)value;
    return Status::ERROR_UNIMPLEMENTED;
}

std::shared_ptr<Allocator> FFmpegDemuxerPlugin::GetAllocator()
{
    return allocator_;
}

Status FFmpegDemuxerPlugin::SetCallback(const std::shared_ptr<Callback>& cb)
{
    callback_ = cb;
    return Status::OK;
}

Status FFmpegDemuxerPlugin::SetDataSource(const std::shared_ptr<DataSource>& source)
{
    ioContext_.dataSource = source;
    return Status::OK;
}

Status FFmpegDemuxerPlugin::GetMediaInfo(MediaInfo& mediaInfo)
{
    if (!mediaInfo_ && !ParseMediaData()) {
        return Status::ERROR_INVALID_PARAMETER;
    }
    mediaInfo = *mediaInfo_;
    return Status::OK;
}

size_t FFmpegDemuxerPlugin::GetTrackCount()
{
    size_t trackCnt = 0;
    if (mediaInfo_) {
        trackCnt = mediaInfo_->tracks.size();
    }
    return trackCnt;
}

Status FFmpegDemuxerPlugin::SelectTrack(int32_t trackId)
{
    if (!mediaInfo_) {
        MEDIA_LOG_E("SelectTrack called before GetMediaInfo()...");
        return Status::ERROR_INVALID_DATA;
    }
    if (trackId < 0 || trackId >= static_cast<int32_t>(mediaInfo_->tracks.size())) {
        MEDIA_LOG_E("SelectTrack called with invalid trackId: %d, number of tracks: %d", trackId,
                    static_cast<int>(mediaInfo_->tracks.size()));
        return Status::ERROR_INVALID_DATA;
    }
    OSAL::ScopedLock lock(mutex_);
    auto it = std::find_if(selectedTrackIds_.begin(), selectedTrackIds_.end(),
                           [trackId](int32_t streamId) { return trackId == streamId; });
    if (it == selectedTrackIds_.end()) {
        selectedTrackIds_.push_back(trackId);
    }
    return Status::OK;
}

Status FFmpegDemuxerPlugin::UnselectTrack(int32_t trackId)
{
    OSAL::ScopedLock lock(mutex_);
    auto it = std::find_if(selectedTrackIds_.begin(), selectedTrackIds_.end(),
                           [trackId](int32_t streamId) { return trackId == streamId; });
    if (it != selectedTrackIds_.end()) {
        selectedTrackIds_.erase(it);
    }
    return Status::OK;
}

Status FFmpegDemuxerPlugin::GetSelectedTracks(std::vector<int32_t>& trackIds)
{
    OSAL::ScopedLock lock(mutex_);
    trackIds = selectedTrackIds_;
    return Status::OK;
}

bool FFmpegDemuxerPlugin::ConvertAVPacketToFrameInfo(const AVStream& avStream, const AVPacket& pkt, Buffer& frameInfo)
{
    frameInfo.streamID = static_cast<uint32_t>(pkt.stream_index);
    int64_t pts = (pkt.pts > 0) ? pkt.pts : 0;
    frameInfo.pts = ConvertTimeFromFFmpeg(pts, avStream.time_base);
    frameInfo.dts = static_cast<uint32_t>(pkt.dts);
    frameInfo.duration = ConvertTimeFromFFmpeg(pkt.duration, avStream.time_base);
    frameInfo.GetBufferMeta()->SetMeta(Tag::MEDIA_POSITION, static_cast<uint32_t>(pkt.pos));

    int frameSize = 0;
    if (avStream.codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
        frameSize = pkt.size;
    } else if (avStream.codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
        if (avStream.codecpar->codec_id == AV_CODEC_ID_RAWVIDEO) {
            MEDIA_LOG_W("unsupport raw video");
            return false;
        }
        frameSize = pkt.size;
    } else {
        MEDIA_LOG_W("unsupported codec type: %d", static_cast<int32_t>(avStream.codecpar->codec_type));
        return false;
    }
    auto data = frameInfo.AllocMemory(allocator_, frameSize);
    if (data) {
        size_t writeSize = data->Write(pkt.data, frameSize);
        ASSERT_CONDITION(writeSize == frameSize, "Copy data failed.");
    }
    return data != nullptr;
}

Status FFmpegDemuxerPlugin::ReadFrame(Buffer& info, int32_t timeOutMs)
{
    (void)timeOutMs;
    AVPacket pkt;
    int res = 0;
    do {
        res = av_read_frame(formatContext_.get(), &pkt);
    } while (res >= 0 && !selectedTrackIds_.empty() && !IsSelectedTrack(pkt.stream_index));
    Status result = Status::ERROR_UNKNOWN;
    if (res == 0 && ConvertAVPacketToFrameInfo(*(formatContext_->streams[pkt.stream_index]), pkt, info)) {
        result = Status::OK;
    } else {
        MEDIA_LOG_W("ReadFrame failed, rtv = %s", AVStrError(res).c_str());
    }
    av_packet_unref(&pkt);
    return (res != AVERROR_EOF) ? result : Status::END_OF_STREAM;
}

/**
 * SeekTo seek operation
 * @param trackId  -1 for unspecified, >= 0 for specific trackid
 * @param timeStampUs
 * @param mode
 * @return operation result.
 */
Status FFmpegDemuxerPlugin::SeekTo(int32_t trackId, int64_t timeStampUs, SeekMode mode)
{
    if (trackId == -1) {
        trackId = av_find_default_stream_index(formatContext_.get());
    }
    if (trackId < 0 || trackId >= static_cast<int32_t>(formatContext_->nb_streams)) {
        MEDIA_LOG_E("SeekTo called with invalid trackid = %d, nb_streams = %d.", trackId, formatContext_->nb_streams);
        return Status::ERROR_INVALID_PARAMETER;
    }

    auto avStream = formatContext_->streams[trackId];
    int64_t ffTime = ConvertTimeToFFmpeg(timeStampUs, avStream->time_base);
    if (avStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
        int keyFrameIdx = av_index_search_timestamp(avStream, ffTime, ConvertSeekModeToFFmpeg(mode));
        MEDIA_LOG_I("SeekTo %" PRId64 ", ffTime: %" PRId64 ", key frame index: %d", timeStampUs, ffTime, keyFrameIdx);
        if (keyFrameIdx >= 0) {
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(58, 78, 0)
            ffTime = avformat_index_get_entry(avStream, keyFrameIdx)->timestamp;
#elif LIBAVFORMAT_VERSION_INT > AV_VERSION_INT(58, 64, 100)
            ffTime = avStream->internal->index_entries[keyFrameIdx].timestamp;
#else
            ffTime = avStream->index_entries[keyFrameIdx].timestamp;
#endif
        }
    }
    auto newTime = ConvertTimeFromFFmpeg(ffTime, avStream->time_base);
    MEDIA_LOG_W("SeekTo %" PRIu64 " / %" PRId64 ", ffTime: %" PRId64, newTime, timeStampUs, ffTime);
    auto rtv = av_seek_frame(formatContext_.get(), trackId, ffTime, ConvertSeekModeToFFmpeg(mode));
    if (rtv < 0) {
        MEDIA_LOG_E("seek failed, return value: %d", rtv);
    }
    return (rtv >= 0) ? Status::OK : Status::ERROR_UNKNOWN;
}

void FFmpegDemuxerPlugin::InitAVFormatContext()
{
    AVFormatContext* formatContext = avformat_alloc_context();
    if (formatContext == nullptr) {
        return;
    }
    formatContext->pb = AllocAVIOContext(AVIO_FLAG_READ);
    formatContext->flags = static_cast<uint32_t>(formatContext->flags) | static_cast<uint32_t>(AVFMT_FLAG_CUSTOM_IO);
    formatContext_ = std::shared_ptr<AVFormatContext>(formatContext, [](AVFormatContext* ptr) {
        if (ptr) {
            auto ctx = ptr->pb;
            if (ctx) {
                av_freep(&ctx->buffer);
                av_free(ctx);
            }
            avformat_close_input(&ptr);
        }
    });
}

std::shared_ptr<AVCodecContext> FFmpegDemuxerPlugin::InitCodecContext(const AVStream& avStream)
{
    auto codecContext = std::shared_ptr<AVCodecContext>(avcodec_alloc_context3(nullptr), [](AVCodecContext* p) {
        if (p) {
            avcodec_free_context(&p);
        }
    });
    if (codecContext == nullptr) {
        MEDIA_LOG_E("cannot create ffmpeg codecContext");
        return nullptr;
    }
    int ret = avcodec_parameters_to_context(codecContext.get(), avStream.codecpar);
    if (ret < 0) {
        MEDIA_LOG_E("avcodec_parameters_to_context failed with return = %s", AVStrError(ret).c_str());
        return nullptr;
    }
    codecContext->workaround_bugs = static_cast<uint32_t>(codecContext->workaround_bugs) | FF_BUG_AUTODETECT;
    codecContext->err_recognition = 1;
    return codecContext;
}

AVIOContext* FFmpegDemuxerPlugin::AllocAVIOContext(int flags)
{
    constexpr int bufferSize = 4096;
    auto buffer = static_cast<unsigned char*>(av_malloc(bufferSize));
    if (buffer == nullptr) {
        MEDIA_LOG_E("AllocAVIOContext failed to av_malloc...");
        return nullptr;
    }
    AVIOContext* avioContext = avio_alloc_context(buffer, bufferSize, flags, static_cast<void*>(&ioContext_),
                                                  AVReadPacket, AVWritePacket, AVSeek);
    if (avioContext == nullptr) {
        MEDIA_LOG_E("AllocAVIOContext failed to avio_alloc_context...");
        av_free(buffer);
        return nullptr;
    }
    avioContext->seekable = AVIO_SEEKABLE_NORMAL;
    if (!(static_cast<uint32_t>(flags) & static_cast<uint32_t>(AVIO_FLAG_WRITE))) {
        avioContext->buf_ptr = avioContext->buf_end;
        avioContext->write_flag = 0;
    }
    return avioContext;
}

bool FFmpegDemuxerPlugin::IsSelectedTrack(int32_t trackId)
{
    bool rtv = false;
    for (const auto& id : selectedTrackIds_) {
        if (id == trackId) {
            rtv = true;
            break;
        }
    }
    return rtv;
}

void FFmpegDemuxerPlugin::SaveFileInfoToMetaInfo(TagMap& meta)
{
    meta.clear();
    AVDictionaryEntry* tag = nullptr;
    while ((tag = av_dict_get(formatContext_->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
        auto it = TAG_MAP.find(tag->key);
        if (it == TAG_MAP.end()) {
            continue;
        }
        if (it->second != Media::Plugin::MetaID::MEDIA_DATE) {
            meta.insert({Tag(it->second), std::string(tag->value)});
        } else {
            uint32_t year = 0;
            uint32_t month = 0;
            uint32_t day = 0;
            if (sscanf_s(tag->value, "%04u-%02u-%02u", &year, &month, &day) == 3) { // 3
                meta.insert({Tag::MEDIA_DATE, RemoveDelimiter(tag->value, '-')});
            }
        }
    }
    int64_t msec = formatContext_->duration / 1000; // 1000
    meta.insert({Tag::MEDIA_DURATION, static_cast<uint64_t>(msec)});
}

bool FFmpegDemuxerPlugin::ParseMediaData()
{
    auto formatContext = formatContext_.get();
    int ret = avformat_open_input(&formatContext, nullptr, pluginImpl_.get(), nullptr);
    if (ret != 0) {
        MEDIA_LOG_E("avformat_open_input using plugin %s failed with return = %s", pluginImpl_->name,
                    AVStrError(ret).c_str());
        return false;
    }
    // retrieve stream information
    avformat_find_stream_info(formatContext, nullptr);
    av_dump_format(formatContext, 0, nullptr, false);

    MemoryHelper::make_unique<MediaInfo>().swap(mediaInfo_);
    size_t streamCnt = formatContext_->nb_streams;
    mediaInfo_->general.clear();
    mediaInfo_->tracks.resize(streamCnt);
    for (size_t i = 0; i < streamCnt; ++i) {
        auto& avStream = *formatContext_->streams[i];
        auto codecContext = InitCodecContext(avStream);
        if (codecContext == nullptr) {
            continue;
        }
        ConvertAVStreamToMetaInfo(avStream, codecContext, mediaInfo_->tracks[i]);
    }
    SaveFileInfoToMetaInfo(mediaInfo_->general);
    return true;
}

// ffmpeg provide buf, we write data
int FFmpegDemuxerPlugin::AVReadPacket(void* opaque, uint8_t* buf, int bufSize)
{
    int rtv = -1;
    auto ioContext = static_cast<IOContext*>(opaque);
    if (ioContext && ioContext->dataSource) {
        auto buffer = std::make_shared<Buffer>();
        auto bufData = buffer->WrapMemory(buf, bufSize, bufSize);
        auto result = ioContext->dataSource->ReadAt(ioContext->offset, buffer, static_cast<size_t>(bufSize));
        MEDIA_LOG_D("AVReadPacket read data size = %d", static_cast<int>(bufData->GetSize()));
        if (result == Status::OK) {
            ioContext->offset += buffer->GetMemory()->GetSize();
            rtv = buffer->GetMemory()->GetSize();
        } else if (result == Status::END_OF_STREAM) {
            ioContext->eos = true;
            rtv = AVERROR_EOF;
        } else {
            MEDIA_LOG_E("AVReadPacket failed with rtv = %d", static_cast<int>(result));
        }
    }
    return rtv;
}

/**
 * write packet unimplemented.
 * @return 0
 */
int FFmpegDemuxerPlugin::AVWritePacket(void* opaque, uint8_t* buf, int bufSize) // NOLINT: intentionally using void*
{
    (void)opaque;
    (void)buf;
    (void)bufSize;
    return 0;
}

int64_t FFmpegDemuxerPlugin::AVSeek(void* opaque, int64_t offset, int whence)
{
    auto ioContext = static_cast<IOContext*>(opaque);
    uint64_t newPos = 0;
    switch (whence) {
        case SEEK_SET:
            newPos = static_cast<uint64_t>(offset);
            ioContext->offset = newPos;
            MEDIA_LOG_I("AVSeek whence: %d, pos = %" PRId64 ", newPos = %" PRIu64, whence, offset, newPos);
            break;
        case SEEK_CUR:
            newPos = ioContext->offset + offset;
            MEDIA_LOG_I("AVSeek whence: %d, pos = %" PRId64 ", newPos = %" PRIu64, whence, offset, newPos);
            break;
        case SEEK_END:
        case AVSEEK_SIZE: {
            size_t mediaDataSize = 0;
            if (ioContext->dataSource->GetSize(mediaDataSize) == Status::OK) {
                newPos = mediaDataSize + offset;
                MEDIA_LOG_I("AVSeek seek end whence: %d, pos = %" PRId64, whence, offset);
            }
            break;
        }
        default:
            MEDIA_LOG_E("AVSeek unexpected whence: %d", whence);
            break;
    }
    if (whence != AVSEEK_SIZE) {
        ioContext->offset = newPos;
    }
    MEDIA_LOG_I("current offset: %" PRId64 ", new pos: %" PRIu64, ioContext->offset, newPos);
    return newPos;
}

namespace {
int ConvertSeekModeToFFmpeg(SeekMode mode)
{
    int seekFlag = AVSEEK_FLAG_BACKWARD;
    switch (mode) {
        case SeekMode::BACKWARD:
            seekFlag = AVSEEK_FLAG_BACKWARD;
            break;
        case SeekMode::FORWARD:
            seekFlag = 0;
            break;
        case SeekMode::FRAME:
            seekFlag = AVSEEK_FLAG_FRAME;
            break;
        case SeekMode::ANY:
            seekFlag = AVSEEK_FLAG_ANY;
            break;
        default:
            MEDIA_LOG_W("unsupported seekmode: %d, using backward mode instead.", static_cast<int>(mode));
            break;
    }
    return seekFlag;
}

int Sniff(const std::string& name, std::shared_ptr<DataSource> dataSource)
{
    auto pluginInfo = PluginManager::Instance().GetPluginInfo(PluginType::DEMUXER, name);
    if (!pluginInfo || !dataSource) {
        MEDIA_LOG_E("Sniff failed due to plugin not found or dataSource invalid for %s.", name.c_str());
        return 0;
    }
    auto plugin = g_pluginInputFormat[pluginInfo->name];
    if (!plugin || !plugin->read_probe) {
        MEDIA_LOG_E("Sniff failed due to invalid plugin for %s.", name.c_str());
        return 0;
    }
    size_t bufferSize = 4096;
    size_t fileSize = 0;
    if (dataSource->GetSize(fileSize) == Status::OK) {
        bufferSize = (bufferSize < fileSize) ? bufferSize : fileSize;
    }
    std::vector<uint8_t> buff(bufferSize);
    auto bufferInfo = std::make_shared<Buffer>();
    auto bufData = bufferInfo->WrapMemory(buff.data(), bufferSize, bufferSize);
    int confidence = 0;
    if (dataSource->ReadAt(0, bufferInfo, bufferSize) == Status::OK) {
        AVProbeData probeData{"", buff.data(), static_cast<int>(bufferInfo->GetMemory()->GetSize()), ""};
        confidence = plugin->read_probe(&probeData);
    }
    MEDIA_LOG_D("Sniff: plugin name = %s, probability = %d / 100 ...", plugin->name, confidence);
    return confidence;
}

bool IsInputFormatSupported(const char* name)
{
    if (!strcmp(name, "audio_device") || !strncmp(name, "image", 5) ||                 // 5
        !strcmp(name, "mjpeg") || !strcmp(name, "redir") || !strncmp(name, "u8", 2) || // 2
        !strncmp(name, "u16", 3) || !strncmp(name, "u24", 3) ||                        // 3
        !strncmp(name, "u32", 3) ||                                                    // 3
        !strncmp(name, "s8", 2) || !strncmp(name, "s16", 3) ||                         // 2 3
        !strncmp(name, "s24", 3) ||                                                    // 3
        !strncmp(name, "s32", 3) || !strncmp(name, "f32", 3) ||                        // 3
        !strncmp(name, "f64", 3) ||                                                    // 3
        !strcmp(name, "mulaw") || !strcmp(name, "alaw")) {
        return false;
    }

    /* no network demuxers */
    if (!strcmp(name, "sdp") || !strcmp(name, "rtsp") || !strcmp(name, "applehttp")) {
        return false;
    }
    return true;
}

Status RegisterPlugins(const std::shared_ptr<Register>& reg)
{
    MEDIA_LOG_D("RegisterPlugins called.");
    if (!reg) {
        MEDIA_LOG_E("RegisterPlugins failed due to null pointer for reg.");
        return Status::ERROR_INVALID_PARAMETER;
    }
    const AVInputFormat* plugin = nullptr;
    void* i = nullptr;
    while ((plugin = av_demuxer_iterate(&i))) {
        MEDIA_LOG_D("Attempting to handle libav demuxer plugin %s [%s]", plugin->name, plugin->long_name);
        /* no emulators */
        if (plugin->long_name != nullptr) {
            if (!strncmp(plugin->long_name, "pcm ", 4)) { // 4
                continue;
            }
        }

        if (!IsInputFormatSupported(plugin->name)) {
            continue;
        }

        std::string pluginName = "avdemux_" + std::string(plugin->name);
        ReplaceDelimiter(".,|-<> ", '_', pluginName);

        DemuxerPluginDef regInfo;
        regInfo.name = pluginName;
        regInfo.description = "adapter for ffmpeg demuxer plugin";
        regInfo.rank = 100; // 100
        SplitString(plugin->extensions, ',').swap(regInfo.extensions);
        g_pluginInputFormat[pluginName] =
            std::shared_ptr<AVInputFormat>(const_cast<AVInputFormat*>(plugin), [](void*) {});
        regInfo.creator = [](const std::string& name) -> std::shared_ptr<DemuxerPlugin> {
            return std::make_shared<FFmpegDemuxerPlugin>(name);
        };
        regInfo.sniffer = Sniff;
        auto rtv = reg->AddPlugin(regInfo);
        if (rtv != Status::OK) {
            MEDIA_LOG_E("RegisterPlugins AddPlugin failed with return %d", static_cast<int>(rtv));
        }
    }
    return Status::OK;
}
} // namespace

PLUGIN_DEFINITION(FFmpegDemuxer, LicenseType::LGPL, RegisterPlugins, [] { g_pluginInputFormat.clear(); });
} // namespace Plugin
} // namespace Media
} // namespace OHOS
