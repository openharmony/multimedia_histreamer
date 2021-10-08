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

#ifdef VIDEO_SUPPORT

#ifndef MEDIA_PIPELINE_VIDEO_SINK_FILTER_H
#define MEDIA_PIPELINE_VIDEO_SINK_FILTER_H

#include <atomic>
#include "osal/thread/task.h"
#include "foundation/utils.h"
#include "foundation/error_code.h"
#include "pipeline/core/filter_base.h"
#include "foundation/blocking_queue.h"
#include "osal/thread/mutex.h"
#include "osal/thread/condition_variable.h"
#include "plugin/core/video_sink.h"
#include "plugin/core/plugin_info.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
class VideoSinkAdapter;
class VideoSinkFilter : public FilterBase {
public:
    explicit VideoSinkFilter(const std::string& name);
    ~VideoSinkFilter() override;

    void Init(EventReceiver* receiver, FilterCallback* callback) override;

    ErrorCode SetParameter(int32_t key, const Plugin::Any& value) override;

    ErrorCode GetParameter(int32_t key, Plugin::Any& value) override;

    bool Negotiate(const std::string &inPort, const std::shared_ptr<const OHOS::Media::Meta> &inMeta,
                   CapabilitySet &outCaps) override;

    ErrorCode PushData(const std::string &inPort, AVBufferPtr buffer) override;

    ErrorCode Start() override;
    ErrorCode Stop() override;

    ErrorCode Pause() override;
    ErrorCode Resume() override;

    void FlushStart() override;
    void FlushEnd() override;

private:
    ErrorCode ConfigurePluginParams(const std::shared_ptr<const Meta> &meta);
    ErrorCode ConfigureLocked(const std::shared_ptr<const OHOS::Media::Meta> &meta);
    void RenderFrame();
    bool DoSync();

    std::shared_ptr<OHOS::Media::BlockingQueue<AVBufferPtr>> inBufQueue_;
    std::shared_ptr<OHOS::Media::OSAL::Task> renderTask_;
    std::shared_ptr<VideoSinkAdapter> adapter_;
    std::atomic<bool> pushThreadIsBlocking_ {false};
    bool isFlushing_ {false};
    OSAL::ConditionVariable startWorkingCondition_ {};
    OSAL::Mutex mutex_;

    std::shared_ptr<Plugin::VideoSink> plugin_ {nullptr};
    std::shared_ptr<Plugin::PluginInfo> pluginInfo_ {nullptr};
};
}
}
}

#endif // MEDIA_PIPELINE_VIDEO_SINK_FILTER_H

#endif