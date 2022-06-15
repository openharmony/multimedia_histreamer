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
#include <utility>
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
constexpr size_t BUFFER_QUEUE_SIZE = 21;

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
//            MEDIA_LOG_D("codec is not supported right now, cap.compName = " PUBLIC_LOG_S, cap.compName);
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
        MEDIA_LOG_D("i = " PUBLIC_LOG_D32 ", support pixfmts: " PUBLIC_LOG_D32, i, (int32_t)cap.port.video.supportPixFmts[i]);
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
//            MEDIA_LOG_D("DEBUG: capability type is : " PUBLIC_LOG_D32 ", compName: " PUBLIC_LOG_S,
//                         capList.get()[i].type, capList.get()[i].compName);
            continue;
        }
        CodecPluginDef definition;
        if (TranslateCapToPluginDef(capList[i], definition, packageName)) {
            definition.rank = 100;
            if (reg->AddPlugin(definition) != Status::OK) {
                MEDIA_LOG_E("add plugin " PUBLIC_LOG_S " failed", definition.name.c_str());
            }
//            MEDIA_LOG_D("add plugin " PUBLIC_LOG_S " success!", definition.name.c_str());
        } else {
//            MEDIA_LOG_W("failed to translate capability to plugin def, compName: " PUBLIC_LOG_S, capList[i].compName);
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
    MEDIA_LOG_D("EventHandler-callback begin");
    auto hdiAdapter = reinterpret_cast<HdiAdapter*>(info->appData);
    MEDIA_LOG_I("appData: " PUBLIC_LOG_D64 ", eEvent: " PUBLIC_LOG_D32 ", nData1: " PUBLIC_LOG_U32 ", nData2: " PUBLIC_LOG_U32,
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
    MEDIA_LOG_D("EmptyBufferDone-callback begin");
    auto hdiAdapter = reinterpret_cast<HdiAdapter*>(appData);
    OSAL::ScopedLock autoLock(hdiAdapter->lockInputBuffers_);
    hdiAdapter->unUsedInBufferId_.push_back(buffer->bufferId);
    MEDIA_LOG_D("EmptyBufferDone-callback end, bufferId: " PUBLIC_LOG_U32, buffer->bufferId);
    return HDF_SUCCESS;
}

int32_t HdiAdapter::FillBufferDone(CodecCallbackType* self, int64_t appData, const OmxCodecBuffer* omxBuffer)
{
    MEDIA_LOG_D("FillBufferDone-callback begin, bufferId: " PUBLIC_LOG_U32, omxBuffer->bufferId);
    auto hdiAdapter = reinterpret_cast<HdiAdapter*>(appData);
    auto iter = hdiAdapter->omxBuffers_.find(omxBuffer->bufferId);
    if ((iter == hdiAdapter->omxBuffers_.end()) || (iter->second == nullptr)) {
        MEDIA_LOG_D("iter == hdiAdapter->omxBuffers_.end() || iter->second == nullptr");
        return HDF_ERR_INVALID_PARAM;
    }
    auto bufferInfo = iter->second;
    Status status;
    {
        OSAL::ScopedLock l(hdiAdapter->lockOutputBuffers_);
        status = hdiAdapter->TransOmxBuffer2OutputBuffer(omxBuffer, bufferInfo);
    }
    if (status == Status::OK || status == Status::END_OF_STREAM) {
        hdiAdapter->NotifyOutputBufferDone(bufferInfo->outputBuffer);
    } else {
        OSAL::ScopedLock autoLock(hdiAdapter->lockOutputBuffers_);
        hdiAdapter->outBufferQ_.Push(bufferInfo->outputBuffer);
        MEDIA_LOG_D("TransOmxBuffer2OutputBuffer return error value, outBufferQ_.Push()");
    }
    // call fillthisbuffer again
    hdiAdapter->ReceiveFrameBuffer(bufferInfo, hdiAdapter);
    MEDIA_LOG_D("FillBufferDone-callback end");
    return HDF_SUCCESS;
}

void HdiAdapter::ReceiveFrameBuffer(std::shared_ptr<BufferInfo>& bufferInfo, HdiAdapter* hdiAdapter)
{
    MEDIA_LOG_D("ReceiveFrameBuffer begin");
    std::shared_ptr<Buffer> outputBuffer = nullptr;
    {
        OSAL::ScopedLock autoLock(hdiAdapter->lockOutputBuffers_);
        if (hdiAdapter->outBufferQ_.Size() == 0) {
            MEDIA_LOG_D("Error: outBufferQ_.size: " PUBLIC_LOG_U64, hdiAdapter->outBufferQ_.Size());
            return;
        }
        outputBuffer = hdiAdapter->outBufferQ_.Pop();
    }
    if (outputBuffer == nullptr || outputBuffer->IsEmpty()) {
        MEDIA_LOG_W("cannot fetch valid buffer to output");
        return;
    }
    auto frameMeta = outputBuffer->GetBufferMeta();
    if (frameMeta == nullptr || frameMeta->GetType() != BufferMetaType::VIDEO) {
        MEDIA_LOG_W("output buffer is not video buffer");
        return;
    }
    TransOutputBufToOmx(outputBuffer, bufferInfo);
    int32_t ret = HDF_SUCCESS;
    if (hdiAdapter->codecComp_ && hdiAdapter->codecComp_->FillThisBuffer) {
        ret = hdiAdapter->codecComp_->FillThisBuffer(codecComp_, bufferInfo->omxBuffer.get());
        if (ret != HDF_SUCCESS) {
            enum OMX_STATETYPE state;
            hdiAdapter->codecComp_->GetState(codecComp_, &state);
            MEDIA_LOG_E("FillThisBuffer error, ret: " PUBLIC_LOG_S ", current state: " PUBLIC_LOG_S,
                        TransHdfStatus2String(ret).c_str(), omxStateToString[state].c_str());
        }
    }
    MEDIA_LOG_D("ReceiveFrameBuffer end");
}

Status HdiAdapter::TransOmxBuffer2OutputBuffer(const OmxCodecBuffer* omxBuffer,
                                               std::shared_ptr<BufferInfo>& bufferInfo)
{
    MEDIA_LOG_D("TransOmxBuffer2OutputBuffer begin, omxBuffer->flag: " PUBLIC_LOG_U32, omxBuffer->flag);
    auto outputBuffer = bufferInfo->outputBuffer;
    if (outputBuffer == nullptr || outputBuffer->IsEmpty()) {
        MEDIA_LOG_W("cannot fetch valid buffer to output");
    }
    auto frameMeta = outputBuffer->GetBufferMeta();
    if (frameMeta == nullptr || frameMeta->GetType() != BufferMetaType::VIDEO) {
        MEDIA_LOG_W("output buffer is not video buffer");
    }
    Status ret = Status::OK;
    outputBuffer->flag = Translate2PluginFlagSet(omxBuffer->flag);
    outputBuffer->pts = omxBuffer->pts;
    MEDIA_LOG_D("TransOmxBuffer2OutputBuffer end, plugin.flag: " PUBLIC_LOG_U64 ", pts: " PUBLIC_LOG_D64, outputBuffer->flag, outputBuffer->pts);
    return ret;
}

void HdiAdapter::TransOutputBufToOmx(const std::shared_ptr<Plugin::Buffer>& outputBuffer,
                                     std::shared_ptr<BufferInfo>& bufferInfo)
{
    MEDIA_LOG_D("TransOutputBufToOmx begin");
    if (outBufferCnt_ <= 0 || outBufferSize_ <= 0) {
        MEDIA_LOG_E("outBufferCnt_ <= 0 || outBufferSize_ <= 0");
        return;
    }
    bufferInfo->omxBuffer->pts = 0;
    bufferInfo->omxBuffer->flag = 0;
    auto bufferHandle = std::static_pointer_cast<Plugin::SurfaceMemory>(outputBuffer->GetMemory())->GetSurfaceBuffer()->GetBufferHandle();
    bufferInfo->omxBuffer->bufferLen = sizeof(BufferHandle) + sizeof(int32_t) * (bufferInfo->bufferHandle->reserveFds +
                                                                  bufferInfo->bufferHandle->reserveInts);
    bufferInfo->omxBuffer->buffer = (uint8_t*)bufferHandle;
    bufferInfo->bufferHandle = bufferHandle;
    bufferInfo->outputBuffer = outputBuffer;
    MEDIA_LOG_D("TransOutputBufToOmx end");
}

// hdi adapter impl
HdiAdapter::HdiAdapter(std::string name)
    : CodecPlugin(std::forward<std::string>(name)), outBufferQ_("vdecPluginQueue", BUFFER_QUEUE_SIZE)
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

    int32_t ret = g_compManager->CreateComponent(&codecComp_, &componentId_, const_cast<char*>(compName.c_str()), (int64_t)this, codecCallback_);
    FALSE_RETURN_V_MSG(codecComp_ != nullptr, Status::ERROR_NULL_POINTER, "create component failed, retVal = " PUBLIC_LOG_D32, (int)ret);

    MEDIA_LOG_D("start codecComp_->GetState()：");
    ret = codecComp_->GetState(codecComp_, &curState_);
    MEDIA_LOG_D("curState_: " PUBLIC_LOG_D32, curState_);
    if (ret != HDF_SUCCESS || curState_ != OMX_StateLoaded) {
        MEDIA_LOG_E("get component status error: curStatus is " PUBLIC_LOG_D32, curState_);
        return Status::ERROR_UNKNOWN;
    }
    // 获取组件版本号
    (void)memset_s(&verInfo_, sizeof(verInfo_), 0, sizeof(verInfo_));
    ret = codecComp_->GetComponentVersion(codecComp_, &verInfo_);
    if (ret != HDF_SUCCESS) {
        MEDIA_LOG_E("get component version error: ret = " PUBLIC_LOG_D32, ret);
        return Status::ERROR_UNKNOWN;
    } else {
        MEDIA_LOG_D("verInfo_.compName = " PUBLIC_LOG_S "verInfo_.compVersion.nVersion = " PUBLIC_LOG_D32,
                    verInfo_.compName, (int)verInfo_.compVersion.nVersion);
    }
    outBufferQ_.SetActive(true);
    MEDIA_LOG_D("codec adapter init end");
    return Status::OK;
}

Plugin::Status HdiAdapter::Deinit()
{
    MEDIA_LOG_D("Deinit begin");
    FALSE_RETURN_V_MSG(g_compManager != nullptr, Status::ERROR_INVALID_PARAMETER, "g_compManager is nullptr");
    FALSE_RETURN_V_MSG(codecComp_ != nullptr, Status::ERROR_INVALID_PARAMETER, "codecComponent is nullptr");
    g_compManager->DestroyComponent(componentId_);
    codecComp_ = nullptr;
    curState_ = OMX_StateInvalid;
    outBufferQ_.SetActive(false);
    outBufferQ_.Clear();
    CodecCallbackTypeStubRelease(codecCallback_);
    codecCallback_ = nullptr;
    MEDIA_LOG_D("Deinit end");
    return Status::OK;
}

Plugin::Status HdiAdapter::Prepare()
{
    MEDIA_LOG_D("prepare begin");
    int32_t ret = HDF_SUCCESS;
    outBufferQ_.SetActive(true);

    UseOmxBuffers(); // 申请 omx buffer
    MEDIA_LOG_D("prepare end");
    return TranslateRets(ret);
}

Plugin::Status HdiAdapter::Reset()
{
    MEDIA_LOG_D("Reset begin");
    outBufferQ_.Clear();
    MEDIA_LOG_D("Reset end");
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
    outBufferQ_.SetActive(true);
    MEDIA_LOG_D("start end");
    return Status::OK;
}

Plugin::Status HdiAdapter::Stop()
{
    MEDIA_LOG_D("Stop begin");
//    if (curState_ == OMX_StateExecuting) {
//        FALSE_RETURN_V_MSG_E(ChangeState(OMX_StateIdle) == Status::OK, Status::ERROR_WRONG_STATE, "ChangeState failed");
//        FALSE_RETURN_V_MSG_E(WaitForState(OMX_StateIdle) == Status::OK, Status::ERROR_WRONG_STATE, "Wait failed");
//    }
//    if (curState_ == OMX_StateIdle) {
//        FALSE_RETURN_V_MSG_E(ChangeState(OMX_StateLoaded) == Status::OK, Status::ERROR_WRONG_STATE, "ChangeState failed");
//        FALSE_RETURN_V_MSG_E(WaitForState(OMX_StateIdle) == Status::OK, Status::ERROR_WRONG_STATE, "Wait failed");
//    }
    outBufferQ_.SetActive(false);
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
    return Status::OK;
}

Status HdiAdapter::QueueInputBuffer(const std::shared_ptr<Plugin::Buffer>& inputBuffer, int32_t timeoutMs)
{
    MEDIA_LOG_D("QueueInputBuffer begin");
    if (curState_ != OMX_StateExecuting) {
        MEDIA_LOG_W("QueueInputBuffer in wrong state: " PUBLIC_LOG_D32, (int)OMX_StateExecuting);
        return Status::ERROR_WRONG_STATE;
    }
    if (inputBuffer->IsEmpty() && !(inputBuffer->flag & BUFFER_FLAG_EOS)) {
        MEDIA_LOG_E("empty input buffer without eos flag");
        return Status::ERROR_INVALID_DATA;
    }
    /*/ ---
    char tmpStr[30] = {0};
    char* dstPtr = tmpStr;
    int len;
    const uint8_t* p = inputBuffer->GetMemory()->GetReadOnlyData();
    for (size_t i = 0; i < 5; i++) {
        len = snprintf_s(dstPtr, 3, 2, "%02x", p[i]); // max write 3 bytes, string len 2
        FALSE_LOG_MSG(len > 0 && len <= 2, "snprintf_s returned unexpected value " PUBLIC_LOG_D32, len); // max len 2
        dstPtr += len;
    }
    MEDIA_LOG_D("tmpStr: " PUBLIC_LOG_S, tmpStr);
    Pipeline::DumpBufferToFile("3gpFile", inputBuffer);
    if (inputBuffer->flag == 1) {
        MEDIA_LOG_D("inputBuffer->flag = 1, sleep 100000");
        OSAL::SleepFor(100000);
    }
    // --- end */
    auto bufferId = GetFreeBufferId();
    while (bufferId < 0) {
        MEDIA_LOG_E("bufferId < 0, SleepFor(1000)");
        OSAL::SleepFor(1000);
        bufferId = GetFreeBufferId();
    }
    auto iter = omxBuffers_.find(bufferId);
    if (iter == omxBuffers_.end()) {
        MEDIA_LOG_E("iter == omxBuffers_.end()");
        return Status::ERROR_UNKNOWN;
    }
    auto bufferInfo = iter->second;
    {
        OSAL::ScopedLock l(avMutex_);
        TransInputBuffer2OmxBuffer(inputBuffer, bufferInfo);
    }
    int32_t ret = HDF_ERR_INVALID_PARAM;
    if (codecComp_ && codecComp_->EmptyThisBuffer) {
        ret = codecComp_->EmptyThisBuffer(codecComp_, bufferInfo->omxBuffer.get());
        MEDIA_LOG_D("EmptyThisBuffer done, ret = " PUBLIC_LOG_S, TransHdfStatus2String(ret).c_str());
    }
    NotifyInputBufferDone(inputBuffer);
    MEDIA_LOG_D("QueueInputBuffer end");
    return TranslateRets(ret);
}

void HdiAdapter::TransInputBuffer2OmxBuffer(const std::shared_ptr<Plugin::Buffer>& pluginBuffer,
                                            std::shared_ptr<BufferInfo>& bufferInfo)
{
    MEDIA_LOG_D("TransBuffer2OmxBuffer begin");
    bufferInfo->omxBuffer->flag = Translate2omxFlagSet(pluginBuffer->flag);
    bufferInfo->omxBuffer->pts = pluginBuffer->pts;
    MEDIA_LOG_D("plugin flag: " PUBLIC_LOG_U32 ", pts: " PUBLIC_LOG_D64 ", memCount: " PUBLIC_LOG_U32,
                bufferInfo->omxBuffer->flag, bufferInfo->omxBuffer->pts, pluginBuffer->GetMemoryCount());
    auto mem = pluginBuffer->GetMemory();
    const uint8_t* addr = mem->GetReadOnlyData();
    size_t length = mem->GetSize();
    MEDIA_LOG_D("write: " PUBLIC_LOG_P " to share memory, size: " PUBLIC_LOG_D64, addr, length);
    (void)bufferInfo->avSharedPtr->Write(addr, length, 0);
    bufferInfo->omxBuffer->offset = 0;
    bufferInfo->omxBuffer->filledLen = length;
    MEDIA_LOG_D("TransBuffer2OmxBuffer end");
}

bool HdiAdapter::FillAllTheBuffer()
{
    MEDIA_LOG_D("FillAllTheBuffer begin");
    for (auto& bufferId : unUsedOutBufferId_) {
        auto iter = omxBuffers_.find(bufferId);
        if (iter != omxBuffers_.end()) {
            auto bufferInfo = iter->second;
            auto omxBuffer = bufferInfo->omxBuffer.get();
            if (bufferInfo->bufferHandle != nullptr) {
                omxBuffer->buffer = (uint8_t *)bufferInfo->bufferHandle;
                omxBuffer->bufferLen = sizeof(BufferHandle) + sizeof(int32_t) * (bufferInfo->bufferHandle->reserveFds +
                                                                              bufferInfo->bufferHandle->reserveInts);
            } else {
                MEDIA_LOG_D("bufferInfo->bufferHandle == nullptr");
            }
            auto ret = codecComp_->FillThisBuffer(codecComp_, omxBuffer);
            if (ret != HDF_SUCCESS) {
                MEDIA_LOG_E("FillThisBuffer error");
                return false;
            }
        } else {
            MEDIA_LOG_D("iter == omxBuffers_.end()");
        }
    }
    MEDIA_LOG_D("FillAllTheBuffer end");
    return true;
}

Status HdiAdapter::QueueOutputBuffer(const std::shared_ptr<Plugin::Buffer>& outputBuffers, int32_t timeoutMs)
{
    MEDIA_LOG_D("QueueOutputBuffer begin");
    outBufferQ_.Push(outputBuffers);
    return Status::OK;
}

Status HdiAdapter::Flush()
{
    MEDIA_LOG_D("Flush begin");
    int32_t ret = HDF_ERR_INVALID_PARAM;
    if (codecComp_ && codecComp_->SendCommand) {
        ret = codecComp_->SendCommand(codecComp_, OMX_CommandFlush, (uint32_t)PortIndex::PORT_INDEX_INPUT, nullptr, 0);
    }
    outBufferQ_.SetActive(true);
    MEDIA_LOG_D("Flush end, ret: " PUBLIC_LOG_S, TransHdfStatus2String(ret).c_str());
    return TranslateRets(ret);
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
    auto ret = codecComp_->GetParameter(codecComp_, OMX_IndexParamVideoPortFormat, (int8_t *)&videoFormat, sizeof(videoFormat));
    if (ret != HDF_SUCCESS) {
        MEDIA_LOG_E("GetParameter OMX_IndexParamVideoPortFormat failed, ret = " PUBLIC_LOG_D32, (int)ret);
        return;
    }
    MEDIA_LOG_I("set Format PortIndex::PORT_INDEX_INPUT eCompressionFormat = " PUBLIC_LOG_D32 ", eColorFormat = " PUBLIC_LOG_D32,
                videoFormat.eCompressionFormat, videoFormat.eColorFormat);
    videoFormat.xFramerate = 30 << 16;  // 30fps,Q16 format
    videoFormat.eCompressionFormat = OMX_VIDEO_CodingAVC;  // H264
    ret = codecComp_->SetParameter(codecComp_, OMX_IndexParamVideoPortFormat, (int8_t *)&videoFormat, sizeof(videoFormat));
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
    MEDIA_LOG_I("PortIndex::PORT_INDEX_OUTPUT eCompressionFormat = " PUBLIC_LOG_D32 ", eColorFormat = " PUBLIC_LOG_D32,
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
    MEDIA_LOG_D("CheckAndUseBufferHandle begin");
    int32_t ret = HDF_SUCCESS;
    UseBufferType type;
    InitParamInOhos(type);
    type.portIndex = (uint32_t)PortIndex::PORT_INDEX_OUTPUT;
    type.bufferType = CODEC_BUFFER_TYPE_HANDLE;
    ret = codecComp_->SetParameter(codecComp_, OMX_IndexParamUseBufferType, (int8_t *)&type, sizeof(type));
    if (ret != HDF_SUCCESS) {
        MEDIA_LOG_E("PORT_INDEX_OUTPUT:OMX_IndexParamUseBufferType:bufferTypes: " PUBLIC_LOG_D32 ", ret: " PUBLIC_LOG_S,
                    type.bufferType, TransHdfStatus2String(ret).c_str());
    }
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
    if (ret != HDF_SUCCESS) {
        MEDIA_LOG_E("failed to GetParameter with OMX_IndexParamPortDefinition: portIndex: " PUBLIC_LOG_D32, static_cast<int>(portIndex));
    }

    bufferSize = param.nBufferSize;
    bufferCount = param.nBufferCountActual;
    portEnable = param.bEnabled;
    bufferCountMin = param.nBufferCountMin;
    MEDIA_LOG_I("buffer size: " PUBLIC_LOG_U32 ", buffer count: " PUBLIC_LOG_U32 ", bufferCountMin: " PUBLIC_LOG_U32 ", portEnable: " PUBLIC_LOG_D32,
                bufferSize, bufferCount, bufferCountMin, static_cast<int>(portEnable));
    {
        OMX_PARAM_BUFFERSUPPLIERTYPE param;
        InitParam(param);
        param.nPortIndex = (uint32_t)portIndex;
        ret = codecComp_->GetParameter(codecComp_, OMX_IndexParamCompBufferSupplier, (int8_t *)&param, sizeof(param));
        MEDIA_LOG_D("param.eBufferSupplier: " PUBLIC_LOG_D32 ", err: " PUBLIC_LOG_D32, (int)param.eBufferSupplier, (int)ret);
    }
    if (portIndex == PortIndex::PORT_INDEX_OUTPUT) {
        outBufferSize_ = bufferSize;
        outBufferCnt_ = bufferCount;
        MEDIA_LOG_D("outBufferCnt_: " PUBLIC_LOG_D32 ", outBufferSize_: " PUBLIC_LOG_D32, outBufferCnt_, outBufferSize_);
    } else {
        inBufferSize_ = bufferSize;
        inBufferCnt_ = bufferCount;
        MEDIA_LOG_D("inBufferCnt_: " PUBLIC_LOG_D32 ", inBufferSize_: " PUBLIC_LOG_D32, inBufferCnt_, inBufferSize_);
    }
    // set port enable
    if (!portEnable) {
        ret = codecComp_->SendCommand(codecComp_, OMX_CommandPortEnable, (uint32_t)portIndex, NULL, 0);
        if (ret != HDF_SUCCESS) {
            MEDIA_LOG_E("SendCommand OMX_CommandPortEnable failed, portIndex: " PUBLIC_LOG_D32, (int)portIndex);
        }
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
        bufferInfo->portIndex = portIndex;
        bufferInfo->bufferHandle = nullptr;
        omxBuffers_.emplace(std::make_pair(omxBuffer->bufferId, bufferInfo));
        unUsedInBufferId_.push_back(omxBuffer->bufferId);
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
    int32_t err = HDF_SUCCESS;
    for (uint32_t i = 0; i < outBufferCnt_; i++) { // buffer count: 21, bufferCountMin: 17
        std::shared_ptr<OmxCodecBuffer> omxBuffer = std::make_shared<OmxCodecBuffer>();
        std::shared_ptr<Buffer> outputBuffer = outBufferQ_.Pop();
        omxBuffer->size = sizeof(OmxCodecBuffer);
        omxBuffer->version.s.nVersionMajor = verInfo_.compVersion.s.nVersionMajor;
        omxBuffer->bufferType = CODEC_BUFFER_TYPE_HANDLE;
        BufferHandle* bufferHandle = std::static_pointer_cast<Plugin::SurfaceMemory>(outputBuffer->GetMemory())->GetSurfaceBuffer()->GetBufferHandle();
        if (bufferHandle) {
            MEDIA_LOG_D("bufferHandle: " PUBLIC_LOG_P, bufferHandle);
        } else {
            MEDIA_LOG_D("bufferHandle is null: " PUBLIC_LOG_P, bufferHandle);
        }
        size_t handleSize =
                sizeof(BufferHandle) + (sizeof(int32_t) * (bufferHandle->reserveFds + bufferHandle->reserveInts));
        MEDIA_LOG_D("handle size: " PUBLIC_LOG_U64, handleSize);
        omxBuffer->bufferLen = handleSize;
        omxBuffer->buffer = (uint8_t*)bufferHandle;
        omxBuffer->allocLen = bufferSize;
        omxBuffer->fenceFd = -1; // check use -1 first with no window
        omxBuffer->pts = 0;
        omxBuffer->flag = 0;
        err = codecComp_->UseBuffer(codecComp_, (uint32_t)PortIndex::PORT_INDEX_OUTPUT, omxBuffer.get());
        if (err != HDF_SUCCESS) {
            MEDIA_LOG_E("failed to UseBuffer with output port, bufferId: " PUBLIC_LOG_U32 ",i: " PUBLIC_LOG_U32, omxBuffer->bufferId, i);
            return err;
        }
        omxBuffer->bufferLen = 0;
        MEDIA_LOG_I("UseBuffer returned out bufferID: " PUBLIC_LOG_U32, omxBuffer->bufferId);

        std::shared_ptr<BufferInfo> bufferInfo = std::make_shared<BufferInfo>();
        bufferInfo->omxBuffer = omxBuffer;
        bufferInfo->bufferHandle = bufferHandle;
        bufferInfo->portIndex = PortIndex::PORT_INDEX_OUTPUT;
        bufferInfo->avSharedPtr = nullptr;
        bufferInfo->outputBuffer = outputBuffer;
        omxBuffers_.emplace(std::make_pair(omxBuffer->bufferId, bufferInfo));
        unUsedOutBufferId_.push_back(omxBuffer->bufferId);
    }
    MEDIA_LOG_D("UseBufferOnOutputPort end, outBufferQ_.size: " PUBLIC_LOG_U64, outBufferQ_.Size());
    return HDF_SUCCESS;
}

int HdiAdapter::GetFreeBufferId()
{
    int bufferID = -1;
    OSAL::ScopedLock autoLock(lockInputBuffers_);
    size_t nSize = unUsedInBufferId_.size();
    if (nSize > 0) {
        bufferID = unUsedInBufferId_.front();
        unUsedInBufferId_.pop_front();
    }
    MEDIA_LOG_D("GetFreeBufferId end, bufferId: " PUBLIC_LOG_D32, bufferID);
    return bufferID;
}

void HdiAdapter::WaitForEvent(OMX_U32 cmd)
{
    MEDIA_LOG_D("WaitForEvent begin");
    OSAL::ScopedLock lock(mutex_);
    uint32_t newCmd = static_cast<uint32_t>(cmd);
    MEDIA_LOG_D("Wait eventdone:" PUBLIC_LOG_D32 ", lastcmd:" PUBLIC_LOG_D32 ", cmd:" PUBLIC_LOG_U32, eventDone_, lastCmd_, cmd);
    cond_.Wait(lock, [this, &newCmd]() { return eventDone_ && (lastCmd_ == newCmd || lastCmd_ == -1); });
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
//    cond_.NotifyAll();
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