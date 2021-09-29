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

#ifndef HISTREAMER_FFMPEG_DEMUXER_PLUGIN_H
#define HISTREAMER_FFMPEG_DEMUXER_PLUGIN_H

#include <memory>
#include <string>
#include <vector>

#include "foundation/type_define.h"
#include "core/plugin_register.h"
#include "interface/demuxer_plugin.h"
#include "thread/mutex.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "libavformat/avformat.h"
#ifdef __cplusplus
};
#endif

namespace OHOS {
namespace Media {
namespace Plugin {
class FFmpegDemuxerPlugin : public DemuxerPlugin {
public:
    explicit FFmpegDemuxerPlugin(std::string name);
    ~FFmpegDemuxerPlugin() override;

    Status Init() override;
    Status Deinit() override;
    Status Prepare() override;
    Status Reset() override;
    Status Start() override;
    Status Stop() override;
    bool IsParameterSupported(Tag tag) override;
    Status GetParameter(Tag tag, ValueType& value) override;
    Status SetParameter(Tag tag, const ValueType& value) override;
    std::shared_ptr<Allocator> GetAllocator() override;
    Status SetCallback(const std::shared_ptr<Callback>& cb) override;

    Status SetDataSource(const std::shared_ptr<DataSource>& source) override;
    Status GetMediaInfo(MediaInfo& mediaInfo) override;
    size_t GetTrackCount() override;
    Status SelectTrack(int32_t trackId) override;
    Status UnselectTrack(int32_t trackId) override;
    Status GetSelectedTracks(std::vector<int32_t>& trackIds) override;
    Status ReadFrame(Buffer& info, int32_t timeOutMs) override;
    Status SeekTo(int32_t trackId, int64_t timeStampUs, SeekMode mode) override;

private:
    class DemuxerPluginAllocator : public Allocator {
    public:
        DemuxerPluginAllocator() = default;
        ~DemuxerPluginAllocator() override = default;
        void* Alloc(size_t size) override;
        void Free(void* ptr) override;
    };

    struct IOContext {
        std::shared_ptr<DataSource> dataSource {nullptr};
        int64_t offset {0};
        bool eos {false};
    };

    void InitAVFormatContext();

    static std::shared_ptr<AVCodecContext> InitCodecContext(const AVStream& avStream);

    AVIOContext* AllocAVIOContext(int flags);

    bool IsSelectedTrack(int32_t trackId);

    void SaveFileInfoToMetaInfo(TagMap &meta);

    bool ParseMediaData();

    bool ConvertAVPacketToFrameInfo(const AVStream& avStream, const AVPacket& pkt, Buffer& frameInfo);

    static int AVReadPacket(void* opaque, uint8_t* buf, int bufSize);

    static int AVWritePacket(void* opaque, uint8_t* buf, int bufSize);

    static int64_t AVSeek(void* opaque, int64_t offset, int whence);

    std::string name_;
    IOContext ioContext_;
    std::shared_ptr<Callback> callback_ {};
    std::shared_ptr<AVInputFormat> pluginImpl_;
    std::shared_ptr<AVFormatContext> formatContext_;
    std::shared_ptr<Allocator> allocator_;
    std::unique_ptr<MediaInfo> mediaInfo_;
    std::vector<int32_t> selectedTrackIds_;
    OSAL::Mutex mutex_ {};
};
} // namespace Plugin
} // namespace Media
} // namespace OHOS

#endif // HISTREAMER_FFMPEG_DEMUXER_PLUGIN_H
