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

#define HST_LOG_TAG "HdiAdapter"

#include "hdi_adapter.h"
#include "codec_callback_type_stub.h"
#include "codec_omx_ext.h"
#include "common/surface_memory.h"
#include "dump_buffer.h"
#include "display_type.h"
#include "foundation/log.h"
#include "hdi_adapter_param_map.h"
#include "hdi_utils.h"
#include "OMX_Audio.h"
#include "OMX_Video.h"
#include "osal/utils/util.h"
#include "pipeline/core/plugin_attr_desc.h"
#include "plugin/common/plugin_caps_builder.h"

namespace OHOS {
namespace Media {
namespace Plugin {
CodecComponentManager* g_compManager = nullptr;
//constexpr size_t DEFAULT_IN_BUFFER_QUEUE_SIZE = 8;
constexpr size_t DEFAULT_OUT_BUFFER_QUEUE_SIZE = 21;

bool UpdateInCaps(const CodecCompCapability& cap, CodecPluginDef& definition)
{
    bool flag = true;
    CapabilityBuilder incapBuilder;
    switch (cap.role) {
        case MEDIA_ROLETYPE_VIDEO_AVC:
            incapBuilder.SetMime(OHOS::Media::MEDIA_MIME_VIDEO_H264);
            break;
        default:
            incapBuilder.SetMime("video/unknown");
            flag = false;
            break;
    }
//    incapBuilder.SetBitRateRange(cap.bitRate.min, cap.bitRate.max);
    if (flag) {
        definition.inCaps.push_back(incapBuilder.Build());
    }
    return flag;
}

void UpdateOutCaps(const CodecCompCapability& cap, CodecPluginDef& definition)
{
    CapabilityBuilder outcapBuilder;
    outcapBuilder.SetMime(OHOS::Media::MEDIA_MIME_VIDEO_RAW);
    int32_t index = 0;
    std::vector<Plugin::VideoPixelFormat> formats;
    for (int i = 0; cap.port.video.supportPixFmts[i] != 0; ++i) {
        MEDIA_LOG_D("i = " PUBLIC_LOG_D32 ", support pixfmts: " PUBLIC_LOG_D32,
                    i, (int32_t)cap.port.video.supportPixFmts[i]);
    }
    if (index) {
        outcapBuilder.SetVideoPixelFormatList(formats);
    }
    definition.outCaps.push_back(outcapBuilder.Build());
}

bool TranslateVideoDecoderCap(const CodecCompCapability& cap, CodecPluginDef& def)
{
//    FALSE_RETURN_V_MSG_W(UpdateInCaps(cap, def), false, "update InCaps failed");
    (void) UpdateInCaps(cap, def);
    UpdateOutCaps(cap, def);
    return true;
}

bool TranslateCapability(const CodecCompCapability& cap, CodecPluginDef& def)
{
    switch (cap.type) {
        case ::CodecType::AUDIO_DECODER:
            break;
        case ::CodecType::VIDEO_DECODER: {
            return TranslateVideoDecoderCap(cap, def);
        }
        case ::CodecType::AUDIO_ENCODER: {
            break;
        }
        case ::CodecType::VIDEO_ENCODER: {
            break;
        }
        default:
            break;
    }
    return false;
}

bool TranslateCapToPluginDef(const CodecCompCapability& capability,
                             CodecPluginDef& def, const std::string& packageName)
{
    if (!Translates(capability.type, def.codecType)) {
        MEDIA_LOG_E("codec type of plugin " PUBLIC_LOG_S "." PUBLIC_LOG_S " mismatched",
                    packageName.c_str(), capability.compName);
        return false;
    }
    if (!TranslateCapability(capability, def)) {
        MEDIA_LOG_W("codec capability of plugin " PUBLIC_LOG_S "." PUBLIC_LOG_S " translate failed",
                    packageName.c_str(), capability.compName);
        return false;
    }
    def.name = packageName + "." + capability.compName;
    def.creator = [] (const std::string& name) -> std::shared_ptr<CodecPlugin> {
        return std::make_shared<HdiAdapter>(name);
    };
    return true;
}

// register
Status RegisterOneCodecPackage(const std::shared_ptr<OHOS::Media::Plugin::Register>& reg,
                               const std::string& packageName)
{
    g_compManager = GetCodecComponentManager();
    if (g_compManager == nullptr) {
        MEDIA_LOG_E("codec package " PUBLIC_LOG_S " has no valid component manager", packageName.c_str());
        return Status::ERROR_INVALID_DATA;
    }
    std::dynamic_pointer_cast<OHOS::Media::Plugin::PackageRegister>(reg)->AddPackage(
            {PLUGIN_INTERFACE_VERSION, packageName, OHOS::Media::Plugin::LicenseType::APACHE_V2});

    int32_t count = g_compManager->GetComponentNum();
    MEDIA_LOG_D("component number is: " PUBLIC_LOG_D32, count);
    CodecCompCapability capList[count];
    g_compManager->GetComponentCapabilityList(capList, count);
    for (int32_t i = 0; i < count; i++) {
        if (capList[i].type != ::CodecType::VIDEO_DECODER) {
            continue;
        }
        CodecPluginDef definition;
        if (TranslateCapToPluginDef(capList[i], definition, packageName)) {
            definition.rank = 100;
            if (reg->AddPlugin(definition) != Status::OK) {
                MEDIA_LOG_E("add plugin " PUBLIC_LOG_S " failed", definition.name.c_str());
            }
        }
    }
    MEDIA_LOG_D("registering video HDI decoders done");
    return Status::OK;
}

void UnRegisterOneCodecPackage(const std::string& packageName)
{
    CodecComponentManagerRelease();
    g_compManager = nullptr;
}

// hdi adapter callback
int32_t HdiAdapter::EventHandler(CodecCallbackType* self, OMX_EVENTTYPE event, EventInfo* info)
{
    auto hdiAdapter = reinterpret_cast<HdiAdapter*>(info->appData);
    MEDIA_LOG_I("appData: " PUBLIC_LOG_D64 ", eEvent: " PUBLIC_LOG_D32
                ", nData1: " PUBLIC_LOG_U32 ", nData2: " PUBLIC_LOG_U32,
                info->appData, static_cast<int>(event), info->data1, info->data2);
    switch (event) {
        case OMX_EventCmdComplete: {
            hdiAdapter->HandelEventCmdComplete(info->data1, info->data2);
            break;
        }
        default:
            break;
    }
    MEDIA_LOG_D("EventHandler-callback end");
    return HDF_SUCCESS;
}

int32_t HdiAdapter::EmptyBufferDone(CodecCallbackType* self, int64_t appData, const OmxCodecBuffer* buffer)
{
    MEDIA_LOG_D("EmptyBufferDone-callback start");
    auto hdiAdapter = reinterpret_cast<HdiAdapter*>(appData);
    hdiAdapter->freeInBufferId_.Push(buffer->bufferId);
    hdiAdapter->HandleFrame();
    MEDIA_LOG_D("EmptyBufferDone-callback end, bufferId: " PUBLIC_LOG_U32, buffer->bufferId);
    return HDF_SUCCESS;
}

int32_t HdiAdapter::FillBufferDone(CodecCallbackType* self, int64_t appData, const OmxCodecBuffer* omxBuffer)
{
    MEDIA_LOG_D("FillBufferDone-callback begin, bufferId: " PUBLIC_LOG_U32 ", flag: " PUBLIC_LOG_U32 ", pts: " PUBLIC_LOG_D64,
                omxBuffer->bufferId, omxBuffer->flag, omxBuffer->pts);
    // 如果空bufferId到达21，则表示没有buffer下去，则不能将 inBuffer fill下去，
    // 那么，queueInputBuffer 和 QueueOutputBuffer 都需要等待
    // 为啥 空 bufferId 数量会到达 21 ？
    auto hdiAdapter = reinterpret_cast<HdiAdapter*>(appData);
    auto iter = hdiAdapter->bufferInfoMap_.find(omxBuffer->bufferId);
    if ((iter == hdiAdapter->bufferInfoMap_.end()) || (iter->second == nullptr)) {
        MEDIA_LOG_D("iter == hdiAdapter->omxBuffers_.end() || iter->second == nullptr");
        return HDF_ERR_INVALID_PARAM;
    }
    auto bufferInfo = iter->second;
    auto outputBuffer = bufferInfo->outputBuffer;
    outputBuffer->flag = Translate2PluginFlagSet(omxBuffer->flag);
    outputBuffer->pts = omxBuffer->pts;
    hdiAdapter->NotifyOutputBufferDone(outputBuffer);
    bufferInfo->outputBuffer = nullptr; // Need: to release output buffer, Decrease the reference count
    hdiAdapter->freeOutBufferId_.Push(omxBuffer->bufferId);
    hdiAdapter->SendEmptyBufToHdi();
    MEDIA_LOG_D("FillBufferDone-callback end, free out bufferId count: " PUBLIC_LOG_ZU, hdiAdapter->freeOutBufferId_.Size());
    return HDF_SUCCESS;
}

// 从空 out buffer 里面拿，然后与 omxBuffer 对应，调用 FillThisBuffer 循环往下送
void HdiAdapter::SendEmptyBufToHdi()
{
    isFirstCall_ = false;
    (void)FillAllTheBuffer();
    MEDIA_LOG_D("SendEmptyBufToHdi end");
}

void HdiAdapter::TransOutputBufToOmxBuf(const std::shared_ptr<Plugin::Buffer>& outputBuffer,
                                        std::shared_ptr<OmxCodecBuffer>& omxBuffer)
{
    omxBuffer->pts = 0;
    omxBuffer->flag = 0;
    auto outMem = std::static_pointer_cast<Plugin::SurfaceMemory>(outputBuffer->GetMemory());
    FALSE_RETURN(outMem != nullptr);
    auto surfaceBuf = outMem->GetSurfaceBuffer();
    FALSE_RETURN(surfaceBuf != nullptr);

    BufferHandle* bufferHandle = surfaceBuf->GetBufferHandle();
    omxBuffer->bufferLen = sizeof(BufferHandle) +
                           sizeof(int32_t) * (bufferHandle->reserveFds + bufferHandle->reserveInts);
    omxBuffer->buffer = (uint8_t*)bufferHandle;
    MEDIA_LOG_D("TransOutputBufToOmx end, omxBufferId: " PUBLIC_LOG_U32, omxBuffer->bufferId);
}

// hdi adapter impl
HdiAdapter::HdiAdapter(std::string name)
    : CodecPlugin(std::forward<std::string>(name)),
      freeInBufferId_("hdiFreeInBufferId", 4),
      freeOutBufferId_("hdiFreeOutBufferId", 21),
      outBufQue_("hdiAdapterOutQueue", DEFAULT_OUT_BUFFER_QUEUE_SIZE)
{
    MEDIA_LOG_D("codec adapter ctor");
}

HdiAdapter::~HdiAdapter()
{
    if (codecCallback_) {
        CodecCallbackTypeStubRelease(codecCallback_);
        codecCallback_ = nullptr;
    }
}

Plugin::Status HdiAdapter::Init()
{
    MEDIA_LOG_D("codec adapter init begin");
    auto firstDotPos = pluginName_.find_first_of('.');
    MEDIA_LOG_D("pluginName_: " PUBLIC_LOG_S, pluginName_.c_str());
    if (firstDotPos == std::string::npos) {
        MEDIA_LOG_E("create codec handle error with plugin name " PUBLIC_LOG_S ", which is wrong format",
                    pluginName_.c_str());
        return Status::ERROR_UNSUPPORTED_FORMAT;
    }
    std::string compName = pluginName_.substr(firstDotPos + 1); // ComponentCapability.compName

    codecCallback_ = CodecCallbackTypeStubGetInstance();
    FALSE_RETURN_V_MSG(codecCallback_ != nullptr, Status::ERROR_NULL_POINTER, "create callback_ failed");
    codecCallback_->EventHandler = &HdiAdapter::EventHandler;
    codecCallback_->EmptyBufferDone = &HdiAdapter::EmptyBufferDone;
    codecCallback_->FillBufferDone = &HdiAdapter::FillBufferDone;

    int32_t ret = g_compManager->CreateComponent(&codecComp_, &componentId_, const_cast<char*>(compName.c_str()),
                                                 (int64_t)this, codecCallback_);
    FALSE_RETURN_V_MSG(codecComp_ != nullptr, Status::ERROR_NULL_POINTER,
                       "create component failed, retVal = " PUBLIC_LOG_D32, (int)ret);

/*    MEDIA_LOG_D("start codecComp_->GetState()：");
    ret = codecComp_->GetState(codecComp_, &curState_);
    MEDIA_LOG_D("curState_: " PUBLIC_LOG_D32, curState_);
    if (ret != HDF_SUCCESS || curState_ != OMX_StateLoaded) {
        MEDIA_LOG_E("get component status error: curStatus is " PUBLIC_LOG_D32, curState_);
        return Status::ERROR_UNKNOWN;
    }*/
    // 获取组件版本号
    (void)memset_s(&verInfo_, sizeof(verInfo_), 0, sizeof(verInfo_));
    ret = codecComp_->GetComponentVersion(codecComp_, &verInfo_);
    FALSE_RETURN_V_MSG_E(ret == HDF_SUCCESS, Status::ERROR_INVALID_DATA,
                         "get component version failed, ret: " PUBLIC_LOG_D32, ret);
    outBufQue_.SetActive(true);
    MEDIA_LOG_D("codec adapter init end");
    return Status::OK;
}

Plugin::Status HdiAdapter::Deinit()
{
    FALSE_RETURN_V_MSG(g_compManager != nullptr, Status::ERROR_INVALID_PARAMETER, "g_compManager is nullptr");
    FALSE_RETURN_V_MSG(codecComp_ != nullptr, Status::ERROR_INVALID_PARAMETER, "codecComponent is nullptr");
//    codecComp_->GetState(codecComp_, &curState_);
//    if (curState_ > OMX_StateLoaded) {
//        // wait state change to OMX_StateLoaded
//        MEDIA_LOG_D("DeInit, state is not OMX_StateLoaded");
//        return Status::ERROR_WRONG_STATE;
//    }
    auto ret = g_compManager->DestroyComponent(componentId_);
    FALSE_RETURN_V_MSG_E(ret != HDF_SUCCESS, Status::ERROR_INVALID_OPERATION, "HDI destroy component failed");
    codecComp_ = nullptr;
    curState_ = OMX_StateInvalid;
    outBufQue_.SetActive(false);
    outBufQue_.Clear();
    CodecCallbackTypeStubRelease(codecCallback_);
    codecCallback_ = nullptr;
    MEDIA_LOG_D("DeInit end");
    return Status::OK;
}

Plugin::Status HdiAdapter::Prepare()
{
    int32_t ret = HDF_SUCCESS;
    outBufQue_.SetActive(true);
    UseOmxBuffers(); // 申请 omx buffer
    MEDIA_LOG_D("prepare end");
    return TranslateRets(ret);
}

Plugin::Status HdiAdapter::Reset()
{
//    inBufQue_.Clear();
//    outBufQue_.Clear();
//    MEDIA_LOG_D("Reset end");
    return Status::OK;
}

Plugin::Status HdiAdapter::Start()
{
    MEDIA_LOG_D("start begin");
//    FALSE_RETURN_V_MSG_E(ChangeState(OMX_StateExecuting) == Status::OK, Status::ERROR_WRONG_STATE, "ChangeState failed");
//    FALSE_RETURN_V_MSG_E(WaitForState(OMX_StateExecuting) == Status::OK, Status::ERROR_WRONG_STATE, "Wait failed");
    auto err = codecComp_->SendCommand(codecComp_, OMX_CommandStateSet, OMX_StateExecuting, NULL, 0);
    if (err != HDF_SUCCESS) {
        MEDIA_LOG_D("failed to SendCommand with OMX_CommandStateSet:OMX_StateExecuting");
        return Status::ERROR_UNKNOWN;
    }
    OSAL::SleepFor(30);
    enum OMX_STATETYPE state = OMX_StateInvalid;
    err = codecComp_->GetState(codecComp_, &state);
    while (state != OMX_StateExecuting) {
        err = codecComp_->GetState(codecComp_, &state);
        OSAL::SleepFor(30);
    }
    MEDIA_LOG_D("change state to exe success, state: " PUBLIC_LOG_D32, static_cast<int32_t>(state));
    if (!FillAllTheBuffer()) {
        MEDIA_LOG_E("Fill all buffer error");
        return Status::ERROR_UNKNOWN;
    }
    outBufQue_.SetActive(true);
    curState_ = OMX_StateExecuting;
    MEDIA_LOG_D("start end");
    return Status::OK;
}

Plugin::Status HdiAdapter::Stop()
{
    MEDIA_LOG_D("Stop begin");
    outBufQue_.SetActive(false);
/*    if (curState_ == OMX_StateExecuting) {
        FALSE_RETURN_V_MSG_E(ChangeState(OMX_StateIdle) == Status::OK, Status::ERROR_WRONG_STATE, "ChangeState failed");
        FALSE_RETURN_V_MSG_E(WaitForState(OMX_StateIdle) == Status::OK, Status::ERROR_WRONG_STATE, "Wait failed");
    }
    if (curState_ == OMX_StateIdle) {
        FALSE_RETURN_V_MSG_E(ChangeState(OMX_StateLoaded) == Status::OK, Status::ERROR_WRONG_STATE, "ChangeState failed");
        FALSE_RETURN_V_MSG_E(WaitForState(OMX_StateIdle) == Status::OK, Status::ERROR_WRONG_STATE, "Wait failed");
    }*/
    auto err = codecComp_->SendCommand(codecComp_, OMX_CommandStateSet, OMX_StateIdle, NULL, 0);
    if (err != HDF_SUCCESS) {
        MEDIA_LOG_D("failed to SendCommand with OMX_StateIdle");
        return Status::ERROR_UNKNOWN;
    }
    OSAL::SleepFor(30);
    enum OMX_STATETYPE state = OMX_StateInvalid;
    err = codecComp_->GetState(codecComp_, &state);
    while (state != OMX_StateIdle) {
        err = codecComp_->GetState(codecComp_, &state);
        OSAL::SleepFor(15);
    }
//    err = codecComp_->SendCommand(codecComp_, OMX_CommandStateSet, OMX_StateLoaded, NULL, 0);
    MEDIA_LOG_D("change state to idle success, state: " PUBLIC_LOG_D32, static_cast<int32_t>(state));
    MEDIA_LOG_D("Stop end");
    return Status::OK;
}

Plugin::Status HdiAdapter::GetParameter(Plugin::Tag tag, ValueType& value)
{
    MEDIA_LOG_D("GetParameter begin");
    switch (tag) {
        case Tag::REQUIRED_OUT_BUFFER_CNT:
            ConfigOmx();
            GetBufferInfoOnPort(PortIndex::PORT_INDEX_INPUT);
            GetBufferInfoOnPort(PortIndex::PORT_INDEX_OUTPUT);
            value = outBufferCnt_;
            break;
        case Tag::VIDEO_HDI_BUFFER_SIZE:
            value = outBufferSize_;
            break;
        default :
            MEDIA_LOG_W("ignore this tag: " PUBLIC_LOG_S, Pipeline::Tag2String(tag));
            break;
    }
    return Status::OK;
}

Plugin::Status HdiAdapter::SetParameter(Plugin::Tag tag, const ValueType& value)
{
    MEDIA_LOG_D("SetParameter begin");
    switch (tag) {
        case Tag::VIDEO_WIDTH :
            width_ = Plugin::AnyCast<uint32_t>(value);
            stride_ = AlignUp(width_, 16);
            break;
        case Tag::VIDEO_HEIGHT :
            height_ = Plugin::AnyCast<uint32_t>(value);
            break;
        case Tag::VIDEO_PIXEL_FORMAT :
            pixelFormat_ = Plugin::AnyCast<VideoPixelFormat>(value);
            MEDIA_LOG_D("pixelFormat: " PUBLIC_LOG_U32, static_cast<uint32_t>(pixelFormat_));
            break;
        default :
            MEDIA_LOG_W("ignore this tag: " PUBLIC_LOG_S, Pipeline::Tag2String(tag));
            break;
    }
    MEDIA_LOG_D("SetParameter end");
    return Status::OK;
}

std::shared_ptr<Plugin::Allocator> HdiAdapter::GetAllocator()
{
    MEDIA_LOG_D("GetAllocator begin");
    return nullptr;
}

Plugin::Status HdiAdapter::SetCallback(Callback* cb)
{
    MEDIA_LOG_D("SetCallback begin");
    callback_ = cb;
    return Status::OK;
}

Status HdiAdapter::QueueInputBuffer(const std::shared_ptr<Plugin::Buffer>& inputBuffer, int32_t timeoutMs)
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

void HdiAdapter::HandleFrame()
{
    MEDIA_LOG_D("handle frame start");
    while (!freeInBufferId_.Empty()) {
        std::shared_ptr<Buffer> inputBuffer = nullptr;
        uint32_t inBufferId = 0;
        {
            OSAL::ScopedLock l(lockInputBuffers_);
            if (inBufQue_.empty()) {
                return;
            }
            inputBuffer = inBufQue_.front();
            inBufferId = freeInBufferId_.Pop(1);
            if (inBufferId == 0) {
                return;
            }
            inBufQue_.pop_front();
        }
        auto iter = bufferInfoMap_.find(inBufferId);
        auto bufferInfo = iter->second;

        auto err = TransInputBuffer2OmxBuffer(inputBuffer, bufferInfo);
        FALSE_RETURN_MSG(err == Status::OK, "TransInputBuffer2OmxBuffer() fail");
        if (codecComp_ && codecComp_->EmptyThisBuffer) {
            auto ret = codecComp_->EmptyThisBuffer(codecComp_, bufferInfo->omxBuffer.get());
            FALSE_LOG_MSG(ret == HDF_SUCCESS, "call EmptyThisBuffer() error, bufferId: " PUBLIC_LOG_D32, inBufferId);
        }
        NotifyInputBufferDone(inputBuffer);
    }
    MEDIA_LOG_D("handle frame end");
}

Status HdiAdapter::TransInputBuffer2OmxBuffer(const std::shared_ptr<Plugin::Buffer>& pluginBuffer,
                                            std::shared_ptr<BufferInfo>& bufferInfo)
{
    bufferInfo->omxBuffer->flag = Translate2omxFlagSet(pluginBuffer->flag);
    bufferInfo->omxBuffer->pts = pluginBuffer->pts;
    MEDIA_LOG_D("plugin flag: " PUBLIC_LOG_U32 ", pts: " PUBLIC_LOG_D64,
                bufferInfo->omxBuffer->flag, bufferInfo->omxBuffer->pts);
    if (pluginBuffer->flag == 1) {
        MEDIA_LOG_D("EOS flag receive, return");
        return Status::ERROR_INVALID_DATA;
    }
    auto mem = pluginBuffer->GetMemory();
    if (mem == nullptr) {
        MEDIA_LOG_D("pluginBuffer->GetMemory() return nullptr");
        return Status::ERROR_INVALID_DATA;
    }
    const uint8_t* memAddr = mem->GetReadOnlyData();
    if (memAddr == nullptr) {
        MEDIA_LOG_D("mem->GetReadOnlyData() return nullptr");
        return Status::ERROR_INVALID_DATA;
    }
    size_t bufLen = mem->GetSize();

    (void)bufferInfo->avSharedPtr->Write(memAddr, bufLen, 0);
    bufferInfo->omxBuffer->offset = 0;
    bufferInfo->omxBuffer->filledLen = bufLen;
    MEDIA_LOG_D("TransBuffer2OmxBuffer end, bufferId: " PUBLIC_LOG_U32, bufferInfo->omxBuffer->bufferId);
    return Status::OK;
}

bool HdiAdapter::FillAllTheBuffer() // freeOutBufferId_ is blocking que
{
    MEDIA_LOG_D("FillAllTheBuffer begin");
    if (isFirstCall_) {
        uint32_t outBufQueSize = outBufferCnt_;
        for (uint32_t i = 0; i < outBufQueSize; ++i) { // 这里判断条件应该是 freeBufferId 的个数
            int outBufferID = static_cast<int>(freeOutBufferId_.Pop());
            auto iter = bufferInfoMap_.find(outBufferID);
            auto bufferInfo = iter->second;
            auto omxBuffer = bufferInfo->omxBuffer;
            int32_t ret = HDF_SUCCESS;
            if (codecComp_ && codecComp_->FillThisBuffer) {
                ret = codecComp_->FillThisBuffer(codecComp_, omxBuffer.get());
                FALSE_RETURN_V_MSG_E(ret == HDF_SUCCESS, false, "call FillThisBuffer() error, bufferId: " PUBLIC_LOG_U32,
                                     omxBuffer->bufferId);
            }
        }
    } else {
        while (!outBufQue_.Empty()) {
            if (freeOutBufferId_.Empty()) {
                MEDIA_LOG_D("outBufQue_ have data, but freeOutBufferId_ is empty");
                return false;
            }
            int outBufferID = static_cast<int>(freeOutBufferId_.Pop(1));
            FALSE_RETURN_V(outBufferID != 0, false);
            auto iter = bufferInfoMap_.find(outBufferID);
            auto bufferInfo = iter->second;
            auto omxBuffer = bufferInfo->omxBuffer;

            auto outputBuffer = outBufQue_.Pop(1);
            if (outputBuffer == nullptr) {
                freeOutBufferId_.Push(outBufferID);
                MEDIA_LOG_E("output buffer is nullptr");
                return false;
            }
            TransOutputBufToOmxBuf(outputBuffer, omxBuffer);
            bufferInfo->outputBuffer = outputBuffer;
            int32_t ret = HDF_SUCCESS;
            if (codecComp_ && codecComp_->FillThisBuffer) {
                ret = codecComp_->FillThisBuffer(codecComp_, omxBuffer.get());
                FALSE_RETURN_V_MSG_E(ret == HDF_SUCCESS, false, "call FillThisBuffer() error, bufferId: " PUBLIC_LOG_U32,
                                     omxBuffer->bufferId);
            }
        }
    }
    MEDIA_LOG_D("FillAllTheBuffer end, free out bufferId count: " PUBLIC_LOG_ZU ", outBufQue_.Size: " PUBLIC_LOG_ZU,
                freeOutBufferId_.Size(), outBufQue_.Size());
    return true;
}

Status HdiAdapter::QueueOutputBuffer(const std::shared_ptr<Plugin::Buffer>& outputBuffers, int32_t timeoutMs)
{
    outBufQue_.Push(outputBuffers);
    if (curState_ == OMX_StateExecuting) {
        isFirstCall_ = false;
        FillAllTheBuffer();
    }
    MEDIA_LOG_D("QueueOutputBuffer end");
    return Status::OK;
}

Status HdiAdapter::Flush()
{
    MEDIA_LOG_D("Flush begin");
    isFlush_ = true;
    {
        OSAL::ScopedLock l(lockInputBuffers_);
        inBufQue_.clear();
    }
    outBufQue_.SetActive(true);
    outBufQue_.Clear();
/*    int32_t ret = HDF_ERR_INVALID_PARAM;
    if (codecComp_ && codecComp_->SendCommand) {
        ret = codecComp_->SendCommand(codecComp_, OMX_CommandFlush, (uint32_t)PortIndex::PORT_INDEX_INPUT, nullptr, 0);
        FALSE_RETURN_V_MSG_E(ret == HDF_SUCCESS, Status::ERROR_INVALID_OPERATION, "flush hdi failed");
    }
    OSAL::SleepFor(30);
    enum OMX_STATETYPE state = OMX_StateInvalid;
    ret = codecComp_->GetState(codecComp_, &state);
    while (state != OMX_StateExecuting) {
        ret = codecComp_->GetState(codecComp_, &state);
        OSAL::SleepFor(30);
    }*/
    MEDIA_LOG_D("flush end");
    return Status::OK;
}

Status HdiAdapter::SetDataCallback(DataCallback* dataCallback)
{
    MEDIA_LOG_D("SetDataCallback begin");
    dataCallback_ = dataCallback;
    return Status::OK;
}

void HdiAdapter::NotifyInputBufferDone(const std::shared_ptr<Buffer>& input)
{
    if (dataCallback_ != nullptr) {
        dataCallback_->OnInputBufferDone(input);
    }
    MEDIA_LOG_DD("NotifyInputBufferDone end");
}

void HdiAdapter::NotifyOutputBufferDone(const std::shared_ptr<Buffer>& output)
{
    if (dataCallback_ != nullptr) {
        dataCallback_->OnOutputBufferDone(output);
        MEDIA_LOG_D("NotifyOutputBufferDone end");
    }
}

void HdiAdapter::ConfigOmx()
{
    MEDIA_LOG_D("ConfigOmx begin");
    ConfigOmxPortDefine();

    // 设置输入数据为H264编码
    OMX_VIDEO_PARAM_PORTFORMATTYPE videoFormat;
    InitParam(videoFormat);
    videoFormat.nPortIndex = (uint32_t)PortIndex::PORT_INDEX_INPUT;
    auto ret = codecComp_->GetParameter(codecComp_, OMX_IndexParamVideoPortFormat,
                                        (int8_t *)&videoFormat, sizeof(videoFormat));
    if (ret != HDF_SUCCESS) {
        MEDIA_LOG_E("GetParameter OMX_IndexParamVideoPortFormat failed, ret = " PUBLIC_LOG_D32, (int)ret);
        return;
    }
    MEDIA_LOG_I("set Format PORT_INDEX_INPUT eCompressionFormat = " PUBLIC_LOG_D32 ", eColorFormat = " PUBLIC_LOG_D32,
                videoFormat.eCompressionFormat, videoFormat.eColorFormat);
    videoFormat.xFramerate = 30 << 16;  // 30fps,Q16 format
    videoFormat.eCompressionFormat = OMX_VIDEO_CodingAVC;  // H264
    ret = codecComp_->SetParameter(codecComp_, OMX_IndexParamVideoPortFormat,
                                   (int8_t *)&videoFormat, sizeof(videoFormat));
    if (ret != HDF_SUCCESS) {
        MEDIA_LOG_E("SetParameter OMX_IndexParamVideoPortFormat failed, ret = " PUBLIC_LOG_D32, (int)ret);
        return;
    }
    ret = CheckAndUseBufferHandle();
    if (ret != HDF_SUCCESS) {
        MEDIA_LOG_E("failed exec CheckAndUseBufferHandle()");
    }
    MEDIA_LOG_D("ConfigOmx end");
}

void HdiAdapter::ConfigOmxPortDefine()
{
    MEDIA_LOG_D("ConfigOmxPortDefine begin");
    MEDIA_LOG_D("width_: " PUBLIC_LOG_U32 ", height_: " PUBLIC_LOG_U32 ", stride_: " PUBLIC_LOG_D32,
                width_, height_, stride_);
    // set width and height on input port
    OMX_PARAM_PORTDEFINITIONTYPE PortDef;
    InitParam(PortDef);
    PortDef.nPortIndex = (uint32_t) PortIndex::PORT_INDEX_INPUT;
    auto ret = codecComp_->GetParameter(codecComp_, OMX_IndexParamPortDefinition, (int8_t*) &PortDef, sizeof(PortDef));
    if (ret != HDF_SUCCESS) {
        MEDIA_LOG_E("GetParameter inPortDef failed, retVal = " PUBLIC_LOG_D32, static_cast<int32_t>(ret));
    }
    MEDIA_LOG_I("PortIndex::PORT_INDEX_INPUT eCompressionFormat = " PUBLIC_LOG_D32 ", eColorFormat = " PUBLIC_LOG_D32,
                PortDef.format.video.eCompressionFormat, PortDef.format.video.eColorFormat);
    PortDef.format.video.eCompressionFormat = OMX_VIDEO_CodingAVC;
    PortDef.format.video.nFrameWidth = width_;
    PortDef.format.video.nFrameHeight = height_;
    PortDef.format.video.nStride = stride_;
    PortDef.format.video.nSliceHeight = height_;
    ret = codecComp_->SetParameter(codecComp_, OMX_IndexParamPortDefinition, (int8_t*) &PortDef, sizeof(PortDef));
    if (ret != HDF_SUCCESS) {
        MEDIA_LOG_E("SetParameter PORT_INDEX_INPUT failed, ret = " PUBLIC_LOG_D32, (int) ret);
        return;
    }
    // set width, height and color format on output port
    InitParam(PortDef);
    PortDef.nPortIndex = (uint32_t) PortIndex::PORT_INDEX_OUTPUT;
    ret = codecComp_->GetParameter(codecComp_, OMX_IndexParamPortDefinition, (int8_t *) &PortDef, sizeof(PortDef));
    if (ret != HDF_SUCCESS) {
        MEDIA_LOG_E("GetParameter PORT_INDEX_OUTPUT failed, ret = " PUBLIC_LOG_D32, (int) ret);
        return;
    }
    MEDIA_LOG_I("PORT_INDEX_OUTPUT eCompressionFormat = " PUBLIC_LOG_D32 ", eColorFormat = " PUBLIC_LOG_D32,
                PortDef.format.video.eCompressionFormat, PortDef.format.video.eColorFormat);
    PortDef.format.video.nFrameWidth = width_;
    PortDef.format.video.nFrameHeight = height_;
    PortDef.format.video.nStride = stride_;
    PortDef.format.video.nSliceHeight = height_;
//    PortDef.format.video.eColorFormat = OMX_COLOR_FormatYUV422SemiPlanar;
    PortDef.format.video.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar; // 输出数据格式设置 VideoPixelFormat::NV12
    ret = codecComp_->SetParameter(codecComp_, OMX_IndexParamPortDefinition, (int8_t *) &PortDef, sizeof(PortDef));
    if (ret != HDF_SUCCESS) {
        MEDIA_LOG_E("SetParameter PORT_INDEX_OUTPUT failed, ret = " PUBLIC_LOG_D32, (int) ret);
        return;
    }
    MEDIA_LOG_D("ConfigOmxPortDefine end");
}

int32_t HdiAdapter::CheckAndUseBufferHandle()
{
    int32_t ret = HDF_SUCCESS;
    UseBufferType type;
    InitParamInOhos(type);
    type.portIndex = (uint32_t)PortIndex::PORT_INDEX_OUTPUT;
    type.bufferType = CODEC_BUFFER_TYPE_HANDLE;
    ret = codecComp_->SetParameter(codecComp_, OMX_IndexParamUseBufferType, (int8_t *)&type, sizeof(type));
    FALSE_LOG_MSG(ret == HDF_SUCCESS, "PORT_INDEX_OUTPUT, bufferTypes: " PUBLIC_LOG_D32 ", ret: " PUBLIC_LOG_S,
                  type.bufferType, TransHdfStatus2String(ret).c_str());
    MEDIA_LOG_D("CheckAndUseBufferHandle end");
    return ret;
}

template <typename T>
void HdiAdapter::InitParam(T& param)
{
    memset_s(&param, sizeof(param), 0x0, sizeof(param));
    param.nSize = sizeof(param);
    param.nVersion.s.nVersionMajor = verInfo_.compVersion.s.nVersionMajor;
}

template <typename T>
void HdiAdapter::InitParamInOhos(T &param)
{
    memset_s(&param, sizeof(param), 0x0, sizeof(param));
    param.size = sizeof(param);
    param.version.s.nVersionMajor = verInfo_.compVersion.s.nVersionMajor;
}

void HdiAdapter::UseOmxBuffers()
{
    MEDIA_LOG_D("UseBuffers begin");
    FALSE_RETURN_W(ChangeState(OMX_StateIdle) == Status::OK);
    auto ret = UseBufferOnInputPort(PortIndex::PORT_INDEX_INPUT, inBufferCnt_, inBufferSize_);
    if (ret != HDF_SUCCESS) {
        MEDIA_LOG_E("UseBufferOnInputPort error");
        return;
    }
    ret = UseBufferOnOutputPort(PortIndex::PORT_INDEX_OUTPUT, outBufferCnt_, outBufferSize_);
    if (ret != HDF_SUCCESS) {
        MEDIA_LOG_E("UseBufferOnOutputPort error");
        return;
    }
    enum OMX_STATETYPE status;
    ret = codecComp_->GetState(codecComp_, &status);
    if (ret != HDF_SUCCESS) {
        MEDIA_LOG_D("GetState err: " PUBLIC_LOG_D32, ret);
        return;
    }
    MEDIA_LOG_D("Wait for OMX_StateIdle status, current status: " PUBLIC_LOG_D32, static_cast<int32_t>(status));
    if (status != OMX_StateIdle) {
        FALSE_RETURN_W(WaitForState(OMX_StateIdle) == Status::OK);
    }
    MEDIA_LOG_D("UseBuffers end, curState_:" PUBLIC_LOG_D32, static_cast<int32_t>(curState_));
}

void HdiAdapter::GetBufferInfoOnPort(PortIndex portIndex)
{
    MEDIA_LOG_D("GetBufferInfoOnPort begin: " PUBLIC_LOG_S, TransPortIndex2String(portIndex).c_str());
    uint32_t bufferSize = 0;
    uint32_t bufferCount = 0;
    uint32_t bufferCountMin = 0;
    bool portEnable = false;

    OMX_PARAM_PORTDEFINITIONTYPE param;
    InitParam(param);
    param.nPortIndex = (OMX_U32)portIndex;
    auto ret = codecComp_->GetParameter(codecComp_, OMX_IndexParamPortDefinition, (int8_t *)&param, sizeof(param));
    FALSE_LOG_MSG(ret == HDF_SUCCESS, "failed to GetParameter with portIndex: " PUBLIC_LOG_D32,
                  static_cast<int>(portIndex));

    bufferSize = param.nBufferSize;
    bufferCount = param.nBufferCountActual;
    portEnable = param.bEnabled;
    bufferCountMin = param.nBufferCountMin;
    MEDIA_LOG_D("bufferCountMin: " PUBLIC_LOG_U32 ", portEnable: " PUBLIC_LOG_D32,
                bufferCountMin, static_cast<int>(portEnable));
    if (portIndex == PortIndex::PORT_INDEX_OUTPUT) {
        outBufferSize_ = bufferSize;
        outBufferCnt_ = bufferCount;
        MEDIA_LOG_D("outBufferCnt_: " PUBLIC_LOG_D32 ", outBufferSize_: " PUBLIC_LOG_D32,
                    outBufferCnt_, outBufferSize_);
    } else {
        inBufferSize_ = bufferSize;
        inBufferCnt_ = bufferCount;
        MEDIA_LOG_D("inBufferCnt_: " PUBLIC_LOG_D32 ", inBufferSize_: " PUBLIC_LOG_D32, inBufferCnt_, inBufferSize_);
    }
    // set port enable
    if (!portEnable) {
        ret = codecComp_->SendCommand(codecComp_, OMX_CommandPortEnable, (uint32_t)portIndex, NULL, 0);
        FALSE_LOG_MSG_W(ret == HDF_SUCCESS, "SendCommand OMX_CommandPortEnable failed, portIndex: " PUBLIC_LOG_D32,
                        (int)portIndex);
    }
}

int32_t HdiAdapter::UseBufferOnInputPort(PortIndex portIndex, int bufferCount, int bufferSize)
{
    MEDIA_LOG_D("UseBufferOnInputPort begin");
    if (bufferCount <= 0 || bufferSize <= 0) {
        return HDF_ERR_INVALID_PARAM;
    }
    for (int i = 0; i < bufferCount; i++) {
        std::shared_ptr<OmxCodecBuffer> omxBuffer = std::make_shared<OmxCodecBuffer>();
        omxBuffer->size = sizeof(OmxCodecBuffer);
        omxBuffer->version.s.nVersionMajor = verInfo_.compVersion.s.nVersionMajor;
        omxBuffer->bufferType = CODEC_BUFFER_TYPE_AVSHARE_MEM_FD;
        omxBuffer->bufferLen = sizeof(int);
        omxBuffer->allocLen = bufferSize;
        omxBuffer->fenceFd = -1;
        omxBuffer->pts = 0;
        omxBuffer->flag = 0;

        omxBuffer->type = READ_ONLY_TYPE;
        shaAlloc_ = std::make_shared<ShareAllocator>(Plugin::ShareMemType::READ_WRITE_TYPE);
        std::shared_ptr<ShareMemory> sharedMem = std::make_shared<ShareMemory>(bufferSize, shaAlloc_, 0);
        omxBuffer->buffer = (uint8_t *)(long long)sharedMem->GetShareMemoryFd();
        MEDIA_LOG_D("share memory fd: " PUBLIC_LOG_D32, sharedMem->GetShareMemoryFd());

        auto err = codecComp_->UseBuffer(codecComp_, (uint32_t)portIndex, omxBuffer.get());
        if (err != HDF_SUCCESS) {
            MEDIA_LOG_E("failed to UseBuffer with PORT_INDEX_INPUT");
            sharedMem = nullptr;
            return err;
        }
        omxBuffer->bufferLen = 0;
        MEDIA_LOG_I("UseBuffer returned in bufferID: " PUBLIC_LOG_D32, (int)omxBuffer->bufferId);

        std::shared_ptr<BufferInfo> bufferInfo = std::make_shared<BufferInfo>();
        bufferInfo->omxBuffer = omxBuffer;
        bufferInfo->avSharedPtr = sharedMem;
        bufferInfoMap_.emplace(std::make_pair(omxBuffer->bufferId, bufferInfo));
        freeInBufferId_.Push(omxBuffer->bufferId);
    }
    MEDIA_LOG_D("UseBufferOnInputPort end");
    return HDF_SUCCESS;
}

int32_t HdiAdapter::UseBufferOnOutputPort(PortIndex portIndex, int bufferCount, int bufferSize)
{
    MEDIA_LOG_D("UseBufferOnOutputPort begin");
    if (bufferCount <= 0 || bufferSize <= 0) {
        return HDF_ERR_INVALID_PARAM;
    }
    int32_t ret = HDF_SUCCESS;
    for (uint32_t i = 0; i < outBufferCnt_; i++) {
        std::shared_ptr<OmxCodecBuffer> omxBuffer = std::make_shared<OmxCodecBuffer>();
        std::shared_ptr<Buffer> outputBuffer = outBufQue_.Pop();
        omxBuffer->size = sizeof(OmxCodecBuffer);
        omxBuffer->version.s.nVersionMajor = verInfo_.compVersion.s.nVersionMajor;
        omxBuffer->bufferType = CODEC_BUFFER_TYPE_HANDLE;
        BufferHandle* bufferHandle = std::static_pointer_cast<Plugin::SurfaceMemory>(outputBuffer->GetMemory())->
            GetSurfaceBuffer()->GetBufferHandle();
        if (!bufferHandle) {
            MEDIA_LOG_W("bufferHandle is null: " PUBLIC_LOG_P, bufferHandle);
        }
        size_t handleSize =
                sizeof(BufferHandle) + (sizeof(int32_t) * (bufferHandle->reserveFds + bufferHandle->reserveInts));
        omxBuffer->bufferLen = handleSize;
        omxBuffer->buffer = (uint8_t*)bufferHandle;
        omxBuffer->allocLen = bufferSize;
        omxBuffer->fenceFd = -1; // check use -1 first with no window
        omxBuffer->pts = 0;
        omxBuffer->flag = 0;
        ret = codecComp_->UseBuffer(codecComp_, (uint32_t)PortIndex::PORT_INDEX_OUTPUT, omxBuffer.get());
        FALSE_RETURN_V_MSG_E(ret == HDF_SUCCESS, ret, "failed to UseBuffer with output port, bufferId: " PUBLIC_LOG_U32,
                             omxBuffer->bufferId);
        omxBuffer->bufferLen = 0;
        MEDIA_LOG_D("UseBuffer returned out bufferID: " PUBLIC_LOG_U32, omxBuffer->bufferId);

        std::shared_ptr<BufferInfo> bufferInfo = std::make_shared<BufferInfo>();
        bufferInfo->omxBuffer = omxBuffer;
        bufferInfo->avSharedPtr = nullptr;
        bufferInfo->outputBuffer = outputBuffer;
        bufferInfoMap_.emplace(std::make_pair(omxBuffer->bufferId, bufferInfo));
        freeOutBufferId_.Push(omxBuffer->bufferId);
    }
    MEDIA_LOG_D("UseBufferOnOutputPort end, outBufQue_.size: " PUBLIC_LOG_ZU, outBufQue_.Size());
    return HDF_SUCCESS;
}

void HdiAdapter::WaitForEvent(OMX_U32 cmd)
{
    MEDIA_LOG_D("WaitForEvent begin");
    OSAL::ScopedLock lock(mutex_);
    uint32_t newCmd = static_cast<uint32_t>(cmd);
    MEDIA_LOG_D("Wait eventdone:" PUBLIC_LOG_D32 ", lastcmd:" PUBLIC_LOG_D32 ", cmd:" PUBLIC_LOG_U32,
                eventDone_, lastCmd_, cmd);
    cond_.Wait(lock, [this, &newCmd]() { return eventDone_ && (lastCmd_ == (int)newCmd || lastCmd_ == -1); });
    eventDone_ = false;
    MEDIA_LOG_D("WaitForEvent end");
}

Status HdiAdapter::WaitForState(OMX_STATETYPE state)
{
    MEDIA_LOG_D("WaitForState begin");
    WaitForEvent(OMX_CommandStateSet);
    if (curState_ != state) {
        MEDIA_LOG_D("Wait state failed");
        return Status::ERROR_WRONG_STATE;
    }
    MEDIA_LOG_D("WaitForState end");
    return Status::OK;
}

Status HdiAdapter::ChangeState(OMX_STATETYPE state)
{
    MEDIA_LOG_I("change state from " PUBLIC_LOG_S " to " PUBLIC_LOG_S,
                omxStateToString[targetState_].c_str(), omxStateToString[state].c_str());
    if (targetState_ != state && curState_ != state) {
        if (codecComp_ && codecComp_->SendCommand) {
            auto ret = codecComp_->SendCommand(codecComp_, OMX_CommandStateSet, state, nullptr, 0);
            FALSE_RETURN_V_MSG(ret == HDF_SUCCESS, Status::ERROR_UNKNOWN, "ChangeState failed");
            targetState_ = state;
        }
    }
    MEDIA_LOG_D("change statue end");
    return Status::OK;
}

void HdiAdapter::HandelEventCmdComplete(OMX_U32 data1, OMX_U32 data2)
{
    MEDIA_LOG_D("HandelEventCmdComplete-callback begin");
    OSAL::ScopedLock lock(mutex_);
    lastCmd_ = static_cast<int>(data1);
    switch (data1) {
        case OMX_CommandStateSet:
            HandelEventStateSet(data2);
            break;
        default:
            break;
    }
    cond_.NotifyOne();
}

void HdiAdapter::HandelEventStateSet(OMX_U32 data)
{
    MEDIA_LOG_D("HandelEventStateSet-callback begin");
    MEDIA_LOG_I("change curState_ from " PUBLIC_LOG_S " to " PUBLIC_LOG_S,
                omxStateToString[curState_].c_str(), omxStateToString[static_cast<OMX_STATETYPE>(data)].c_str());
    curState_ = static_cast<OMX_STATETYPE>(data);
    eventDone_ = true;
}
} // namespace Plugin
} // namespace Media
} // namespace OHOS