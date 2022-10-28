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

#if !defined(OHOS_LITE) && defined(VIDEO_SUPPORT)

#define HST_LOG_TAG "HdiAdapter"

#include "hdi_adapter.h"
#include "codec_callback_type_stub.h"
#include "codec_omx_ext.h"
#include "common/surface_memory.h"
#include "display_type.h"
#include "foundation/log.h"
#include "hdf_base.h"
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
constexpr size_t DEFAULT_IN_BUFFER_ID_QUEUE_SIZE = 4;
constexpr size_t DEFAULT_OUT_BUFFER_ID_QUEUE_SIZE = 21;
constexpr size_t DEFAULT_OUT_BUFFER_QUEUE_SIZE = 21;
constexpr uint32_t DEFAULT_FRAME_RATE = 30 << 16; // 30fps,Q16 format

void UpdateInCaps(const CodecCompCapability& cap, CodecPluginDef& definition)
{
    CapabilityBuilder incapBuilder;
    switch (cap.role) {
        case MEDIA_ROLETYPE_VIDEO_AVC:
            incapBuilder.SetMime(OHOS::Media::MEDIA_MIME_VIDEO_H264);
            incapBuilder.SetVideoBitStreamFormatList({VideoBitStreamFormat::ANNEXB});
            break;
        default:
            incapBuilder.SetMime("video/unknown");
            break;
    }
    definition.inCaps.push_back(incapBuilder.Build());
}

void UpdateOutCaps(const CodecCompCapability& cap, CodecPluginDef& definition)
{
    CapabilityBuilder outcapBuilder;
    outcapBuilder.SetMime(OHOS::Media::MEDIA_MIME_VIDEO_RAW);
    int32_t index = 0;
    std::vector<Plugin::VideoPixelFormat> formats;
    for (index = 0; cap.port.video.supportPixFmts[index] != 0; ++index) {
        auto supportFormat = ConvertPixelFormatFromHdi(cap.port.video.supportPixFmts[index]);
        if (supportFormat != VideoPixelFormat::UNKNOWN) {
            formats.push_back(supportFormat);
        }
    }
    if (index) {
        outcapBuilder.SetVideoPixelFormatList(formats);
    }
    definition.outCaps.push_back(outcapBuilder.Build());
}

bool TranslateVideoDecoderCap(const CodecCompCapability& cap, CodecPluginDef& def)
{
    UpdateInCaps(cap, def);
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
        MEDIA_LOG_E("Codec type of plugin " PUBLIC_LOG_S "." PUBLIC_LOG_S " mismatched",
                    packageName.c_str(), capability.compName);
        return false;
    }
    if (!TranslateCapability(capability, def)) {
        MEDIA_LOG_W("Codec capability of plugin " PUBLIC_LOG_S "." PUBLIC_LOG_S " translate failed",
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
        MEDIA_LOG_E("Codec package " PUBLIC_LOG_S " has no valid component manager", packageName.c_str());
        return Status::ERROR_INVALID_DATA;
    }
    std::dynamic_pointer_cast<OHOS::Media::Plugin::PackageRegister>(reg)->
        AddPackage({PLUGIN_INTERFACE_VERSION, packageName, OHOS::Media::Plugin::LicenseType::APACHE_V2});

    int32_t count = g_compManager->GetComponentNum();
    MEDIA_LOG_D("Component number is: " PUBLIC_LOG_D32, count);
    CodecCompCapability capList[count];
    g_compManager->GetComponentCapabilityList(capList, count);
    for (int32_t i = 0; i < count; i++) {
        if (capList[i].type != ::CodecType::VIDEO_DECODER) {
            continue;
        }
        CodecPluginDef definition;
        if (TranslateCapToPluginDef(capList[i], definition, packageName)) {
            definition.rank = 100; // 100 default rank
            if (reg->AddPlugin(definition) != Status::OK) {
                MEDIA_LOG_E("Add plugin " PUBLIC_LOG_S " failed", definition.name.c_str());
            }
        }
    }
    MEDIA_LOG_D("Register HDI adapter done");
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
        case OMX_EventCmdComplete:
            hdiAdapter->HandelCmdCompleteEvent(info->data1, info->data2);
            break;
        default:
            break;
    }
    MEDIA_LOG_D("EventHandler-callback end");
    return HDF_SUCCESS;
}

int32_t HdiAdapter::EmptyBufferDone(CodecCallbackType* self, int64_t appData, const OmxCodecBuffer* buffer)
{
    MEDIA_LOG_DD("EmptyBufferDone-callback begin, bufferId: " PUBLIC_LOG_U32, buffer->bufferId);
    auto hdiAdapter = reinterpret_cast<HdiAdapter*>(appData);
    hdiAdapter->freeInBufferId_.Push(buffer->bufferId);
    if (!hdiAdapter->isFlushing_) {
        hdiAdapter->HandleFrame();
    }
    MEDIA_LOG_D("EmptyBufferDone-callback end, free in bufferId count: " PUBLIC_LOG_ZU,
                hdiAdapter->freeInBufferId_.Size());
    return HDF_SUCCESS;
}

int32_t HdiAdapter::FillBufferDone(CodecCallbackType* self, int64_t appData, const OmxCodecBuffer* omxBuffer)
{
    MEDIA_LOG_DD("FillBufferDone-callback begin, bufferId: " PUBLIC_LOG_U32 ", flag: " PUBLIC_LOG_U32
        ", pts: " PUBLIC_LOG_D64, omxBuffer->bufferId, omxBuffer->flag, omxBuffer->pts);
    auto hdiAdapter = reinterpret_cast<HdiAdapter*>(appData);
    auto iter = hdiAdapter->bufferInfoMap_.find(omxBuffer->bufferId);
    if ((iter == hdiAdapter->bufferInfoMap_.end()) || (iter->second == nullptr)) {
        MEDIA_LOG_DD("iter == hdiAdapter->omxBuffers_.end() || iter->second == nullptr");
        return HDF_ERR_INVALID_PARAM;
    }
    auto bufferInfo = iter->second;
    auto outputBuffer = bufferInfo->outputBuffer;
    bufferInfo->outputBuffer = nullptr; // Need: to release output buffer, Decrease the reference count
    hdiAdapter->freeOutBufferId_.Push(omxBuffer->bufferId);
    if (hdiAdapter->isFlushing_) {
        MEDIA_LOG_DD("hdi adapter is flushing, ignore this data");
        outputBuffer = nullptr;
        return HDF_SUCCESS;
    }
    outputBuffer->flag = Translate2PluginFlagSet(omxBuffer->flag);
    outputBuffer->pts = omxBuffer->pts;
    hdiAdapter->NotifyOutputBufferDone(outputBuffer);

    (void)hdiAdapter->FillAllTheOutBuffer(); // call FillThisBuffer() again
    MEDIA_LOG_D("FillBufferDone-callback end, free out bufferId count: " PUBLIC_LOG_ZU,
                hdiAdapter->freeOutBufferId_.Size());
    return HDF_SUCCESS;
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

HdiAdapter::HdiAdapter(std::string name)
    : CodecPlugin(std::forward<std::string>(name)),
      freeInBufferId_("hdiFreeInBufferId", DEFAULT_IN_BUFFER_ID_QUEUE_SIZE),
      freeOutBufferId_("hdiFreeOutBufferId", DEFAULT_OUT_BUFFER_ID_QUEUE_SIZE),
      outBufQue_("hdiAdapterOutQueue", DEFAULT_OUT_BUFFER_QUEUE_SIZE)
{
    shaAlloc_ = std::make_shared<ShareAllocator>(Plugin::ShareMemType::READ_WRITE_TYPE);
    MEDIA_LOG_D("codec adapter ctor");
}

HdiAdapter::~HdiAdapter()
{
    if (codecCallback_) {
        CodecCallbackTypeStubRelease(codecCallback_);
        codecCallback_ = nullptr;
    }
}

Status HdiAdapter::Init()
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
    compManager_ = GetCodecComponentManager();
    codecCallback_ = CodecCallbackTypeStubGetInstance();
    FALSE_RETURN_V_MSG(codecCallback_ != nullptr, Status::ERROR_NULL_POINTER, "create callback_ failed");
    FALSE_RETURN_V_MSG(compManager_ != nullptr, Status::ERROR_NULL_POINTER, "create component manager failed");
    codecCallback_->EventHandler = &HdiAdapter::EventHandler;
    codecCallback_->EmptyBufferDone = &HdiAdapter::EmptyBufferDone;
    codecCallback_->FillBufferDone = &HdiAdapter::FillBufferDone;

    int32_t ret = compManager_->CreateComponent(&codecComp_, &componentId_, const_cast<char*>(componentName_.c_str()),
                                                (int64_t)this, codecCallback_);
    FALSE_RETURN_V_MSG(codecComp_ != nullptr, Status::ERROR_NULL_POINTER,
                       "create component failed, retVal = " PUBLIC_LOG_D32, (int)ret);

    // 获取组件版本号
    (void)memset_s(&verInfo_, sizeof(verInfo_), 0, sizeof(verInfo_));
    ret = codecComp_->GetComponentVersion(codecComp_, &verInfo_);
    FALSE_RETURN_V_MSG_E(ret == HDF_SUCCESS, Status::ERROR_INVALID_DATA,
                         "get component version failed, ret: " PUBLIC_LOG_D32, ret);
    bufferConfigured_ = false;
    MEDIA_LOG_D("codec adapter init end, component Id = " PUBLIC_LOG_D32, componentId_);
    return Status::OK;
}

Status HdiAdapter::Deinit()
{
    MEDIA_LOG_D("HdiAdapter DeInit Enter");
    FALSE_RETURN_V_MSG_E(Reset() == Status::OK, Status::ERROR_INVALID_DATA, "Reset value failed");
    auto ret = compManager_->DestroyComponent(componentId_);
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
    if (compManager_) {
        CodecComponentManagerRelease();
        compManager_ = nullptr;
    }
    MEDIA_LOG_D("HdiAdapter DeInit End;");
    return  Status::OK;
}

Status HdiAdapter::Prepare()
{
    outBufQue_.SetActive(true);
    InitOmxBuffers(); // 申请 omx buffer
    MEDIA_LOG_D("prepare end");
    return Status::OK;
}

Status HdiAdapter::Reset()
{
    auto ret = codecComp_->SendCommand(codecComp_, OMX_CommandStateSet, OMX_StateIdle, nullptr, 0);
    FALSE_RETURN_V_MSG_E(ret == HDF_SUCCESS, TransHdiRetVal2Status(ret), "Change omx state to Idle failed");
    auto val = FreeBuffers();
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
    curState_ = OMX_StateInvalid;
    return Status::OK;
}

Status HdiAdapter::Start()
{
    MEDIA_LOG_D("start begin");
    FALSE_RETURN_V_MSG_E(ChangeState(OMX_StateExecuting) == Status::OK, Status::ERROR_WRONG_STATE,
                         "Change omx state to executing failed");
    FALSE_RETURN_V_MSG_E(WaitForState(OMX_StateExecuting) == Status::OK, Status::ERROR_WRONG_STATE,
                         "Wait omx state to executing failed");
    if (!FillAllTheOutBuffer()) {
        MEDIA_LOG_E("Fill all buffer error");
        return Status::ERROR_UNKNOWN;
    }
    outBufQue_.SetActive(true);
    curState_ = OMX_StateExecuting;
    MEDIA_LOG_D("start end");
    return Status::OK;
}

Status HdiAdapter::Stop()
{
    MEDIA_LOG_D("HdiAdapter Stop Enter");
    outBufQue_.SetActive(false);
    MEDIA_LOG_D("HdiAdapter Stop End");
    return Status::OK;
}

Status HdiAdapter::GetParameter(Plugin::Tag tag, ValueType& value)
{
    MEDIA_LOG_D("GetParameter begin");
    switch (tag) {
        case Tag::REQUIRED_OUT_BUFFER_CNT:
            value = outBufferCnt_;
            break;
        case Tag::REQUIRED_OUT_BUFFER_SIZE:
            value = outBufferSize_;
            break;
        default:
            MEDIA_LOG_W("ignore this tag: " PUBLIC_LOG_S, Pipeline::Tag2String(tag));
            break;
    }
    return Status::OK;
}

Status HdiAdapter::SetParameter(Plugin::Tag tag, const ValueType& value)
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
    if (width_ != 0 && height_ != 0 && pixelFormat_ != VideoPixelFormat::UNKNOWN && !bufferConfigured_) {
        FALSE_RETURN_V_MSG_E(ConfigOmx() == Status::OK, Status::ERROR_INVALID_OPERATION, "Configure omx failed");
    }
    MEDIA_LOG_D("SetParameter end");
    return Status::OK;
}

std::shared_ptr<Plugin::Allocator> HdiAdapter::GetAllocator()
{
    MEDIA_LOG_D("GetAllocator begin");
    return nullptr;
}

Status HdiAdapter::SetCallback(Callback* cb)
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

// 循环从输入buffer队列中取出一个buffer，转换成 omxBuffer 后调用 HDI 的 EmptyThisBuffer() 进行解码
void HdiAdapter::HandleFrame()
{
    MEDIA_LOG_DD("handle frame begin");
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
    MEDIA_LOG_DD("handle frame end");
}

Status HdiAdapter::TransInputBuffer2OmxBuffer(const std::shared_ptr<Plugin::Buffer>& pluginBuffer,
                                              std::shared_ptr<BufferInfo>& bufferInfo)
{
    bufferInfo->omxBuffer->flag = Translate2omxFlagSet(pluginBuffer->flag);
    bufferInfo->omxBuffer->pts = pluginBuffer->pts;
    MEDIA_LOG_DD("plugin flag: " PUBLIC_LOG_U32 ", pts: " PUBLIC_LOG_D64,
                 bufferInfo->omxBuffer->flag, bufferInfo->omxBuffer->pts);
    if (pluginBuffer->flag & BUFFER_FLAG_EOS) {
        MEDIA_LOG_D("EOS flag receive, return");
        return Status::OK;
    }
    auto mem = pluginBuffer->GetMemory();
    if (mem == nullptr) {
        MEDIA_LOG_DD("pluginBuffer->GetMemory() return nullptr");
        return Status::ERROR_INVALID_DATA;
    }
    const uint8_t* memAddr = mem->GetReadOnlyData();
    if (memAddr == nullptr) {
        MEDIA_LOG_DD("mem->GetReadOnlyData() return nullptr");
        return Status::ERROR_INVALID_DATA;
    }
    size_t bufLen = mem->GetSize();

    (void)bufferInfo->avSharedPtr->Write(memAddr, bufLen, 0);
    bufferInfo->omxBuffer->offset = 0;
    bufferInfo->omxBuffer->filledLen = bufLen;
    MEDIA_LOG_DD("TransBuffer2OmxBuffer end, bufferId: " PUBLIC_LOG_U32, bufferInfo->omxBuffer->bufferId);
    return Status::OK;
}

bool HdiAdapter::FillAllTheOutBuffer()
{
    MEDIA_LOG_DD("FillAllTheBuffer begin");
    if (isFirstCall_) {
        isFirstCall_ = false;
        for (uint32_t i = 0; i < outBufferCnt_; ++i) {
            int outBufferID = static_cast<int>(freeOutBufferId_.Pop());
            auto iter = bufferInfoMap_.find(outBufferID);
            auto bufferInfo = iter->second;
            auto omxBuffer = bufferInfo->omxBuffer;
            int32_t ret = HDF_SUCCESS;
            if (codecComp_ && codecComp_->FillThisBuffer) {
                ret = codecComp_->FillThisBuffer(codecComp_, omxBuffer.get());
                FALSE_RETURN_V_MSG_E(ret == HDF_SUCCESS, false, "call FillThisBuffer() error, bufferId: "
                    PUBLIC_LOG_U32 ", ret: " PUBLIC_LOG_S ", isFirstCall: " PUBLIC_LOG_D32, omxBuffer->bufferId,
                    HdfStatus2String(ret).c_str(), isFirstCall_);
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
                FALSE_RETURN_V_MSG_E(ret == HDF_SUCCESS, false, "call FillThisBuffer() error, bufferId: "
                    PUBLIC_LOG_U32 ", ret: " PUBLIC_LOG_S ", isFirstCall: " PUBLIC_LOG_D32, omxBuffer->bufferId,
                    HdfStatus2String(ret).c_str(), isFirstCall_);
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
        FillAllTheOutBuffer();
    }
    MEDIA_LOG_DD("QueueOutputBuffer end");
    return Status::OK;
}

Status HdiAdapter::Flush()
{
    MEDIA_LOG_D("HdiAdapter Flush begin");
    OSAL::ScopedLock lock(flushMutex_);
    isFlushing_ = true;
    {
        OSAL::ScopedLock l(lockInputBuffers_);
        inBufQue_.clear();
    }
    // -1: Refresh input and output ports
    auto ret = codecComp_->SendCommand(codecComp_, OMX_CommandFlush, -1, nullptr, 0);
    FALSE_RETURN_V_MSG_E(ret == HDF_SUCCESS, TransHdiRetVal2Status(ret), "Flush in/out port failed, ret: " PUBLIC_LOG_S,
                         HdfStatus2String(ret).c_str());
    flushCond_.Wait(lock, [this]() {
        return !isFlushing_;
    });
    MEDIA_LOG_D("HdiAdapter Flush end");
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
    }
    MEDIA_LOG_DD("NotifyOutputBufferDone end");
}

Status HdiAdapter::ConfigOmx()
{
    MEDIA_LOG_D("ConfigOmx begin");
    Status ret = Status::OK;
    ret = ConfigInPortVideoFormat();
    FALSE_RETURN_V_MSG(ret == Status::OK, ret, "failed exec ConfigInPortVideoFormat()");
    ret = ConfigOutPortBufType();
    FALSE_RETURN_V_MSG(ret == Status::OK, ret, "failed exec ConfigOutPortBufType()");

    ret = ConfigOmxPortDefine(PortIndex::PORT_INDEX_INPUT);
    FALSE_RETURN_V_MSG(ret == Status::OK, ret, "ConfigOmxPortDefine of INPUT failed, retVal = " PUBLIC_LOG_D32, ret);
    ret = ConfigOmxPortDefine(PortIndex::PORT_INDEX_OUTPUT);
    FALSE_RETURN_V_MSG(ret == Status::OK, ret, "ConfigOmxPortDefine of OUTPUT failed, retVal = " PUBLIC_LOG_D32, ret);

    GetBufferInfoOnPort(PortIndex::PORT_INDEX_INPUT, inBufferCnt_, inBufferSize_);
    GetBufferInfoOnPort(PortIndex::PORT_INDEX_OUTPUT, outBufferCnt_, outBufferSize_);
    bufferConfigured_ = true;
    MEDIA_LOG_D("ConfigOmx end");
    return ret;
}

Status HdiAdapter::ConfigOmxPortDefine(PortIndex portIndex)
{
    int32_t ret = HDF_SUCCESS;
    MEDIA_LOG_D("ConfigOmxPortDefine begin, PortIndex: " PUBLIC_LOG_S, PortIndex2String(portIndex).c_str());
    MEDIA_LOG_D("width_: " PUBLIC_LOG_U32 ", height_: " PUBLIC_LOG_U32 ", stride_: " PUBLIC_LOG_D32,
                width_, height_, stride_);
    OMX_PARAM_PORTDEFINITIONTYPE portDef;
    InitOmxParam(portDef, verInfo_);
    portDef.nPortIndex = (uint32_t)portIndex;

    // Get other default value, cause InitOmxParam set value = 0
    ret = codecComp_->GetParameter(codecComp_, OMX_IndexParamPortDefinition, (int8_t*) &portDef, sizeof(portDef));
    FALSE_RETURN_V_MSG(ret == HDF_SUCCESS, TransHdiRetVal2Status(ret),
                       "GetParameter failed, retVal = " PUBLIC_LOG_D32, ret);
    MEDIA_LOG_I("eCompressionFormat = " PUBLIC_LOG_D32 ", eColorFormat = " PUBLIC_LOG_D32,
                portDef.format.video.eCompressionFormat, portDef.format.video.eColorFormat);
    if (portIndex == PortIndex::PORT_INDEX_INPUT) {
        portDef.format.video.eCompressionFormat = OMX_VIDEO_CodingAVC;
    } else {
        switch (pixelFormat_) {
            case VideoPixelFormat::NV12:
                portDef.format.video.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
                break;
            default:
                MEDIA_LOG_E("Current color format not support, pixelFormat: " PUBLIC_LOG_U32, pixelFormat_);
                break;
        }
    }
    portDef.format.video.nFrameWidth = width_;
    portDef.format.video.nFrameHeight = height_;
    portDef.format.video.nStride = stride_;
    portDef.format.video.nSliceHeight = height_;
    ret = codecComp_->SetParameter(codecComp_, OMX_IndexParamPortDefinition, (int8_t*) &portDef, sizeof(portDef));
    FALSE_RETURN_V_MSG(ret == HDF_SUCCESS, TransHdiRetVal2Status(ret),
                       "SetParameter failed, ret = " PUBLIC_LOG_D32, ret);
    MEDIA_LOG_D("ConfigOmxPortDefine end");
    return TransHdiRetVal2Status(ret);
}

Status HdiAdapter::ConfigInPortVideoFormat()
{
    // 设置输入数据为H264编码
    int32_t ret = HDF_SUCCESS;
    OMX_VIDEO_PARAM_PORTFORMATTYPE videoFormat;
    InitOmxParam(videoFormat, verInfo_);
    videoFormat.nPortIndex = (uint32_t)PortIndex::PORT_INDEX_INPUT;
    ret = codecComp_->GetParameter(codecComp_, OMX_IndexParamVideoPortFormat,
                                   (int8_t *)&videoFormat, sizeof(videoFormat));
    FALSE_RETURN_V_MSG(ret == HDF_SUCCESS, TransHdiRetVal2Status(ret),
                       "GetParameter OMX_IndexParamVideoPortFormat failed, ret = " PUBLIC_LOG_D32, ret);
    MEDIA_LOG_D("set Format PORT_INDEX_INPUT eCompressionFormat = " PUBLIC_LOG_D32 ", eColorFormat = " PUBLIC_LOG_D32,
                videoFormat.eCompressionFormat, videoFormat.eColorFormat);
    if (frameRate_ == 0) {
        frameRate_ = DEFAULT_FRAME_RATE;
    }
    videoFormat.xFramerate = frameRate_;
    if (componentName_ == "OMX.rk.video_decoder.avc") { // HdiAdapter.OMX.rk.video_decoder.avc
        videoFormat.eCompressionFormat = OMX_VIDEO_CodingAVC;
    } else {
        MEDIA_LOG_E("Can not enable this compression format");
        return Status::ERROR_INVALID_DATA;
    }
    ret = codecComp_->SetParameter(codecComp_, OMX_IndexParamVideoPortFormat,
                                   (int8_t *)&videoFormat, sizeof(videoFormat));
    FALSE_LOG_MSG(ret == HDF_SUCCESS, "SetParameter OMX_IndexParamVideoPortFormat failed, ret = " PUBLIC_LOG_D32, ret);
    return TransHdiRetVal2Status(ret);
}

Status HdiAdapter::ConfigOutPortBufType()
{
    int32_t ret = HDF_SUCCESS;
    UseBufferType type;
    InitHdiParam(type, verInfo_);
    type.portIndex = (uint32_t)PortIndex::PORT_INDEX_OUTPUT;
    type.bufferType = CODEC_BUFFER_TYPE_HANDLE;
    ret = codecComp_->SetParameter(codecComp_, OMX_IndexParamUseBufferType, (int8_t *)&type, sizeof(type));
    FALSE_LOG_MSG(ret == HDF_SUCCESS, "PORT_INDEX_OUTPUT, bufferTypes: " PUBLIC_LOG_D32 ", ret: " PUBLIC_LOG_S,
                  type.bufferType, HdfStatus2String(ret).c_str());
    MEDIA_LOG_D("ConfigOutPortBufType end");
    return TransHdiRetVal2Status(ret);
}

// 将状态改为Idle, 并根据buffer大小和个数 初始化HDI组件的输入输出 buffer
void HdiAdapter::InitOmxBuffers()
{
    MEDIA_LOG_D("UseBuffers begin");
    FALSE_RETURN_MSG(ChangeState(OMX_StateIdle) == Status::OK, "Change hdi state to idle failed");

    auto ret = InitBufferOnPort(PortIndex::PORT_INDEX_INPUT, inBufferCnt_, inBufferSize_);
    FALSE_RETURN_MSG(ret == Status::OK, "UseBufferOnInputPort error");
    ret = InitBufferOnPort(PortIndex::PORT_INDEX_OUTPUT, outBufferCnt_, outBufferSize_);
    FALSE_RETURN_MSG(ret == Status::OK, "UseBufferOnOutputPort error");

    // Only after in port & out port inited, hdi change state to idle success
    FALSE_RETURN_MSG(WaitForState(OMX_StateIdle) == Status::OK, "Wait hdi state to idle failed");
    MEDIA_LOG_D("UseBuffers end, curState:" PUBLIC_LOG_D32, static_cast<int32_t>(curState_));
}

void HdiAdapter::GetBufferInfoOnPort(PortIndex portIndex, uint32_t& bufCount, uint32_t& bufSize)
{
    MEDIA_LOG_D("GetBufferInfoOnPort begin: " PUBLIC_LOG_S, PortIndex2String(portIndex).c_str());
    bool portEnable = false;

    OMX_PARAM_PORTDEFINITIONTYPE param;
    InitOmxParam(param, verInfo_);
    param.nPortIndex = (OMX_U32)portIndex;
    auto ret = codecComp_->GetParameter(codecComp_, OMX_IndexParamPortDefinition, (int8_t *)&param, sizeof(param));
    FALSE_LOG_MSG(ret == HDF_SUCCESS, "failed to GetParameter with portIndex: " PUBLIC_LOG_D8, portIndex);

    bufSize = param.nBufferSize;
    bufCount = param.nBufferCountActual;
    MEDIA_LOG_D("PortIndex: " PUBLIC_LOG_S ", bufCnt: " PUBLIC_LOG_D32 ", bufSize: " PUBLIC_LOG_D32,
                PortIndex2String(portIndex).c_str(), bufCount, bufSize);

    portEnable = param.bEnabled;
    MEDIA_LOG_D("portEnable: " PUBLIC_LOG_D8, portEnable);
    if (!portEnable) {
        ret = codecComp_->SendCommand(codecComp_, OMX_CommandPortEnable, (uint32_t)portIndex, NULL, 0);
        FALSE_LOG_MSG_W(ret == HDF_SUCCESS, "SendCommand OMX_CommandPortEnable failed, portIndex: " PUBLIC_LOG_D32,
                        (int)portIndex);
    }
}

Status HdiAdapter::InitBufferOnPort(PortIndex portIndex, uint32_t bufferCount, uint32_t bufferSize)
{
    MEDIA_LOG_D("InitBufferOnPort begin");
    for (uint32_t i = 0; i < bufferCount; i++) {
        std::shared_ptr<OmxCodecBuffer> omxBuffer = nullptr;
        std::shared_ptr<ShareMemory> sharedMem = nullptr;
        std::shared_ptr<Buffer> outputBuffer = nullptr;
        if (portIndex == PortIndex::PORT_INDEX_INPUT) {
            sharedMem = std::make_shared<ShareMemory>(bufferSize, shaAlloc_, 0);
            omxBuffer = InitOmxBuffer(sharedMem, nullptr, portIndex, bufferSize);
        } else {
            outputBuffer = outBufQue_.Pop();
            omxBuffer = InitOmxBuffer(nullptr, outputBuffer, portIndex, bufferSize);
        }
        if (codecComp_ && codecComp_->UseBuffer) {
            auto err = codecComp_->UseBuffer(codecComp_, (uint32_t)portIndex, omxBuffer.get());
            if (err != HDF_SUCCESS) {
                MEDIA_LOG_E("failed to UseBuffer");
                sharedMem = nullptr;
                return Status::ERROR_INVALID_DATA;
            }
        }
        omxBuffer->bufferLen = 0;
        MEDIA_LOG_D("UseBuffer returned bufferID: " PUBLIC_LOG_D32 "PortIndex: " PUBLIC_LOG_S,
                    (int)omxBuffer->bufferId, PortIndex2String(portIndex).c_str());
        std::shared_ptr<BufferInfo> bufferInfo = std::make_shared<BufferInfo>();
        bufferInfo->omxBuffer = omxBuffer;
        bufferInfo->avSharedPtr = sharedMem;
        bufferInfo->outputBuffer = outputBuffer;
        bufferInfo->portIndex = portIndex;
        bufferInfoMap_.emplace(std::make_pair(omxBuffer->bufferId, bufferInfo));
        if (portIndex == PortIndex::PORT_INDEX_INPUT) {
            freeInBufferId_.Push(omxBuffer->bufferId);
        } else {
            freeOutBufferId_.Push(omxBuffer->bufferId);
        }
    }
    MEDIA_LOG_D("InitBufferOnPort end");
    return Status::OK;
}

std::shared_ptr<OmxCodecBuffer> HdiAdapter::InitOmxBuffer(std::shared_ptr<ShareMemory> sharedMem,
                                                          std::shared_ptr<Buffer> outputBuffer,
                                                          PortIndex portIndex,
                                                          uint32_t bufferSize)
{
    std::shared_ptr<OmxCodecBuffer> omxBuffer = std::make_shared<OmxCodecBuffer>();
    omxBuffer->size = sizeof(OmxCodecBuffer);
    omxBuffer->version.s.nVersionMajor = verInfo_.compVersion.s.nVersionMajor;
    omxBuffer->allocLen = bufferSize;
    omxBuffer->fenceFd = -1; // check use -1 first with no window
    omxBuffer->pts = 0;
    omxBuffer->flag = 0;
    if (portIndex == PortIndex::PORT_INDEX_INPUT) {
        omxBuffer->bufferType = CODEC_BUFFER_TYPE_AVSHARE_MEM_FD;
        omxBuffer->bufferLen = sizeof(int);
        omxBuffer->type = READ_ONLY_TYPE;
        omxBuffer->buffer = (uint8_t *)(long long)sharedMem->GetShareMemoryFd();
        MEDIA_LOG_D("share memory fd: " PUBLIC_LOG_D32, sharedMem->GetShareMemoryFd());
    } else {
        omxBuffer->bufferType = CODEC_BUFFER_TYPE_HANDLE;
        BufferHandle* bufferHandle = std::static_pointer_cast<Plugin::SurfaceMemory>(outputBuffer->GetMemory())->
            GetSurfaceBuffer()->GetBufferHandle();
        if (!bufferHandle) {
            MEDIA_LOG_W("bufferHandle is null: " PUBLIC_LOG_P, bufferHandle);
        }
        omxBuffer->bufferLen =
            sizeof(BufferHandle) + (sizeof(int32_t) * (bufferHandle->reserveFds + bufferHandle->reserveInts));
        omxBuffer->buffer = (uint8_t*)bufferHandle;
    }
    return omxBuffer;
}

Status HdiAdapter::FreeBuffers()
{
    MEDIA_LOG_D("Free omx buffer begin");
    FALSE_RETURN_V_MSG_E(ChangeState(OMX_StateLoaded) == Status::OK, Status::ERROR_WRONG_STATE,
                         "Change omx state to loaded failed");
    auto iter = bufferInfoMap_.begin();
    while (iter != bufferInfoMap_.end()) {
        auto bufferInfo = iter->second;
        bufferInfo->omxBuffer->bufferLen = 0;
        iter = bufferInfoMap_.erase(iter);
        auto ret = codecComp_->FreeBuffer(codecComp_, (uint32_t)bufferInfo->portIndex, bufferInfo->omxBuffer.get());
        FALSE_RETURN_V_MSG_E(ret == HDF_SUCCESS, TransHdiRetVal2Status(ret),
                             "codec component free buffer failed, omxBufId: " PUBLIC_LOG_U32, iter->first);
    }
    FALSE_RETURN_V_MSG_E(WaitForState(OMX_StateLoaded) == Status::OK, Status::ERROR_WRONG_STATE,
                         "Wait omx state to loaded failed");
    freeOutBufferId_.Clear();
    freeInBufferId_.Clear();
    MEDIA_LOG_D("Free omx buffer end");
    return Status::OK;
}

void HdiAdapter::WaitForEvent(OMX_U32 cmd)
{
    MEDIA_LOG_D("WaitForEvent begin");
    OSAL::ScopedLock lock(stateSetMutex_);
    auto newCmd = static_cast<uint32_t>(cmd);
    MEDIA_LOG_D("Wait eventdone:" PUBLIC_LOG_D32 ", lastcmd:" PUBLIC_LOG_D32 ", cmd:" PUBLIC_LOG_U32,
                eventDone_, lastCmd_, cmd);
    stateSetCond_.Wait(lock, [this, &newCmd]() { return eventDone_ && (lastCmd_ == (int)newCmd || lastCmd_ == -1); });
    eventDone_ = false;
    MEDIA_LOG_D("WaitForEvent end");
}

Status HdiAdapter::WaitForState(OMX_STATETYPE state)
{
    MEDIA_LOG_D("WaitForState begin");
    WaitForEvent(OMX_CommandStateSet);
    if (curState_ != state) {
        MEDIA_LOG_E("Wait state failed");
        return Status::ERROR_WRONG_STATE;
    }
    MEDIA_LOG_D("WaitForState end");
    return Status::OK;
}

Status HdiAdapter::ChangeState(OMX_STATETYPE state)
{
    MEDIA_LOG_I("change state from " PUBLIC_LOG_S " to " PUBLIC_LOG_S,
                OmxStateToString(targetState_).c_str(), OmxStateToString(state).c_str());
    if (targetState_ != state && curState_ != state) {
        if (codecComp_ && codecComp_->SendCommand) {
            auto ret = codecComp_->SendCommand(codecComp_, OMX_CommandStateSet, state, nullptr, 0);
            FALSE_RETURN_V_MSG(ret == HDF_SUCCESS, Status::ERROR_UNKNOWN, "ChangeState failed");
            targetState_ = state;
        }
    }
    MEDIA_LOG_D("change state end");
    return Status::OK;
}

void HdiAdapter::HandelCmdCompleteEvent(OMX_U32 data1, OMX_U32 data2)
{
    MEDIA_LOG_D("HandelCmdCompleteEvent begin");
    switch (data1) {
        case OMX_CommandStateSet:
            HandelEventStateSet(data1, data2);
            break;
        case OMX_CommandFlush:
            HandelEventFlush(data1, data2);
            break;
        default:
            break;
    }
}

void HdiAdapter::HandelEventStateSet(OMX_U32 data1, OMX_U32 data2)
{
    MEDIA_LOG_D("HandelEventStateSet-callback begin");
    OSAL::ScopedLock lock(stateSetMutex_);
    lastCmd_ = static_cast<int>(data1);
    MEDIA_LOG_I("change curState from " PUBLIC_LOG_S " to " PUBLIC_LOG_S,
                OmxStateToString(curState_).c_str(), OmxStateToString(static_cast<OMX_STATETYPE>(data2)).c_str());
    curState_ = static_cast<OMX_STATETYPE>(data2);
    eventDone_ = true;
    stateSetCond_.NotifyAll();
}

void HdiAdapter::HandelEventFlush(OMX_U32 data1, OMX_U32 data2)
{
    MEDIA_LOG_D("HandelEventFlush begin, data1: " PUBLIC_LOG_U32 ", data2: " PUBLIC_LOG_U32, data1, data2);
    if (data2 == static_cast<uint32_t>(PortIndex::PORT_INDEX_OUTPUT)) {
        OSAL::ScopedLock l(flushMutex_);
        isFlushing_ = false;
        flushCond_.NotifyAll();
    }
}
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif