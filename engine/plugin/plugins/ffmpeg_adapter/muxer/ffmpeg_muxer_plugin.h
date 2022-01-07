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

#ifndef HISTREAMER_FFMPEG_MUXER_PLUGIN_H
#define HISTREAMER_FFMPEG_MUXER_PLUGIN_H

#include "plugin/interface/muxer_plugin.h"
#include "foundation/osal/thread/mutex.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "libavformat/avformat.h"
#ifdef __cplusplus
}
#endif

namespace OHOS {
namespace Media {
namespace FFMux {
class FFmpegMuxerPlugin : public Plugin::MuxerPlugin {
public:
    explicit FFmpegMuxerPlugin(std::string name);
    ~ FFmpegMuxerPlugin() override;

    Plugin::Status Init() override;

    Plugin::Status Deinit() override;

    std::shared_ptr<Plugin::Allocator> GetAllocator() override;

    Plugin::Status SetCallback(Plugin::Callback* cb) override;

    Plugin::Status GetParameter(Plugin::Tag tag, Plugin::ValueType &value) override;

    Plugin::Status SetParameter(Plugin::Tag tag, const Plugin::ValueType &value) override;

    Plugin::Status Prepare() override;

    Plugin::Status Reset() override;

    Plugin::Status AddTrack(uint32_t &trackId) override;

    Plugin::Status SetTrackParameter(uint32_t trackId, Plugin::Tag tag, const Plugin::ValueType& value) override;

    Plugin::Status GetTrackParameter(uint32_t trackId, Plugin::Tag tag, Plugin::ValueType& value) override;

    Plugin::Status SetDataSink(const std::shared_ptr<Plugin::DataSink> &dataSink) override;

    Plugin::Status WriteHeader() override;

    Plugin::Status WriteFrame(const std::shared_ptr<Plugin::Buffer>& buffer) override;

    Plugin::Status WriteTrailer() override;

private:
    struct IOContext {
        std::shared_ptr<Plugin::DataSink> dataSink_{};
        int64_t pos_ {0};
        int64_t end_ {0};
    };

    static int32_t IoRead(void* opaque, uint8_t* buf, int bufSize);

    static int32_t IoWrite(void* opaque, uint8_t* buf, int bufSize);

    static int64_t IoSeek(void* opaque, int64_t offset, int whence);

    AVIOContext* InitAvIoCtx();

    static void DeInitAvIoCtx(AVIOContext* ptr);

    static void ResetIoCtx(IOContext& ioContext);

    Plugin::Status InitFormatCtxLocked();

    Plugin::Status Release();

    std::shared_ptr<AVOutputFormat> outputFormat_{};

    std::map<uint32_t, Plugin::TagMap> trackParameters_{};

    Plugin::TagMap generalParameters_{};

    OSAL::Mutex fmtMutex_{};
    std::shared_ptr<AVFormatContext> formatContext_{};

    IOContext ioContext_;
};
} // FFMux
} // Media
} // OHOS
#endif // HISTREAMER_FFMPEG_MUXER_PLUGIN_H
