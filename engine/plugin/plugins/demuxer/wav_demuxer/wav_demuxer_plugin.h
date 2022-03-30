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

#ifndef WAV_DEMUXER_PLUGIN_H
#define WAV_DEMUXER_PLUGIN_H

#include <memory>
#include <string>
#include <vector>

#include "core/plugin_register.h"
#include "plugin/interface/demuxer_plugin.h"
#include "foundation/osal/thread/mutex.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace WavPlugin {
class WavDemuxerPlugin : public DemuxerPlugin {
public:
    explicit WavDemuxerPlugin(std::string name);
    ~WavDemuxerPlugin() override;

    Status SetDataSource(const std::shared_ptr<DataSource>& source) override;
    Status GetMediaInfo(MediaInfo& mediaInfo) override;
    Status ReadFrame(Buffer& outBuffer, int32_t timeOutMs) override;
    Status SeekTo(int32_t trackId, int64_t hstTime, SeekMode mode) override;
    Status Reset() override;
    Status GetParameter(Tag tag, ValueType &value) override;
    Status SetParameter(Tag tag, const ValueType &value) override;
    std::shared_ptr<Allocator> GetAllocator() override;
    Status SetCallback(Callback* cb) override;
    size_t GetTrackCount() override;
    Status SelectTrack(int32_t trackId) override;
    Status UnselectTrack(int32_t trackId) override;
    Status GetSelectedTracks(std::vector<int32_t>& trackIds) override;
    Status GetDataFromSource();
private:
    struct IOContext {
        std::shared_ptr<DataSource> dataSource {nullptr};
        int64_t offset {0};
        bool eos {false};
    };
    size_t              fileSize_;
    IOContext           ioContext_;
    uint32_t            dataOffset_;
    bool                isSeekable_;
};
} // namespace Wav
} // namespace Plugin
} // namespace Media
} // namespace OHOS

#endif // WAV_DEMUXER_PLUGIN_H
