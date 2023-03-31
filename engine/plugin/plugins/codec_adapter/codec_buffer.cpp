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

#define HST_LOG_TAG "CodecBuffer"

#include "codec_buffer.h"
#include "codec_utils.h"
#include "plugin/common/surface_memory.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace CodecAdapter {
CodecBuffer::CodecBuffer(std::shared_ptr<Buffer>& buffer, CompVerInfo& verInfo, bool isInput)
    : buffer_(buffer), verInfo_(verInfo)
{
    Init(isInput);
}

void CodecBuffer::Init(bool isInput)
{
    MEDIA_LOG_DD("CodecBuffer Init Start");
    omxBuffer_ = std::make_shared<OmxCodecBuffer>();
    omxBuffer_->size = sizeof(OmxCodecBuffer);
    omxBuffer_->version.s.nVersionMajor = verInfo_.compVersion.s.nVersionMajor;
    memory_ = buffer_->GetMemory();
    omxBuffer_->allocLen = memory_->GetCapacity();
    omxBuffer_->fenceFd = -1; // check use -1 first with no window
    omxBuffer_->pts = 0;
    omxBuffer_->flag = 0;
    omxBuffer_->bufferType = GetOmxBufferType(memory_->GetMemoryType(), isInput);
    switch (memory_->GetMemoryType()) {
        case MemoryType::SURFACE_BUFFER: {
            if (isInput) {
                omxBuffer_->bufferLen = 0;
                omxBuffer_->buffer = nullptr;
            } else {
                BufferHandle* bufferHandle =
                    std::static_pointer_cast<Plugin::SurfaceMemory>(memory_)->GetSurfaceBuffer()->GetBufferHandle();
                FALSE_LOG_MSG(bufferHandle != nullptr, "bufferHandle is null");
                omxBuffer_->bufferLen =
                    sizeof(BufferHandle) + (sizeof(int32_t) * (bufferHandle->reserveFds + bufferHandle->reserveInts));
                omxBuffer_->buffer = (uint8_t*)bufferHandle;
            }
            break;
        }
        case MemoryType::SHARE_MEMORY: {
            omxBuffer_->bufferLen = sizeof(int);
            omxBuffer_->type = isInput ? READ_ONLY_TYPE : READ_WRITE_TYPE;
            omxBuffer_->buffer =
                (uint8_t *)(long long)std::static_pointer_cast<Plugin::ShareMemory>(memory_)->GetShareMemoryFd();
            MEDIA_LOG_D("share memory fd: " PUBLIC_LOG_D32,
                        std::static_pointer_cast<Plugin::ShareMemory>(memory_)->GetShareMemoryFd());
            break;
        }
        default:
            MEDIA_LOG_W("UnKnow MemoryType: " PUBLIC_LOG_D32, (int)memory_->GetMemoryType());
            break;
    }
}

std::shared_ptr<OmxCodecBuffer> CodecBuffer::GetOmxBuffer()
{
    return omxBuffer_;
}

uint32_t CodecBuffer::GetBufferId()
{
    return omxBuffer_->bufferId;
}

Status CodecBuffer::Copy(const std::shared_ptr<Plugin::Buffer>& pluginBuffer)
{
    omxBuffer_->flag = Translate2omxFlagSet(pluginBuffer->flag);
    omxBuffer_->pts = pluginBuffer->pts;
    MEDIA_LOG_DD("plugin flag: " PUBLIC_LOG_U32 ", pts: " PUBLIC_LOG_D64, omxBuffer_->flag, omxBuffer_->pts);
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
    memory_->Write(memAddr, mem->GetSize(), 0);
    omxBuffer_->offset = 0;
    omxBuffer_->filledLen = mem->GetSize();
    MEDIA_LOG_DD("CopyBuffer end, bufferId: " PUBLIC_LOG_U32, bufferInfo->omxBuffer->bufferId);
    return Status::OK;
}

Status CodecBuffer::Rebind(const std::shared_ptr<Plugin::Buffer>& pluginBuffer)
{
    MEDIA_LOG_DD("Rebind Start");
    omxBuffer_->flag = Translate2omxFlagSet(pluginBuffer->flag);
    omxBuffer_->pts = pluginBuffer->pts;
    MEDIA_LOG_DD("plugin flag: " PUBLIC_LOG_U32 ", pts: " PUBLIC_LOG_D64, omxBuffer_->flag, omxBuffer_->pts);
    switch (pluginBuffer->GetMemory()->GetMemoryType()) {
        case MemoryType::SURFACE_BUFFER: {
            auto outMem = std::static_pointer_cast<Plugin::SurfaceMemory>(pluginBuffer->GetMemory());
            FALSE_RETURN_V_MSG_E(outMem != nullptr, Status::ERROR_INVALID_DATA, "GetMemory failed");
            auto surfaceBuf = outMem->GetSurfaceBuffer();
            FALSE_RETURN_V_MSG_E(surfaceBuf != nullptr, Status::ERROR_INVALID_DATA, "GetSurfaceBuffer failed");
            BufferHandle* bufferHandle = surfaceBuf->GetBufferHandle();
            FALSE_RETURN_V_MSG_E(bufferHandle != nullptr, Status::ERROR_INVALID_DATA, "GetBufferHandle failed");
            omxBuffer_->bufferLen =
                sizeof(BufferHandle) + sizeof(int32_t) * (bufferHandle->reserveFds + bufferHandle->reserveInts);
            omxBuffer_->buffer = (uint8_t*)bufferHandle;
            break;
        }
        case MemoryType::SHARE_MEMORY:
            omxBuffer_->bufferLen = sizeof(int);
            omxBuffer_->buffer = (uint8_t *)(long long)
                std::static_pointer_cast<Plugin::ShareMemory>(pluginBuffer->GetMemory())->GetShareMemoryFd();
            break;
        case MemoryType::VIRTUAL_ADDR:
            MEDIA_LOG_E("Rebind pluginBuffer failed, MemoryType is VIRTUAL_ADDR");
            break;
        default:
            MEDIA_LOG_E("MemoryType invalid!");
            break;
    }
    // 这里buffer需要保存一下，为了方便往下一节点传数据，通过GetBuffer()获取
    buffer_ = pluginBuffer;
    MEDIA_LOG_D("Rebind end, omxBufferId: " PUBLIC_LOG_U32, omxBuffer_->bufferId);
    return Status::OK;
}

Status CodecBuffer::Unbind(std::shared_ptr<Plugin::Buffer>& buffer, const OmxCodecBuffer* omxBuffer)
{
    // 因为Rebind()里面用buffer_保存了PluginBuf，所以这里的buffer_需要主动释放，减少智能指针的引用计数
    // decoder 时，PluginBuf 的真正释放时机应该是在sink节点，该数据送显后才能释放
    buffer = buffer_;
    buffer->flag = Translate2PluginFlagSet(omxBuffer->flag);
    buffer->pts = omxBuffer->pts;
    buffer_ = nullptr;
    return Status::OK;
}
} // namespace CodecAdapter
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif