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

#define HST_LOG_TAG "HdiCodecAdapter"

#include "hdi_codec_adapter.h"
#include <utility>
#include "codec_callback_if.h"
#include "codec_callback_type_stub.h"
#include "codec_component_if.h"
#include "codec_utils.h"
#include "foundation/log.h"
#include "hdf_base.h"
#include "hdi_codec_manager.h"
#include "plugin/common/plugin_attr_desc.h"

namespace {
using namespace OHOS::Media::Plugin;
using namespace CodecAdapter;
Status RegisterHdiAdapterPlugins(const std::shared_ptr<OHOS::Media::Plugin::Register>& reg)
{
    MEDIA_LOG_I("RegisterHdiAdapterPlugins Start");
    return HdiCodecManager::GetInstance().RegisterCodecPlugins(reg);
}

void UnRegisterHdiAdapterPlugins()
{
    MEDIA_LOG_I("UnRegisterHdiAdapterPlugins Start");
    HdiCodecManager::GetInstance().UnRegisterCodecPlugins();
}
} // namespace

PLUGIN_DEFINITION(CodecAdapter, LicenseType::APACHE_V2, RegisterHdiAdapterPlugins, UnRegisterHdiAdapterPlugins);

namespace OHOS {
namespace Media {
namespace Plugin {
namespace CodecAdapter {
// hdi adapter callback
int32_t HdiCodecAdapter::EventHandler(CodecCallbackType* self, OMX_EVENTTYPE event, EventInfo* info)
{
    MEDIA_LOG_I("EventHandler-callback Start, appData: " PUBLIC_LOG_D64 ", eEvent: " PUBLIC_LOG_D32
                ", nData1: " PUBLIC_LOG_U32 ", nData2: " PUBLIC_LOG_U32,
                info->appData, static_cast<int>(event), info->data1, info->data2);
    auto hdiAdapter = reinterpret_cast<HdiCodecAdapter*>(info->appData);
    hdiAdapter->codecCmdExecutor_->OnEvent(event, info);
    MEDIA_LOG_D("EventHandler-callback end");
    return HDF_SUCCESS;
}

int32_t HdiCodecAdapter::EmptyBufferDone(CodecCallbackType* self, int64_t appData, const OmxCodecBuffer* omxBuffer)
{
    MEDIA_LOG_DD("EmptyBufferDone begin, bufferId: " PUBLIC_LOG_U32, omxBuffer->bufferId);
    auto hdiAdapter = reinterpret_cast<HdiCodecAdapter*>(appData);
    hdiAdapter->inBufPool_->UseBufferDone(omxBuffer->bufferId);
    if (!hdiAdapter->isFlushing_) {
        hdiAdapter->HandleFrame();
    }
    MEDIA_LOG_DD("EmptyBufferDone-callback end, free in buffer count: " PUBLIC_LOG_U32,
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
    {
        OSAL::ScopedLock l(hdiAdapter->bufferMetaMutex_);
        auto iter = hdiAdapter->bufferMetaMap_.find(omxBuffer->pts);
        if (iter != hdiAdapter->bufferMetaMap_.end()) {
            outputBuffer->UpdateBufferMeta(*(iter->second));
            hdiAdapter->bufferMetaMap_.erase(omxBuffer->pts);
        } else {
            uint32_t frameNum = 0;
            outputBuffer->GetBufferMeta()->SetMeta(Tag::USER_FRAME_NUMBER, frameNum);
        }
    }
    if (hdiAdapter->isFlushing_) {
        MEDIA_LOG_DD("hdi adapter is flushing, ignore this data");
        outputBuffer = nullptr;
        return HDF_SUCCESS;
    }
    hdiAdapter->NotifyOutputBufferDone(outputBuffer);
    (void)hdiAdapter->FillAllTheOutBuffer(); // call FillThisBuffer() again
    MEDIA_LOG_DD("FillBufferDone-callback end, free out buffer count: " PUBLIC_LOG_U32,
                 hdiAdapter->outBufPool_->EmptyBufferCount());
    return HDF_SUCCESS;
}

HdiCodecAdapter::HdiCodecAdapter(std::string componentName, std::string pluginMime)
    : CodecPlugin(std::move(componentName)), pluginMime_(std::move(pluginMime))
{
    MEDIA_LOG_I("ctor called");
    shaAlloc_ = std::make_shared<ShareAllocator>(Plugin::ShareMemType::READ_WRITE_TYPE);
}

HdiCodecAdapter::~HdiCodecAdapter()
{
    MEDIA_LOG_I("dtor called");
    if (codecCallback_) {
        CodecCallbackTypeStubRelease(codecCallback_);
        codecCallback_ = nullptr;
    }
}

Status HdiCodecAdapter::Init()
{
    MEDIA_LOG_D("Init begin");
    auto firstDotPos = pluginName_.find_first_of('.'); // pluginName_: HdiCodecAdapter.OMX.rk.video_decoder.avc
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

    int32_t ret = HdiCodecManager::GetInstance().CreateComponent(&codecComp_, componentId_,
                                                                 const_cast<char*>(componentName_.c_str()),
                                                                 (int64_t)this, codecCallback_);
    FALSE_RETURN_V_MSG(codecComp_ != nullptr, Status::ERROR_NULL_POINTER,
                       "create component failed, retVal = " PUBLIC_LOG_D32, (int)ret);
    FALSE_RETURN_V_MSG(InitVersion() == Status::OK, Status::ERROR_INVALID_DATA, "Init compVersion failed!");
    FALSE_RETURN_V_MSG(InitPortIndex() == Status::OK, Status::ERROR_INVALID_DATA, "Init compVersion failed!");
    inCodecPort_ = std::make_shared<CodecPort>(codecComp_, inPortIndex_, verInfo_);
    outCodecPort_ = std::make_shared<CodecPort>(codecComp_, outPortIndex_, verInfo_);
    codecCmdExecutor_ = std::make_shared<CodecCmdExecutor>(codecComp_, inPortIndex_);
    portConfigured_ = false;
    MEDIA_LOG_D("Init end, component Id = " PUBLIC_LOG_D32, componentId_);
    return Status::OK;
}

Status HdiCodecAdapter::InitVersion()
{
    (void)memset_s(&verInfo_, sizeof(verInfo_), 0, sizeof(verInfo_));
    auto ret = codecComp_->GetComponentVersion(codecComp_, &verInfo_);
    FALSE_RETURN_V_MSG_E(ret == HDF_SUCCESS, Status::ERROR_INVALID_DATA,
                         "get component version failed, ret: " PUBLIC_LOG_D32, ret);
    return Status::OK;
}

Status HdiCodecAdapter::InitPortIndex()
{
    MEDIA_LOG_D("InitPortIndex begin");
    InitOmxParam(portParam_, verInfo_);
    auto ret = HdiGetParameter(codecComp_, OMX_IndexParamVideoInit, portParam_);
    FALSE_RETURN_V_MSG_E(ret == HDF_SUCCESS, Status::ERROR_INVALID_DATA,
                         "Get portParam failed, ret: " PUBLIC_LOG_D32, ret);
    inPortIndex_ = portParam_.nStartPortNumber;
    outPortIndex_ = portParam_.nStartPortNumber + 1;
    MEDIA_LOG_I("inPortIndex: " PUBLIC_LOG_U32 ", outPortIndex: " PUBLIC_LOG_U32, inPortIndex_, outPortIndex_);
    return Status::OK;
}

Status HdiCodecAdapter::Deinit()
{
    MEDIA_LOG_D("DeInit Enter");
    FALSE_RETURN_V_MSG_E(Reset() == Status::OK, Status::ERROR_INVALID_DATA, "Reset value failed");
    auto ret = HdiCodecManager::GetInstance().DestroyComponent(codecComp_, componentId_);
    FALSE_RETURN_V_MSG_E(ret == HDF_SUCCESS, Status::ERROR_INVALID_OPERATION,
        "HDI destroy component failed, ret = " PUBLIC_LOG_S, HdfStatus2String(ret).c_str());
    if (codecComp_) {
        CodecComponentTypeRelease(codecComp_);
        codecComp_ = nullptr;
    }
    if (codecCallback_) {
        CodecCallbackTypeStubRelease(codecCallback_);
        codecCallback_ = nullptr;
    }
    MEDIA_LOG_D("DeInit End;");
    return Status::OK;
}

Status HdiCodecAdapter::Prepare()
{
    MEDIA_LOG_D("Prepare Start");
    FALSE_RETURN_V_MSG_E(ChangeState(OMX_StateIdle) == Status::OK, Status::ERROR_WRONG_STATE,
                         "Change omx state to idle failed");
    outBufQue_->SetActive(true);
    inBufPool_ = std::make_shared<CodecBufferPool>(codecComp_, verInfo_, inPortIndex_, inBufferCnt_);
    outBufPool_ = std::make_shared<CodecBufferPool>(codecComp_, verInfo_, outPortIndex_, outBufferCnt_);
    OHOS::Media::BlockingQueue<std::shared_ptr<Buffer>> inBufQue("TempInBufferQue", inBufferCnt_);
    for (uint32_t i = 0; i < inBufferCnt_; i++) {
        auto buf = std::make_shared<Buffer>(BufferMetaType::VIDEO);
        if (inputMemoryType_ == MemoryType::VIRTUAL_ADDR && !buf->AllocMemory(shaAlloc_, inBufferSize_)) {
            MEDIA_LOG_E("alloc buffer " PUBLIC_LOG_U32 " fail, i: ", static_cast<uint32_t>(i));
        }
        inBufQue.Push(buf);
    }
    auto inputMemoryType = MemoryType::SHARE_MEMORY;
    if (inputMemoryType_ == MemoryType::SURFACE_BUFFER) {
        inputMemoryType = MemoryType::SURFACE_BUFFER;
    }
    bool isInput = true;
    inBufPool_->UseBuffers(inBufQue, inputMemoryType, isInput, inBufferSize_);
    outBufPool_->UseBuffers(*outBufQue_, outputMemoryType_, !isInput, outBufferSize_);
    FALSE_RETURN_V_MSG_E(WaitForState(OMX_StateIdle) == Status::OK, Status::ERROR_WRONG_STATE,
                         "Wait omx state to idle failed");
    MEDIA_LOG_D("prepare end");
    return Status::OK;
}

Status HdiCodecAdapter::Reset()
{
    MEDIA_LOG_D("Reset Start");
    FALSE_RETURN_V_MSG_E(ChangeState(OMX_StateIdle) == Status::OK,
        Status::ERROR_WRONG_STATE, "Change omx state to idle failed");
    FALSE_RETURN_V_MSG_E(WaitForState(OMX_StateIdle) == Status::OK,
        Status::ERROR_WRONG_STATE, "Wait omx state to idle failed");
    curState_ = OMX_StateIdle;
    FALSE_RETURN_V_MSG_E(FreeBuffers() == Status::OK, Status::ERROR_WRONG_STATE, "FreeBuffers failed");
    outBufQue_->SetActive(false);
    outBufQue_->Clear();
    inBufQue_.clear();
    {
        OSAL::ScopedLock l(bufferMetaMutex_);
        bufferMetaMap_.clear();
    }
    width_ = 0;
    height_ = 0;
    inBufferSize_ = 0;
    inBufferCnt_ = 0;
    outBufferSize_= 0;
    outBufferCnt_ = 0;
    inputMemoryType_ = MemoryType::VIRTUAL_ADDR;
    outputMemoryType_ = MemoryType::VIRTUAL_ADDR;
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
    outBufQue_->SetActive(true);
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
    outBufQue_->SetActive(false);
    MEDIA_LOG_D("Stop End");
    return Status::OK;
}

Status HdiCodecAdapter::Flush()
{
    MEDIA_LOG_D("Flush begin");
    isFlushing_ = true;
    {
        OSAL::ScopedLock l(lockInputBuffers_);
        inBufQue_.clear();
    }
    // -1: Refresh input and output ports
    auto ret = codecCmdExecutor_->SendCmd(OMX_CommandFlush, inPortIndex_);
    FALSE_RETURN_V_MSG_E(ret == Status::OK, Status::ERROR_UNKNOWN, "Flush inPort failed");
    auto err = codecCmdExecutor_->WaitCmdResult(OMX_CommandFlush, inPortIndex_);
    FALSE_RETURN_V_MSG_E(err == true, Status::ERROR_UNKNOWN, "Wait flush inPort failed");

    ret = codecCmdExecutor_->SendCmd(OMX_CommandFlush, outPortIndex_);
    FALSE_RETURN_V_MSG_E(ret == Status::OK, Status::ERROR_UNKNOWN, "Flush outPort failed");
    err = codecCmdExecutor_->WaitCmdResult(OMX_CommandFlush, outPortIndex_);
    FALSE_RETURN_V_MSG_E(err == true, Status::ERROR_UNKNOWN, "Wait flush outPort failed");
    isFlushing_ = false;
    MEDIA_LOG_D("Flush end");
    return Status::OK;
}

Status HdiCodecAdapter::GetParameter(Plugin::Tag tag, ValueType &value)
{
    MEDIA_LOG_D("GetParameter begin");
    switch (tag) {
        case Tag::REQUIRED_OUT_BUFFER_CNT:
            if (!outBufferCnt_) {
                return Status::ERROR_INVALID_DATA;
            }
            value = outBufferCnt_;
            break;
        case Tag::REQUIRED_OUT_BUFFER_SIZE:
            if (!outBufferSize_) {
                return Status::ERROR_INVALID_DATA;
            }
            value = outBufferSize_;
            break;
        default:
            MEDIA_LOG_W("ignore this tag: " PUBLIC_LOG_S, Tag2String(tag));
            break;
    }
    return Status::OK;
}

Status HdiCodecAdapter::SetParameter(Plugin::Tag tag, const ValueType &value)
{
    // When use hdi as codec plugin, must set width & height into hdi,
    // Hdi use these params to calc out buffer size & count then return to filter
    MEDIA_LOG_D("SetParameter begin");
    switch (tag) {
        case Tag::VIDEO_WIDTH:
            width_ = Plugin::AnyCast<uint32_t>(value);
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
        case Tag::MEDIA_BITRATE:
            bitRate_ = Plugin::AnyCast<int64_t>(value);
            break;
        case Tag::INPUT_MEMORY_TYPE:
            inputMemoryType_ = Plugin::AnyCast<MemoryType>(value);
            break;
        case Tag::OUTPUT_MEMORY_TYPE:
            outputMemoryType_ = Plugin::AnyCast<MemoryType>(value);
            break;
        default:
            MEDIA_LOG_W("Ignore this tag: " PUBLIC_LOG_S, Tag2String(tag));
            break;
    }
    if (width_ != 0 && height_ != 0 && pixelFormat_ != VideoPixelFormat::UNKNOWN && !portConfigured_ &&
        frameRate_ != 0) {
        FALSE_RETURN_V_MSG_E(ConfigOmx() == Status::OK, Status::ERROR_INVALID_OPERATION, "Configure omx failed");
    }
    MEDIA_LOG_D("SetParameter end");
    return Status::OK;
}

Status HdiCodecAdapter::ConfigOmx()
{
    MEDIA_LOG_D("ConfigOmx Start");
    Meta meta;
    meta.Set<Tag::MIME>(pluginMime_);
    meta.Set<Tag::VIDEO_WIDTH>(width_);
    meta.Set<Tag::VIDEO_HEIGHT>(height_);
    meta.Set<Tag::VIDEO_FRAME_RATE>(frameRate_);
    meta.Set<Tag::VIDEO_PIXEL_FORMAT>(pixelFormat_);
    meta.Set<Tag::MEDIA_BITRATE>(bitRate_);
    auto ret = inCodecPort_->Config(meta);
    FALSE_RETURN_V_MSG_E(ret == Status::OK, Status::ERROR_INVALID_OPERATION, "Configure inCodecPort failed");
    ret = outCodecPort_->Config(meta);
    FALSE_RETURN_V_MSG_E(ret == Status::OK, Status::ERROR_INVALID_OPERATION, "Configure outCodecPort failed");
    PortInfo portInfo;
    inCodecPort_->QueryParam(portInfo);
    inBufferCnt_ = portInfo.bufferCount;
    inBufferSize_ = portInfo.bufferSize;
    MEDIA_LOG_D("inBufCnt: " PUBLIC_LOG_D32 ", inBufSize: " PUBLIC_LOG_D32, inBufferCnt_, inBufferSize_);
    if (!portInfo.enabled) {
        codecCmdExecutor_->SendCmd(OMX_CommandPortEnable, inPortIndex_);
        (void) codecCmdExecutor_->WaitCmdResult(OMX_CommandPortEnable, inPortIndex_);
    }
    outCodecPort_->QueryParam(portInfo);
    outBufferCnt_ = portInfo.bufferCount;
    outBufferSize_ = portInfo.bufferSize;
    outBufQue_ = std::make_shared<BlockingQueue<std::shared_ptr<Buffer>>>("hdiAdapterOutQueue", outBufferCnt_);
    MEDIA_LOG_D("outBufCnt: " PUBLIC_LOG_D32 ", outBufSize: " PUBLIC_LOG_D32, outBufferCnt_, outBufferSize_);
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
    return shaAlloc_;
}

Status HdiCodecAdapter::QueueInputBuffer(const std::shared_ptr<Buffer>& inputBuffer, int32_t timeoutMs)
{
    MEDIA_LOG_DD("QueueInputBuffer Start");
    if (inputBuffer->IsEmpty() && !(inputBuffer->flag & BUFFER_FLAG_EOS)) {
        MEDIA_LOG_E("empty input buffer without eos flag");
        return Status::ERROR_INVALID_DATA;
    }
    {
        OSAL::ScopedLock l(lockInputBuffers_);
        inBufQue_.push_back(inputBuffer);
        MEDIA_LOG_DD("QueueInputBuffer end, inBufQue_.size: " PUBLIC_LOG_ZU, inBufQue_.size());
    }
    if (!isFlushing_) {
        HandleFrame();
    }
    return Status::OK;
}

// 循环从输入buffer队列中取出一个buffer，转换成 omxBuffer 后调用 HDI 的 EmptyThisBuffer() 进行解码
void HdiCodecAdapter::HandleFrame()
{
    MEDIA_LOG_DD("handle frame begin");
    while (inBufPool_->EmptyBufferCount()) {
        std::shared_ptr<Buffer> inputBuffer = nullptr;
        std::shared_ptr<CodecBuffer> codecBuffer = nullptr;
        {
            OSAL::ScopedLock l(lockInputBuffers_);
            if (inBufQue_.empty()) {
                return;
            }
            inputBuffer = inBufQue_.front();
            codecBuffer = inBufPool_->GetBuffer();
            FALSE_RETURN(codecBuffer != nullptr);
            inBufQue_.pop_front();
        }
        if (inputMemoryType_ == MemoryType::VIRTUAL_ADDR) {
            FALSE_RETURN_MSG(codecBuffer->Copy(inputBuffer) == Status::OK, "Copy inBuffer into codecBuffer fail");
        } else {
            FALSE_RETURN_MSG(codecBuffer->Rebind(inputBuffer) == Status::OK, "Rebind inBuffer into codecBuffer fail");
        }
        {
            OSAL::ScopedLock l(bufferMetaMutex_);
            bufferMetaMap_.emplace(inputBuffer->pts, inputBuffer->GetBufferMeta()->Clone());
        }
        auto ret = HdiEmptyThisBuffer(codecComp_, codecBuffer->GetOmxBuffer().get());
        FALSE_LOG_MSG(ret == HDF_SUCCESS, "call EmptyThisBuffer() error, ret: " PUBLIC_LOG_S,
                      HdfStatus2String(ret).c_str());
        NotifyInputBufferDone(inputBuffer);
    }
    MEDIA_LOG_DD("handle frame end");
}

Status HdiCodecAdapter::QueueOutputBuffer(const std::shared_ptr<Buffer>& outputBuffer, int32_t timeoutMs)
{
    MEDIA_LOG_DD("QueueOutputBuffer start, outBufQue size: " PUBLIC_LOG_ZU, outBufQue_->Size());
    outBufQue_->Push(outputBuffer);
    if (curState_ == OMX_StateExecuting && !isFlushing_) {
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
    OSAL::ScopedLock l(fillAllTheOutBufferMutex_);
    MEDIA_LOG_DD("FillAllTheBuffer begin");
    if (isFirstCall_) {
        MEDIA_LOG_I("isFirstCall: " PUBLIC_LOG_D32, isFirstCall_);
        isFirstCall_ = false;
        for (uint32_t i = 0; i < outBufferCnt_; ++i) {
            auto codecBuffer = outBufPool_->GetBuffer();
            FALSE_RETURN_V_MSG_E(codecBuffer != nullptr, false, "Get codecBuffer failed");
            auto ret = HdiFillThisBuffer(codecComp_, codecBuffer->GetOmxBuffer().get());
            FALSE_RETURN_V_MSG_E(ret == HDF_SUCCESS, false, "Call FillThisBuffer() error, ret: " PUBLIC_LOG_S
                ", isFirstCall: " PUBLIC_LOG_D32, HdfStatus2String(ret).c_str(), isFirstCall_);
        }
    } else {
        while (!outBufQue_->Empty()) {
            if (!outBufPool_->EmptyBufferCount()) {
                MEDIA_LOG_D("outBufQue_ have data, but freeBufferId is empty");
                return false;
            }
            auto codecBuffer = outBufPool_->GetBuffer();
            FALSE_RETURN_V(codecBuffer != nullptr, false);
            auto outputBuffer = outBufQue_->Pop(1);
            if (outputBuffer == nullptr) {
                MEDIA_LOG_E("output buffer is nullptr");
                outBufPool_->UseBufferDone(codecBuffer->GetBufferId());
                return false;
            }
            codecBuffer->Rebind(outputBuffer); // 这里outBuf需要保存到codecBuffer里面，方便往下一节点传数据
            auto ret = HdiFillThisBuffer(codecComp_, codecBuffer->GetOmxBuffer().get());
            if (ret != HDF_SUCCESS) {
                codecBuffer->Unbind(outputBuffer, codecBuffer->GetOmxBuffer().get());
                outBufPool_->UseBufferDone(codecBuffer->GetBufferId());
                MEDIA_LOG_E("Call FillThisBuffer() error, ret: " PUBLIC_LOG_S ", isFirstCall: " PUBLIC_LOG_D32,
                    HdfStatus2String(ret).c_str(), isFirstCall_);
            }
        }
    }
    MEDIA_LOG_DD("FillAllTheBuffer end, free out bufferId count: " PUBLIC_LOG_U32 ", outBufQue_->Size: " PUBLIC_LOG_ZU,
                 outBufPool_->EmptyBufferCount(), outBufQue_->Size());
    return true;
}

Status HdiCodecAdapter::FreeBuffers()
{
    MEDIA_LOG_D("FreeBuffers begin");
    FALSE_RETURN_V_MSG_E(ChangeState(OMX_StateLoaded) == Status::OK,
                         Status::ERROR_WRONG_STATE, "Change omx state to loaded failed");
    auto val = inBufPool_->FreeBuffers();
    FALSE_RETURN_V_MSG_E(val == Status::OK, val, "free buffers failed");
    val = outBufPool_->FreeBuffers();
    FALSE_RETURN_V_MSG_E(val == Status::OK, val, "free buffers failed");
    FALSE_RETURN_V_MSG_E(WaitForState(OMX_StateLoaded) == Status::OK,
                         Status::ERROR_WRONG_STATE, "Wait omx state to loaded failed");
    curState_ = OMX_StateLoaded;
    return Status::OK;
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