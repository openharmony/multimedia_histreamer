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

#ifndef HISTREAMER_PLUGIN_CORE_CODEC_ADAPTER_H
#define HISTREAMER_PLUGIN_CORE_CODEC_ADAPTER_H

#include "codec_component_manager.h"
#include <list>
#include "common/share_allocator.h"
#include "common/share_memory.h"
#include "common/surface_allocator.h"
#include "foundation/osal/thread/mutex.h"
#include "foundation/osal/thread/scoped_lock.h"
#include "interface/codec_plugin.h"
#include "OMX_Component.h"
#include "osal/thread/task.h"
#include "plugin/interface/hcodec_plugin.h"
#include "utils/blocking_queue.h"

namespace OHOS {
namespace Media {
namespace Plugin {
Status RegisterOneCodecPackage(const std::shared_ptr<OHOS::Media::Plugin::Register>& reg,
                               const std::string& packageName);
void UnRegisterOneCodecPackage(const std::string& packageName);

enum class PortIndex { PORT_INDEX_INPUT = 0, PORT_INDEX_OUTPUT = 1 };

class HdiAdapter : public CodecPlugin {
public:
    explicit HdiAdapter(std::string name);
    ~HdiAdapter() override;
public:
    Plugin::Status Init() override;
    Plugin::Status Deinit() override;
    Plugin::Status Prepare() override;
    Plugin::Status Reset() override;
    Plugin::Status Start() override;
    Plugin::Status Stop() override;
    Plugin::Status GetParameter(Plugin::Tag tag, Plugin::ValueType& value) override;
    Plugin::Status SetParameter(Plugin::Tag tag, const Plugin::ValueType& value) override;

    std::shared_ptr<Plugin::Allocator> GetAllocator() override;
    Status QueueInputBuffer(const std::shared_ptr<Buffer>& inputBuffer, int32_t timeoutMs) override;
    Status QueueOutputBuffer(const std::shared_ptr<Buffer>& outputBuffers, int32_t timeoutMs) override;

    Status Flush() override;
    Status SetCallback(Callback* cb) override;
    Status SetDataCallback(DataCallback* dataCallback) override;

private:
    struct BufferInfo {
        std::shared_ptr<OmxCodecBuffer> omxBuffer;
        std::shared_ptr<ShareMemory> avSharedPtr;
        std::shared_ptr<Buffer> outputBuffer;
        BufferInfo()
        {
            omxBuffer = nullptr;
            avSharedPtr = nullptr;
            outputBuffer = nullptr;
        }
        ~BufferInfo()
        {
            omxBuffer = nullptr;
            avSharedPtr = nullptr;
            outputBuffer = nullptr;
        }
    };
    using BufferInfo = struct BufferInfo;

    // HDI
    static constexpr uint32_t alignment_ = 16;
    std::map<uint32_t, std::shared_ptr<BufferInfo>> bufferInfoMap_;  // key is buferid
//    std::list<uint32_t> freeInBufferId_;
//    std::list<uint32_t> freeOutBufferId_;
    OHOS::Media::BlockingQueue<uint32_t> freeInBufferId_;
    OHOS::Media::BlockingQueue<uint32_t> freeOutBufferId_;


    int GetFreeBufferId();

private:
    template <typename T>
    void InitParam(T& param);

    template <typename T>
    void InitParamInOhos(T &param);

    int isHead4_ = 0;
    void HandleFrame();
    Status TransInputBuffer2OmxBuffer(const std::shared_ptr<Plugin::Buffer>& inputBuffer,
                                    std::shared_ptr<BufferInfo>& bufferInfo);

    void NotifyInputBufferDone(const std::shared_ptr<Buffer>& input);
    void NotifyOutputBufferDone(const std::shared_ptr<Buffer>& output);

    // HDI
    void ConfigOmx();
    void ConfigOmxPortDefine();
    int32_t CheckAndUseBufferHandle();

    void UseOmxBuffers();
    void GetBufferInfoOnPort(PortIndex portIndex);
    int32_t UseBufferOnInputPort(PortIndex portIndex, int bufferCount, int bufferSize);
    int32_t UseBufferOnOutputPort(PortIndex portIndex, int bufferCount, int bufferSize);

    bool isFirstCall_ = true;
    bool FillAllTheBuffer();
    void TransOutputBufToOmxBuf(const std::shared_ptr<Plugin::Buffer>& outputBuffer,
                                std::shared_ptr<OmxCodecBuffer>& omxBuffer);
    void SendEmptyBufToHdi();

    // HDI callback
    static int32_t EventHandler(CodecCallbackType *self, OMX_EVENTTYPE event, EventInfo *info);
    static int32_t EmptyBufferDone(CodecCallbackType *self, int64_t appData, const OmxCodecBuffer *buffer);
    static int32_t FillBufferDone(CodecCallbackType *self, int64_t appData, const OmxCodecBuffer *buffer);

    void WaitForEvent(OMX_U32 cmd);
    Status WaitForState(OMX_STATETYPE state);
    Status ChangeState(OMX_STATETYPE state);
    void HandelEventCmdComplete(OMX_U32 data1, OMX_U32 data2);
    void HandelEventStateSet(OMX_U32 data);

    OSAL::Mutex mutex_;
    OSAL::ConditionVariable cond_;
    int lastCmd_ = -2; // -1 for error cmd and -2 for invaild
    bool eventDone_ = false;

    OSAL::Mutex lockInputBuffers_;
    OSAL::Mutex lockOutputBuffers_;

private:
    struct CodecComponentType* codecComp_ {nullptr};
    struct CodecCallbackType* codecCallback_ {nullptr};
    uint32_t componentId_;
    struct CompVerInfo verInfo_;

    Callback* callback_ {nullptr};
    DataCallback* dataCallback_ {nullptr};
    std::list<std::shared_ptr<Buffer>> inBufQue_ {};
    OHOS::Media::BlockingQueue<std::shared_ptr<Buffer>> outBufQue_ {nullptr};

    OMX_STATETYPE curState_ {OMX_StateInvalid};
    OMX_STATETYPE targetState_ {OMX_StateInvalid};

    uint32_t width_;
    uint32_t height_;
    int32_t stride_;
    VideoPixelFormat pixelFormat_;
    uint32_t inBufferSize_;
    uint32_t inBufferCnt_;
    uint32_t outBufferSize_;
    uint32_t outBufferCnt_;

    std::shared_ptr<ShareAllocator> shaAlloc_ {nullptr};
    bool isFlush_ = false;
};
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PLUGIN_CORE_CODEC_ADAPTER_H
