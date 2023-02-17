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

#define HST_LOG_TAG "HdiCodecAdapter"

#include "hdi_codec_adapter.h"
#include <utility>
#include "codec_callback_type_stub.h"
#include "codec_callback_if.h"
#include "codec_component_if.h"
#include "codec_omx_ext.h"
#include "codec_utils.h"
#include "foundation/log.h"
#include "hdf_base.h"
#include "hdi_codec_manager.h"
#include "pipeline/core/plugin_attr_desc.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace CodecAdapter {
constexpr size_t DEFAULT_OUT_BUFFER_QUEUE_SIZE = 21;

// hdi adapter callback
int32_t HdiCodecAdapter::EventHandler(CodecCallbackType* self, OMX_EVENTTYPE event, EventInfo* info)
{
//    auto hdiAdapter = reinterpret_cast<HdiCodecAdapter*>(info->appData);
    MEDIA_LOG_I("appData: " PUBLIC_LOG_D64 ", eEvent: " PUBLIC_LOG_D32
                ", nData1: " PUBLIC_LOG_U32 ", nData2: " PUBLIC_LOG_U32,
                info->appData, static_cast<int>(event), info->data1, info->data2);
    auto hdiAdapter = reinterpret_cast<HdiCodecAdapter*>(info->appData);
    hdiAdapter->codecCmdExecutor_->OnEvent(event, info);
    MEDIA_LOG_D("EventHandler-callback end");
    return HDF_SUCCESS;
}

int32_t HdiCodecAdapter::EmptyBufferDone(CodecCallbackType* self, int64_t appData, const OmxCodecBuffer* buffer)
{
    MEDIA_LOG_DD("EmptyBufferDone-callback begin, bufferId: " PUBLIC_LOG_U32, buffer->bufferId);
    auto hdiAdapter = reinterpret_cast<HdiCodecAdapter*>(appData);
    hdiAdapter->inBufPool_->UseBufferDone(buffer->bufferId);
    if (!hdiAdapter->isFlushing_) {
        hdiAdapter->HandleFrame();
    }
    MEDIA_LOG_D("EmptyBufferDone-callback end, free in buffer count: " PUBLIC_LOG_ZU,
                hdiAdapter->inBufPool_->EmptyBufferCount());
    return HDF_SUCCESS;
}

int32_t HdiCodecAdapter::FillBufferDone(CodecCallbackType* self, int64_t appData, const OmxCodecBuffer* omxBuffer)
{
    MEDIA_LOG_DD("FillBufferDone-callback begin, bufferId: " PUBLIC_LOG_U32 ", flag: " PUBLIC_LOG_U32
                         ", pts: " PUBLIC_LOG_D64, omxBuffer->bufferId, omxBuffer->flag, omxBuffer->pts);
    auto hdiAdapter = reinterpret_cast<HdiCodecAdapter*>(appData);
    auto codecBuffer = hdiAdapter->outBufPool_->GetBuffer(omxBuffer->bufferId);
    std::shared_ptr<Plugin::Buffer> outputBuffer = nullptr;
    (void)codecBuffer->Unbind(outputBuffer, omxBuffer);
    hdiAdapter->outBufPool_->UseBufferDone(omxBuffer->bufferId);
    if (hdiAdapter->isFlushing_) {
        MEDIA_LOG_DD("hdi adapter is flushing, ignore this data");
        outputBuffer = nullptr;
        return HDF_SUCCESS;
    }
    hdiAdapter->NotifyOutputBufferDone(outputBuffer);
    (void)hdiAdapter->FillAllTheOutBuffer(); // call FillThisBuffer() again
    MEDIA_LOG_D("FillBufferDone-callback end, free out buffer count: " PUBLIC_LOG_ZU,
                hdiAdapter->outBufPool_->EmptyBufferCount());
    return HDF_SUCCESS;
}

HdiCodecAdapter::HdiCodecAdapter(std::string componentName, std::shared_ptr<CodecManager>& codecManager)
    : CodecPlugin(std::move(componentName)),
      codecMgr_(codecManager),
      outBufQue_("hdiAdapterOutQueue", DEFAULT_OUT_BUFFER_QUEUE_SIZE)
{
    shaAlloc_ = std::make_shared<ShareAllocator>(Plugin::ShareMemType::READ_WRITE_TYPE);
    FALSE_LOG_MSG(codecMgr_ != nullptr, "Get codec manager failed");
}

HdiCodecAdapter::~HdiCodecAdapter()
{
    if (codecCallback_) {
        CodecCallbackTypeStubRelease(codecCallback_);
        codecCallback_ = nullptr;
    }
}

Status HdiCodecAdapter::Init()
{
    MEDIA_LOG_D("codec adapter init begin");
    auto firstDotPos = pluginName_.find_first_of('.');
    MEDIA_LOG_D("pluginName_: " PUBLIC_LOG_S, pluginName_.c_str());
    if (firstDotPos == std::string::npos) {
        MEDIA_LOG_E("create codec handle error with plugin name " PUBLIC_LOG_S ", which is wrong format",
                    pluginName_.c_str());
        return Status::ERROR_UNSUPPORTED_FORMAT;
    }
    componentName_ = pluginName_.substr(firstDotPos + 1); // ComponentCapability.compName
    codecCallback_ = CodecCallbackTypeStubGetInstance();
    FALSE_RETURN_V_MSG(codecCallback_ != nullptr, Status::ERROR_NULL_POINTER, "create callback_ failed");

    codecCallback_->EventHandler = &HdiCodecAdapter::EventHandler;
    codecCallback_->EmptyBufferDone = &HdiCodecAdapter::EmptyBufferDone;
    codecCallback_->FillBufferDone = &HdiCodecAdapter::FillBufferDone;

    int32_t ret = codecMgr_->CreateComponent(&codecComp_, componentId_, const_cast<char*>(componentName_.c_str()),
                                             (int64_t)this, codecCallback_);
    FALSE_RETURN_V_MSG(codecComp_ != nullptr, Status::ERROR_NULL_POINTER,
                       "create component failed, retVal = " PUBLIC_LOG_D32, (int)ret);

    // 获取组件版本号
    (void)memset_s(&verInfo_, sizeof(verInfo_), 0, sizeof(verInfo_));
    ret = codecComp_->GetComponentVersion(codecComp_, &verInfo_);
    FALSE_RETURN_V_MSG_E(ret == HDF_SUCCESS, Status::ERROR_INVALID_DATA,
                         "get component version failed, ret: " PUBLIC_LOG_D32, ret);
    inPortIndex_ = portParam_.nStartPortNumber;
    outPortIndex_ = portParam_.nStartPortNumber + 1;
    inCodecPort_ = std::make_shared<CodecPort>(codecComp_, inPortIndex_, verInfo_);
    outCodecPort_ = std::make_shared<CodecPort>(codecComp_, outPortIndex_, verInfo_);
    codecCmdExecutor_ = std::make_shared<CodecCmdExecutor>(codecComp_, inPortIndex_);
    portConfigured_ = false;
    MEDIA_LOG_D("codec adapter init end, component Id = " PUBLIC_LOG_D32, componentId_);
    return Status::OK;
}

Status HdiCodecAdapter::Deinit() 
{
    MEDIA_LOG_D("HdiAdapter DeInit Enter");
    FALSE_RETURN_V_MSG_E(Reset() == Status::OK, Status::ERROR_INVALID_DATA, "Reset value failed");
    if (codecMgr_) {
        auto ret = codecMgr_->DestroyComponent(codecComp_, componentId_);
        FALSE_RETURN_V_MSG_E(ret == HDF_SUCCESS, Status::ERROR_INVALID_OPERATION,
            "HDI destroy component failed, ret = " PUBLIC_LOG_S, HdfStatus2String(ret).c_str());
    }
    if (codecComp_) {
        CodecComponentTypeRelease(codecComp_);
        codecComp_ = nullptr;
    }
    if (codecCallback_) {
        CodecCallbackTypeStubRelease(codecCallback_);
        codecCallback_ = nullptr;
    }
    MEDIA_LOG_D("HdiAdapter DeInit End;");
    return Status::OK;
}

Status HdiCodecAdapter::Prepare() 
{
    FALSE_RETURN_V_MSG_E(ChangeState(OMX_StateIdle) == Status::OK,
        Status::ERROR_WRONG_STATE, "Change omx state to idle failed");
    FALSE_RETURN_V_MSG_E(WaitForState(OMX_StateIdle) == Status::OK,
        Status::ERROR_WRONG_STATE, "Wait omx state to idle failed");
    outBufQue_.SetActive(true);
    inBufPool_ = std::make_shared<CodecBufferPool>(codecComp_, verInfo_, inPortIndex_);
    outBufPool_ = std::make_shared<CodecBufferPool>(codecComp_, verInfo_, outPortIndex_);
    OHOS::Media::BlockingQueue<std::shared_ptr<Buffer>> inBufQue("TempInBufferQue", inBufferCnt_);
    for (uint32_t i = 0; i < inBufferCnt_; i++) {
        inBufQue.Push(Buffer::CreateDefaultBuffer(BufferMetaType::VIDEO, inBufferSize_, shaAlloc_));
    }
    inBufPool_->UseBuffers(inBufQue, MemoryType::SHARE_MEMORY);
    outBufPool_->UseBuffers(outBufQue_, MemoryType::SURFACE_BUFFER);
    MEDIA_LOG_D("prepare end");
    return Status::OK;
}

Status HdiCodecAdapter::Reset()
{
    FALSE_RETURN_V_MSG_E(ChangeState(OMX_StateIdle) == Status::OK,
        Status::ERROR_WRONG_STATE, "Change omx state to idle failed");
    FALSE_RETURN_V_MSG_E(WaitForState(OMX_StateIdle) == Status::OK,
        Status::ERROR_WRONG_STATE, "Wait omx state to idle failed");
    curState_ = OMX_StateIdle;
    auto val = inBufPool_->FreeBuffers();
    FALSE_RETURN_V_MSG_E(val == Status::OK, val, "free buffers failed");
    val = outBufPool_->FreeBuffers();
    FALSE_RETURN_V_MSG_E(val == Status::OK, val, "free buffers failed");
    outBufQue_.SetActive(false);
    outBufQue_.Clear();
    inBufQue_.clear();
    width_ = 0;
    height_ = 0;
    stride_ = 0;
    inBufferSize_ = 0;
    inBufferCnt_ = 0;
    outBufferSize_= 0;
    outBufferCnt_ = 0;
    return Status::OK;
}

Status HdiCodecAdapter::Start()
{
    MEDIA_LOG_D("start begin");
    FALSE_RETURN_V_MSG_E(ChangeState(OMX_StateExecuting) == Status::OK,
        Status::ERROR_WRONG_STATE, "Change omx state to idle failed");
    FALSE_RETURN_V_MSG_E(WaitForState(OMX_StateExecuting) == Status::OK,
        Status::ERROR_WRONG_STATE, "Wait omx state to idle failed");
    curState_ = OMX_StateExecuting;
    outBufQue_.SetActive(true);
    if (!FillAllTheOutBuffer()) {
        MEDIA_LOG_E("Fill all buffer error");
        return Status::ERROR_UNKNOWN;
    }
    MEDIA_LOG_D("start end");
    return Status::OK;
}

Status HdiCodecAdapter::Stop() 
{
    MEDIA_LOG_D("Stop Enter");
    outBufQue_.SetActive(false);
    MEDIA_LOG_D("Stop End");
    return Status::OK;
}

Status HdiCodecAdapter::Flush() 
{
    MEDIA_LOG_D("HdiCodecAdapter Flush begin");
    isFlushing_ = true;
    {
        OSAL::ScopedLock l(lockInputBuffers_);
        inBufQue_.clear();
    }
    // -1: Refresh input and output ports
    auto ret = codecCmdExecutor_->SendCmd(OMX_CommandFlush, inPortIndex_);
    FALSE_RETURN_V_MSG_E(ret == Status::OK, Status::ERROR_UNKNOWN, "Flush inPort failed");
    ret = codecCmdExecutor_->SendCmd(OMX_CommandFlush, outPortIndex_);
    FALSE_RETURN_V_MSG_E(ret == Status::OK, Status::ERROR_UNKNOWN, "Flush outPort failed");
    codecCmdExecutor_->WaitCmdResult(OMX_CommandFlush, inPortIndex_);
    codecCmdExecutor_->WaitCmdResult(OMX_CommandFlush, outPortIndex_);
    isFlushing_ = false;
    MEDIA_LOG_D("HdiAdapter Flush end");
    return Status::OK;
}

Status HdiCodecAdapter::GetParameter(Plugin::Tag tag, ValueType &value) 
{
    MEDIA_LOG_D("GetParameter begin");
    switch (tag) {
        case Tag::REQUIRED_OUT_BUFFER_CNT:
            if (outBufferCnt_ == std::numeric_limits<uint32_t>::max()) {
                return Status::ERROR_INVALID_DATA;
            }
            value = outBufferCnt_;
            break;
        case Tag::REQUIRED_OUT_BUFFER_SIZE:
            if (outBufferSize_ == std::numeric_limits<uint32_t>::max()) {
                return Status::ERROR_INVALID_DATA;
            }
            value = outBufferSize_;
            break;
        default:
            MEDIA_LOG_W("ignore this tag: " PUBLIC_LOG_S, Pipeline::Tag2String(tag));
            break;
    }
    return Status::OK;
}

Status HdiCodecAdapter::SetParameter(Plugin::Tag tag, const ValueType &value) 
{
    MEDIA_LOG_D("SetParameter begin");
    switch (tag) {
        case Tag::VIDEO_WIDTH:
            width_ = Plugin::AnyCast<uint32_t>(value);
            stride_ = AlignUp(width_, 16); // 16 byte alignment
            break;
        case Tag::VIDEO_HEIGHT:
            height_ = Plugin::AnyCast<uint32_t>(value);
            break;
        case Tag::VIDEO_PIXEL_FORMAT:
            pixelFormat_ = Plugin::AnyCast<VideoPixelFormat>(value);
            break;
        case Tag::VIDEO_FRAME_RATE:
            frameRate_ = Plugin::AnyCast<uint32_t>(value);
            break;
        default:
            MEDIA_LOG_W("ignore this tag: " PUBLIC_LOG_S, Pipeline::Tag2String(tag));
            break;
    }
    if (width_ != 0 && height_ != 0 && pixelFormat_ != VideoPixelFormat::UNKNOWN && !portConfigured_) {
        FALSE_RETURN_V_MSG_E(ConfigOmx() == Status::OK, Status::ERROR_INVALID_OPERATION, "Configure omx failed");
    }
    MEDIA_LOG_D("SetParameter end");
    return Status::OK;
}

Status HdiCodecAdapter::ConfigOmx()
{
    TagMap tagMap;
    tagMap.Insert<Tag::MIME>(ComponentNameToMime(componentName_));
    tagMap.Insert<Tag::VIDEO_WIDTH>(width_);
    tagMap.Insert<Tag::VIDEO_HEIGHT>(height_);
    tagMap.Insert<Tag::VIDEO_FRAME_RATE>(frameRate_);
    tagMap.Insert<Tag::VIDEO_PIXEL_FORMAT>(pixelFormat_);
    auto ret = inCodecPort_->Config(tagMap);
    FALSE_RETURN_V_MSG_E(ret == Status::OK, Status::ERROR_INVALID_OPERATION, "Configure inCodecPort failed");
    ret = outCodecPort_->Config(tagMap);
    FALSE_RETURN_V_MSG_E(ret == Status::OK, Status::ERROR_INVALID_OPERATION, "Configure outCodecPort failed");
    PortInfo portInfo;
    inCodecPort_->QueryParam(portInfo);
    inBufferCnt_ = portInfo.bufferCount;
    inBufferSize_ = portInfo.bufferSize;
    if (!portInfo.enabled) {
        codecCmdExecutor_->SendCmd(OMX_CommandPortEnable, inPortIndex_);
        (void) codecCmdExecutor_->WaitCmdResult(OMX_CommandPortEnable, inPortIndex_);
    }
    outCodecPort_->QueryParam(portInfo);
    outBufferCnt_ = portInfo.bufferCount;
    outBufferSize_ = portInfo.bufferSize;
    if (!portInfo.enabled) {
        codecCmdExecutor_->SendCmd(OMX_CommandPortEnable, outPortIndex_);
        (void) codecCmdExecutor_->WaitCmdResult(OMX_CommandPortEnable, outPortIndex_);
    }
    portConfigured_ = true;
    return Status::OK;
}

std::shared_ptr<Plugin::Allocator> HdiCodecAdapter::GetAllocator()
{
    MEDIA_LOG_D("GetAllocator begin");
    return nullptr;
}

Status HdiCodecAdapter::QueueInputBuffer(const std::shared_ptr<Buffer>& inputBuffer, int32_t timeoutMs)
{
    if (inputBuffer->IsEmpty() && !(inputBuffer->flag & BUFFER_FLAG_EOS)) {
        MEDIA_LOG_E("empty input buffer without eos flag");
        return Status::ERROR_INVALID_DATA;
    }
    {
        OSAL::ScopedLock l(lockInputBuffers_);
        inBufQue_.push_back(inputBuffer);
        MEDIA_LOG_D("QueueInputBuffer end, inBufQue_.size: " PUBLIC_LOG_ZU, inBufQue_.size());
    }
    HandleFrame();
    return Status::OK;
}

// 循环从输入buffer队列中取出一个buffer，转换成 omxBuffer 后调用 HDI 的 EmptyThisBuffer() 进行解码
void HdiCodecAdapter::HandleFrame()
{
    MEDIA_LOG_DD("handle frame begin");
    while (inBufPool_->EmptyBufferCount()) {
        std::shared_ptr<Buffer> inputBuffer = nullptr;
        std::shared_ptr<CodecBuffer> codecBuffer = nullptr;
        uint32_t inBufferId = 0;
        {
            OSAL::ScopedLock l(lockInputBuffers_);
            if (inBufQue_.empty()) {
                return;
            }
            inputBuffer = inBufQue_.front();
            codecBuffer = inBufPool_->GetBuffer();
            if (!codecBuffer) {
                return;
            }
            inBufQue_.pop_front();
        }
        FALSE_RETURN_MSG(codecBuffer->Copy(inputBuffer) == Status::OK,
                         "Copy inBuffer into codecBuffer fail");
        auto ret = HdiEmptyThisBuffer(codecComp_, codecBuffer->GetOmxBuffer().get());
        FALSE_LOG_MSG(ret == HDF_SUCCESS, "call EmptyThisBuffer() error, bufferId: " PUBLIC_LOG_D32, inBufferId);
        NotifyInputBufferDone(inputBuffer);
    }
    MEDIA_LOG_DD("handle frame end");
}

Status HdiCodecAdapter::QueueOutputBuffer(const std::shared_ptr<Buffer>& outputBuffers, int32_t timeoutMs)
{
    outBufQue_.Push(outputBuffers);
    if (curState_ == OMX_StateExecuting) {
        FillAllTheOutBuffer();
    }
    MEDIA_LOG_DD("QueueOutputBuffer end");
    return Status::OK;
}

Status HdiCodecAdapter::SetCallback(Callback* cb)
{
    MEDIA_LOG_D("SetCallback begin");
    callback_ = cb;
    codecCmdExecutor_->SetCallback(cb);
    return Status::OK;
}

Status HdiCodecAdapter::SetDataCallback(DataCallback* dataCallback)
{
    MEDIA_LOG_D("SetDataCallback begin");
    dataCallback_ = dataCallback;
    return Status::OK;
}

void HdiCodecAdapter::NotifyInputBufferDone(const std::shared_ptr<Buffer>& input)
{
    if (dataCallback_ != nullptr) {
        dataCallback_->OnInputBufferDone(input);
    }
    MEDIA_LOG_DD("NotifyInputBufferDone end");
}

void HdiCodecAdapter::NotifyOutputBufferDone(const std::shared_ptr<Buffer>& output)
{
    if (dataCallback_ != nullptr) {
        dataCallback_->OnOutputBufferDone(output);
    }
    MEDIA_LOG_DD("NotifyOutputBufferDone end");
}

bool HdiCodecAdapter::FillAllTheOutBuffer()
{
    MEDIA_LOG_DD("FillAllTheBuffer begin");
    if (isFirstCall_) {
        isFirstCall_ = false;
        for (uint32_t i = 0; i < outBufferCnt_; ++i) {
            auto codecBuffer = outBufPool_->GetBuffer();
            auto ret = HdiFillThisBuffer(codecComp_, codecBuffer->GetOmxBuffer().get());
            FALSE_RETURN_V_MSG_E(ret == HDF_SUCCESS, false, "call FillThisBuffer() error, ret: " PUBLIC_LOG_S
                ", isFirstCall: " PUBLIC_LOG_D32, HdfStatus2String(ret).c_str(), isFirstCall_);
        }
    } else {
        while (!outBufQue_.Empty()) {
            if (!outBufPool_->EmptyBufferCount()) {
                MEDIA_LOG_D("outBufQue_ have data, but freeBufferId is empty");
                return false;
            }
            auto codecBuffer = outBufPool_->GetBuffer();
            FALSE_RETURN_V(codecBuffer != nullptr, false);
            auto outputBuffer = outBufQue_.Pop(1);
            if (outputBuffer == nullptr) {
                MEDIA_LOG_E("output buffer is nullptr");
                outBufPool_->UseBufferDone(codecBuffer->GetBufferId());
                return false;
            }
            codecBuffer->Rebind(outputBuffer); // 这里outBuf需要保存到codecBuffer里面，方便往下一节点传数据
            auto ret = HdiFillThisBuffer(codecComp_, codecBuffer->GetOmxBuffer().get());
            FALSE_RETURN_V_MSG_E(ret == HDF_SUCCESS, false, "call FillThisBuffer() error, ret: " PUBLIC_LOG_S
                ", isFirstCall: " PUBLIC_LOG_D32, HdfStatus2String(ret).c_str(), isFirstCall_);
        }
    }
    MEDIA_LOG_D("FillAllTheBuffer end, free out bufferId count: " PUBLIC_LOG_ZU ", outBufQue_.Size: " PUBLIC_LOG_ZU,
                outBufPool_->EmptyBufferCount(), outBufQue_.Size());
    return true;
}

Status HdiCodecAdapter::ChangeState(OMX_STATETYPE state)
{
    MEDIA_LOG_I("change state from " PUBLIC_LOG_S " to " PUBLIC_LOG_S,
                OmxStateToString(targetState_).c_str(), OmxStateToString(state).c_str());
    if (targetState_ != state && curState_ != state) {
        auto ret = codecCmdExecutor_->SendCmd(OMX_CommandStateSet, state);
        FALSE_RETURN_V_MSG(ret == Status::OK, Status::ERROR_INVALID_OPERATION, "HdiSendCommand failed");
        targetState_ = state;
    }
    return Status::OK;
}

Status HdiCodecAdapter::WaitForState(OMX_STATETYPE state)
{
    MEDIA_LOG_D("WaitForState begin");
    auto ret = codecCmdExecutor_->WaitCmdResult(OMX_CommandStateSet, state);
    if (!ret) {
        MEDIA_LOG_E("Wait state failed");
        return Status::ERROR_WRONG_STATE;
    }
    curState_ = state;
    MEDIA_LOG_D("WaitForState end");
    return Status::OK;
}
} // namespace CodecAdapter
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif