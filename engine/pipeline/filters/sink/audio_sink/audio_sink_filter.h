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

#ifndef MEDIA_PIPELINE_AUDIO_SINK_FILTER_H
#define MEDIA_PIPELINE_AUDIO_SINK_FILTER_H

#include <atomic>

#include "foundation/error_code.h"
#include "osal/thread/condition_variable.h"
#include "osal/thread/mutex.h"
#include "pipeline/core/clock_provider.h"
#include "pipeline/core/filter_base.h"
#include "plugin/core/audio_sink.h"
#include "plugin/core/plugin_info.h"
#include "utils/utils.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
class AudioSinkFilter : public std::enable_shared_from_this<AudioSinkFilter>, public FilterBase, public ClockProvider {
public:
    explicit AudioSinkFilter(const std::string& name);
    ~AudioSinkFilter() override;

    void Init(EventReceiver* receiver, FilterCallback* callback) override;

    ErrorCode SetParameter(int32_t key, const Plugin::Any& value) override;

    ErrorCode GetParameter(int32_t key, Plugin::Any& value) override;

    bool Negotiate(const std::string& inPort, const std::shared_ptr<const Plugin::Capability>& upstreamCap,
                   Capability& upstreamNegotiatedCap) override;

    bool Configure(const std::string& inPort, const std::shared_ptr<const Plugin::Meta>& upstreamMeta) override;

    /**
     *
     * @param inPort
     * @param buffer
     * @param offset always ignore this parameter
     * @return
     */
    ErrorCode PushData(const std::string& inPort, AVBufferPtr buffer, int64_t offset) override;

    ErrorCode Start() override;
    ErrorCode Stop() override;

    ErrorCode Pause() override;
    ErrorCode Resume() override;

    void FlushStart() override;
    void FlushEnd() override;
    ErrorCode SetVolume(float volume);

    ErrorCode CheckPts(int64_t pts, int64_t& delta) override;
    ErrorCode GetCurrentPosition(int64_t& position) override;
    ErrorCode GetCurrentTimeNano(int64_t& nowNano) override;

private:
    ErrorCode SetPluginParameter(Tag tag, const Plugin::ValueType& value);
    ErrorCode ConfigureToPreparePlugin(const std::shared_ptr<const Plugin::Meta>& meta);
    ErrorCode ConfigureWithMeta(const std::shared_ptr<const Plugin::Meta>& meta);
    void ReportCurrentPosition(int64_t pts);

    ErrorCode UpdateLatestPts(int64_t pts);
    int64_t lastPts_ {0};
    int64_t latestSysClock_ {0}; // now+latency
    int64_t frameCnt_ {0};

    std::atomic<bool> pushThreadIsBlocking {false};
    bool isFlushing {false};
    OSAL::ConditionVariable startWorkingCondition_ {};
    OSAL::Mutex mutex_ {};

    std::shared_ptr<Plugin::AudioSink> plugin_ {};
};
} // namespace Pipeline
} // namespace Media
} // namespace OHOS
#endif