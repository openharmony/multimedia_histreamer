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

#define HST_LOG_TAG "CodecBufferPool"

#include "codec_buffer_pool.h"
#include "codec_utils.h"
#include "foundation/log.h"
#include "hdf_base.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace CodecAdapter {
CodecBufferPool::CodecBufferPool(CodecComponentType* compType, CompVerInfo& verInfo,
    uint32_t portIndex, uint32_t bufferCnt)
    : codecComp_(compType), verInfo_(verInfo), portIndex_(portIndex), freeBufferId_("hdiFreeBufferId", bufferCnt)
{
}

// 当前实现仅仅支持异步模式，hdi的工作模式决定了仅支持异步，要求提前将所有 out buffer 配置好
Status CodecBufferPool::UseBuffers(OHOS::Media::BlockingQueue<std::shared_ptr<Buffer>>& bufQue, MemoryType bufMemType,
                                   bool isInput, uint32_t bufferSize)
{
    MEDIA_LOG_D("UseBuffers begin");
    FALSE_RETURN_V_MSG_E(ConfigBufType(bufMemType, isInput) == Status::OK, Status::ERROR_INVALID_DATA,
                         "ConfigBufType failed");
    auto count = bufQue.Size();
    for (uint32_t i = 0; i < count; i++) {
        auto pluginBuffer = bufQue.Pop();
        auto codecBuffer = std::make_shared<CodecBuffer>(pluginBuffer, verInfo_, isInput, bufferSize, bufMemType);
        FALSE_RETURN_V_MSG(codecBuffer != nullptr, Status::ERROR_NULL_POINTER, "Create codec buffer failed");
        auto err = codecComp_->UseBuffer(codecComp_, portIndex_, codecBuffer->GetOmxBuffer().get());
        if (err != HDF_SUCCESS) {
            MEDIA_LOG_E("failed to UseBuffer");
            return Status::ERROR_INVALID_DATA;
        }
        MEDIA_LOG_D("UseBuffer returned bufferID: " PUBLIC_LOG_U32 ", PortIndex: " PUBLIC_LOG_U32,
                    codecBuffer->GetBufferId(), portIndex_);
        codecBufMap_.emplace(codecBuffer->GetBufferId(), codecBuffer);
        freeBufferId_.Push(codecBuffer->GetBufferId());
    }
    MEDIA_LOG_D("UseBuffers end, freeBufId.size: " PUBLIC_LOG_ZU ", portIndex: " PUBLIC_LOG_U32,
                freeBufferId_.Size(), portIndex_);
    return Status::OK;
}

Status CodecBufferPool::FreeBuffers()
{
    MEDIA_LOG_D("FreeBuffers begin");
    for (auto& codecBuf : codecBufMap_) {
        auto& codecBuffer = codecBuf.second;
        FALSE_RETURN_V_MSG_E(codecComp_ != nullptr, Status::ERROR_NULL_POINTER, "Codec component is null.");
        FALSE_RETURN_V_MSG_E(codecBuffer != nullptr, Status::ERROR_NULL_POINTER, "Codec buffer is null.");
        auto omxBuffer = codecBuffer->GetOmxBuffer().get();
        FALSE_RETURN_V_MSG_E(omxBuffer != nullptr, Status::ERROR_NULL_POINTER, "Omx buffer is null.");
        auto ret = codecComp_->FreeBuffer(codecComp_, portIndex_, omxBuffer);
        FALSE_RETURN_V_MSG_E(ret == HDF_SUCCESS, TransHdiRetVal2Status(ret),
            "codec component free buffer failed, omxBufId: " PUBLIC_LOG_U32, codecBuffer->GetBufferId());
    }
    codecBufMap_.clear();
    freeBufferId_.Clear();
    MEDIA_LOG_D("FreeBuffers end");
    return Status::OK;
}

Status CodecBufferPool::ConfigBufType(const MemoryType& bufMemType, bool isInput)
{
    if (bufMemType == MemoryType::SHARE_MEMORY) {
        return Status::OK;
    }
    UseBufferType bufType{};
    InitHdiParam(bufType, verInfo_);
    bufType.portIndex = portIndex_;
    bufType.bufferType = GetOmxBufferType(bufMemType, isInput);
    auto ret = codecComp_->SetParameter(codecComp_, OMX_IndexParamUseBufferType, (int8_t *)&bufType, sizeof(bufType));
    FALSE_LOG_MSG(ret == HDF_SUCCESS, "isInput: " PUBLIC_LOG_D32 ", bufferTypes: " PUBLIC_LOG_D32 ", ret: "
            PUBLIC_LOG_S, static_cast<int32_t>(isInput), bufType.bufferType, OmxErrorType2String(ret).c_str());
    MEDIA_LOG_D("ConfigOutPortBufType end");
    return TransHdiRetVal2Status(ret);
}

uint32_t CodecBufferPool::EmptyBufferCount()
{
    OSAL::ScopedLock lock(mutex_);
    return freeBufferId_.Size();
}

Status CodecBufferPool::UseBufferDone(uint32_t bufId)
{
    OSAL::ScopedLock lock(mutex_);
    freeBufferId_.Push(bufId);
    return Status::OK;
}

std::shared_ptr<CodecBuffer> CodecBufferPool::GetBuffer(int32_t bufferId)
{
    OSAL::ScopedLock lock(mutex_);
    auto bufId = bufferId >= 0 ? static_cast<uint32_t>(bufferId) : freeBufferId_.Pop(1);
    return codecBufMap_.count(bufId) ? codecBufMap_[bufId] : nullptr;
}
} // namespace CodecAdapter
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif