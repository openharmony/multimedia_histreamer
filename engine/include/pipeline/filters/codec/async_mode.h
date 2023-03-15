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

#ifndef HISTREAMER_PIPELINE_FILTER_ASYNC_MODE_H
#define HISTREAMER_PIPELINE_FILTER_ASYNC_MODE_H
#include <atomic>

#include "foundation/osal/thread/condition_variable.h"
#include "foundation/osal/thread/mutex.h"
#include "foundation/osal/thread/scoped_lock.h"
#include "foundation/osal/thread/task.h"
#include "foundation/utils/blocking_queue.h"
#include "pipeline/filters/codec/codec_mode.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
class AsyncMode : public CodecMode {
public:
    explicit AsyncMode(std::string name);
    ~AsyncMode() override;

    ErrorCode Configure() override;

    ErrorCode PushData(const std::string &inPort, const AVBufferPtr& buffer, int64_t offset) override;

    ErrorCode Stop() override;

    void FlushStart() override;

    void FlushEnd() override;

    void OnOutputBufferDone(const std::shared_ptr<Plugin::Buffer>& buffer) override;

    ErrorCode Prepare() override;

    ErrorCode Release() override;

protected:
    ErrorCode HandleFrame();

    ErrorCode DecodeFrame();

    ErrorCode FinishFrame();

    ErrorCode QueueAllBufferInPoolToPluginLocked();

    ErrorCode CheckBufferValidity(std::shared_ptr<AVBuffer>& buffer);

private:
    // dequeue from es bufferQ then enqueue to plugin
    std::shared_ptr<OHOS::Media::OSAL::Task> handleFrameTask_ {};

    // this task will queue output buffer and decode
    std::shared_ptr<OHOS::Media::OSAL::Task> decodeFrameTask_ {};

    // this task will dequeue from the plugin and then push to downstream
    std::shared_ptr<OHOS::Media::OSAL::Task> pushTask_ {nullptr};

    std::shared_ptr<OHOS::Media::BlockingQueue<OHOS::Media::AVBufferPtr>> inBufQue_ {nullptr};
    std::queue<AVBufferPtr> outBufQue_;  // PCM data
    mutable OSAL::Mutex renderMutex_ {};
    bool stopped_ {false};

    mutable OSAL::ConditionVariable cv_;
    std::atomic<bool> isNeedQueueInputBuffer_;
    mutable OSAL::Mutex mutex_;
};
} // namespace Pipeline
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PIPELINE_FILTER_ASYNC_MODE_H