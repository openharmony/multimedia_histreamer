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
#ifdef RECORDER_SUPPORT
#ifndef HISTREAMER_PIPELINE_MUXER_FILTER_H
#define HISTREAMER_PIPELINE_MUXER_FILTER_H

#include "filter_base.h"
#include "plugin/core/muxer.h"
#include "plugin/core/plugin_info.h"
#include "plugin/interface/muxer_plugin.h"
#include "plugin/core/plugin_meta.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
class DataCollector;
class DataSpliter;
class MuxerFilter : public FilterBase {
public:
    explicit MuxerFilter(std::string name);
    ~MuxerFilter() override;

    void Init(EventReceiver* receiver, FilterCallback* callback) override;

    bool Negotiate(const std::string& inPort, const std::shared_ptr<const Capability>& upstreamCap,
                   Capability& upstreamNegotiatedCap) override;

    bool Configure(const std::string& inPort, const std::shared_ptr<const Plugin::Meta>& upstreamMeta) override;

    ErrorCode SetOutputFormat(std::string containerMime);
    ErrorCode AddTrack(std::shared_ptr<InPort>& trackPort);
    ErrorCode SetMaxDurationUs(uint64_t maxDurationUs);
    ErrorCode SplitMuxBegin();
    ErrorCode SplitMuxEnd();

    /**
     *
     * @param inPort
     * @param buffer
     * @param offset always ignore is parameter
     * @return
     */
    ErrorCode PushData(const std::string& inPort, AVBufferPtr buffer, int64_t offset) override;
private:
    class MuxerDataSink : public Plugin::DataSinkHelper {
    public:
        virtual Plugin::Status WriteAt(int64_t offset, const std::shared_ptr<Plugin::Buffer>& buffer);
        MuxerFilter* muxerFilter_;
    };

    int32_t GetTrackIdByInPort(const std::shared_ptr<InPort>& inPort);
    int32_t UpdateTrackIdOfInPort(const std::shared_ptr<InPort>& inPort, int32_t trackId);

    bool UpdateAndInitPluginByInfo(const std::shared_ptr<Plugin::PluginInfo>& selectedPluginInfo);

    ErrorCode ConfigureToStart();
    ErrorCode AddTrackThenConfigure(const std::pair<std::string, Plugin::Meta>& meta);

    std::string containerMime_{};
    std::vector<std::pair<std::string, uint32_t>> portTrackIdMap_{};
    std::shared_ptr<Plugin::Muxer> plugin_{};
    std::shared_ptr<Plugin::PluginInfo> targetPluginInfo_ {nullptr};
    std::shared_ptr<DataSpliter> dataSpliter_{};
    std::shared_ptr<DataCollector> dataCollector_{};
    std::vector<std::pair<std::string, Capability>> capabilityCache_{};
    std::vector<std::pair<std::string, Plugin::Meta>> metaCache_{};
    bool hasWriteHeader_{false};
    std::shared_ptr<MuxerDataSink> muxerDataSink_;
};
} // Pipeline
} // Media
} // OHOS
#endif // HISTREAMER_PIPELINE_MUXER_FILTER_H
#endif
