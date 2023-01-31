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

#ifndef HISTREAMER_PLUGIN_CORE_HDI_CODEC_ADAPTER_H
#define HISTREAMER_PLUGIN_CORE_HDI_CODEC_ADAPTER_H

#include "codec_manager.h"
#include "codec_component_manager.h"
#include "interface/codec_plugin.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace CodecAdapter {
class HdiCodecAdapter : public CodecPlugin {
public:
    explicit HdiCodecAdapter(std::string componentName);
    ~HdiCodecAdapter() override = default;
    Status Init() override;
    Status Deinit() override;
    Status Prepare() override;
    Status Reset() override;
    Status Start() override;
    Status Stop() override;
    Status Flush() override;
    Status GetParameter(Plugin::Tag tag, Plugin::ValueType& value) override;
    Status SetParameter(Plugin::Tag tag, const Plugin::ValueType& value) override;
    Status QueueInputBuffer(const std::shared_ptr<Buffer>& inputBuffer, int32_t timeoutMs) override;
    Status QueueOutputBuffer(const std::shared_ptr<Buffer>& outputBuffers, int32_t timeoutMs) override;
    Status SetCallback(Callback* cb) override;
    Status SetDataCallback(DataCallback* dataCallback) override;

private:
    void NotifyInputBufferDone(const std::shared_ptr<Buffer>& input);
    void NotifyOutputBufferDone(const std::shared_ptr<Buffer>& output);

    // HDI callback
    static int32_t EventHandler(CodecCallbackType* self, OMX_EVENTTYPE event, EventInfo* info);
    static int32_t EmptyBufferDone(CodecCallbackType* self, int64_t appData, const OmxCodecBuffer* buffer);
    static int32_t FillBufferDone(CodecCallbackType* self, int64_t appData, const OmxCodecBuffer* buffer);

    std::shared_ptr<CodecManager> codecMgr_;
    struct CodecComponentType* codecComp_ {nullptr};
    struct CodecCallbackType* codecCallback_ {nullptr};
    uint32_t componentId_;
    CompVerInfo verInfo_ {};
};
} // namespace CodecAdapter
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PLUGIN_CORE_HDI_CODEC_ADAPTER_H
#endif