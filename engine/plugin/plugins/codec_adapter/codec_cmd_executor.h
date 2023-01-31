/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#if !defined(OHOS_LITE) && defined(VIDEO_SUPPORT)

#ifndef HISTREAMER_PLUGIN_CODEC_CMD_EXECUTOR_H
#define HISTREAMER_PLUGIN_CODEC_CMD_EXECUTOR_H

#include "codec_component_type.h"
#include "common/plugin_types.h"
#include "foundation/osal/thread/condition_variable.h"
#include "foundation/osal/thread/mutex.h"
#include "OMX_Core.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace CodecAdapter {
class CodecCmdExecutor {
public:
    CodecCmdExecutor() = default;
    ~CodecCmdExecutor() = default;

    Status OnEvent(OMX_EVENTTYPE event, EventInfo* info);

    Status SendCmd(OMX_COMMANDTYPE cmd, bool wait);
    Status WaitCmdComplete();

private:
    void HandelEventCmdComplete(uint32_t data1, uint32_t data2);
    void HandelEventCmdCompleteEvent(OMX_U32 data1, OMX_U32 data2);
    void HandelEventStateSet(OMX_U32 data1, OMX_U32 data2);
    void HandelEventFlush(OMX_U32 data1, OMX_U32 data2);
    void HandleEventPortSettingsChangedEvent(OMX_U32 data1, OMX_U32 data2);
    void HandleEventError(OMX_U32 data1);

    struct CodecComponentType* codecComp_ {nullptr};
    OSAL::Mutex mutex_;
    OSAL::ConditionVariable cond_;
    int lastCmd_ = -2; // -1 for error cmd and -2 for invaild
    bool eventDone_ = false;
};
} // namespace CodecAdapter
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PLUGIN_CODEC_CMD_EXECUTOR_H
#endif