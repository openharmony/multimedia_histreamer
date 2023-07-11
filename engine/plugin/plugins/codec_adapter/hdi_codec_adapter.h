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
#if defined(VIDEO_SUPPORT)

#ifndef HISTREAMER_PLUGIN_HDI_CODEC_ADAPTER_H
#define HISTREAMER_PLUGIN_HDI_CODEC_ADAPTER_H
#include <limits>
#include <list>
#include "codec_buffer_pool.h"
#include "codec_cmd_executor.h"
#include "codec_component_manager.h"
#include "codec_manager.h"
#include "codec_port.h"
#include "foundation/utils/blocking_queue.h"
#include "plugin/interface/codec_plugin.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace CodecAdapter {
class HdiCodecAdapter : public CodecPlugin {
public:
    HdiCodecAdapter(std::string componentName, std::string pluginMime);
    ~HdiCodecAdapter() override;
    Status Init() override;
    Status Deinit() override;
    Status Prepare() override;
    Status Reset() override;
    Status Start() override;
    Status Stop() override;
    Status Flush() override;
    Status GetParameter(Plugin::Tag tag, Plugin::ValueType& value) override;
    Status SetParameter(Plugin::Tag tag, const Plugin::ValueType& value) override;
    std::shared_ptr<Plugin::Allocator> GetAllocator() override;
    Status QueueInputBuffer(const std::shared_ptr<Buffer>& inputBuffer, int32_t timeoutMs) override;
    Status QueueOutputBuffer(const std::shared_ptr<Buffer>& outputBuffer, int32_t timeoutMs) override;
    Status SetCallback(Callback* cb) override;
    Status SetDataCallback(DataCallback* dataCallback) override;

private:
    Status InitVersion();
    Status InitPortIndex();
    Status FreeBuffers();
    void HandleFrame();
    bool isFirstCall_ = true;
    bool FillAllTheOutBuffer();

private:
    void NotifyInputBufferDone(const std::shared_ptr<Buffer>& input);
    void NotifyOutputBufferDone(const std::shared_ptr<Buffer>& output);

    // HDI callback
    static int32_t EventHandler(CodecCallbackType* self, OMX_EVENTTYPE event, EventInfo* info);
    static int32_t EmptyBufferDone(CodecCallbackType* self, int64_t appData, const OmxCodecBuffer* omxBuffer);
    static int32_t FillBufferDone(CodecCallbackType* self, int64_t appData, const OmxCodecBuffer* omxBuffer);

    Status ConfigOmx();

    Status ChangeState(OMX_STATETYPE state);
    Status WaitForState(OMX_STATETYPE state);

    CodecComponentType* codecComp_ {nullptr};
    CodecCallbackType* codecCallback_ {nullptr};
    std::string componentName_ {};
    uint32_t componentId_{};
    std::string pluginMime_{};
    std::list<std::shared_ptr<Buffer>> inBufQue_ {};
    std::shared_ptr<OHOS::Media::BlockingQueue<std::shared_ptr<Buffer>>> outBufQue_{};

    uint32_t inBufferSize_{};
    uint32_t inBufferCnt_{};
    uint32_t outBufferSize_{};
    uint32_t outBufferCnt_{};
    std::shared_ptr<CodecBufferPool> inBufPool_ {};
    std::shared_ptr<CodecBufferPool> outBufPool_ {};
    MemoryType inputMemoryType_ {MemoryType::VIRTUAL_ADDR};
    MemoryType outputMemoryType_ {MemoryType::VIRTUAL_ADDR};

    bool portConfigured_ {false};
    uint32_t width_{};
    uint32_t height_{};
    uint32_t frameRate_{};
    VideoPixelFormat pixelFormat_ {};
    int64_t bitRate_{};

    Callback* callback_ {nullptr};
    DataCallback* dataCallback_ {nullptr};

    OSAL::Mutex lockInputBuffers_;

    std::shared_ptr<ShareAllocator> shaAlloc_ {nullptr};
    std::atomic<bool> isFlushing_ {false};
    uint32_t inPortIndex_{};
    uint32_t outPortIndex_{};
    CompVerInfo verInfo_ {};
    OMX_PORT_PARAM_TYPE portParam_ = {};
    std::shared_ptr<CodecPort> inCodecPort_{};
    std::shared_ptr<CodecPort> outCodecPort_{};

    OSAL::Mutex fillAllTheOutBufferMutex_;
    OSAL::Mutex bufferMetaMutex_;
    std::map<int64_t, std::shared_ptr<BufferMeta>> bufferMetaMap_;

    std::shared_ptr<CodecCmdExecutor> codecCmdExecutor_{};
    OMX_STATETYPE curState_ {OMX_StateInvalid};
    OMX_STATETYPE targetState_ {OMX_StateInvalid};
};
} // namespace CodecAdapter
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PLUGIN_HDI_CODEC_ADAPTER_H
#endif