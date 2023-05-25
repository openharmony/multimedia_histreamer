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

#ifndef HISTREAMER_PLUGIN_CODEC_BUFFER_POOL_H
#define HISTREAMER_PLUGIN_CODEC_BUFFER_POOL_H

#include "codec_buffer.h"
#include "codec_component_if.h"
#include "codec_component_type.h"
#include "foundation/osal/thread/mutex.h"
#include "foundation/utils/blocking_queue.h"
#include "plugin/common/plugin_types.h"
#include "plugin/common/share_allocator.h"
#include "plugin/common/share_memory.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace CodecAdapter {
class CodecBufferPool {
public:
    CodecBufferPool(CodecComponentType* compType, CompVerInfo& verInfo, uint32_t portIndex, uint32_t bufferCnt);

    ~CodecBufferPool() = default;

    /**
     * Init codec buffer and create buffer map
     * Encoder:
     * inBuf is surfaceBuf and allocated dynamic, outBuf is sharedBuf (READ_WRITE_TYPE)
     * Decoder:
     * inBuf is sharedBuf (READ_ONLY_TYPE),  outBuf is surfaceBuf and pre-allocation
     */
    Status UseBuffers(OHOS::Media::BlockingQueue<std::shared_ptr<Buffer>>& bufQue, MemoryType bufMemType,
                      bool isInput, uint32_t bufferSize);

    Status FreeBuffers(); // 释放所有buffer

    uint32_t EmptyBufferCount();

    Status UseBufferDone(uint32_t bufId); // 根据该bufferId，重置omxBuffer对应的CodecBuffer

    std::shared_ptr<CodecBuffer> GetBuffer(int32_t bufferId = -1);

private:
    Status ConfigBufType(const MemoryType& bufMemType, bool isInput);

private:
    OSAL::Mutex mutex_;
    CodecComponentType* codecComp_ {nullptr};
    CompVerInfo verInfo_;

    uint32_t portIndex_;
    OHOS::Media::BlockingQueue<uint32_t> freeBufferId_;
    std::map<uint32_t, std::shared_ptr<CodecBuffer>> codecBufMap_;  // key is buffer id
};
} // namespace CodecAdapter
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PLUGIN_CODEC_BUFFER_POOL_H
#endif