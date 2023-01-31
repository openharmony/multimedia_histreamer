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

#ifndef HISTREAMER_PLUGIN_CODEC_BUFFER_POOL_H
#define HISTREAMER_PLUGIN_CODEC_BUFFER_POOL_H

#include "codec_component_type.h"
#include "common/plugin_types.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace CodecAdapter {
class CodecBufferPool {
public:
    CodecBufferPool() = default;
    ~CodecBufferPool() = default;

    Status ConfigOutPortBufType();
    Status AllocBuffer(OmxCodecBuffer& buffer);
    Status FreeBuffer();
    Status UseBuffer(uint32_t size, uint32_t count); //fill buffer into hdi

private:
//    shared_ptr<BufferInfoMap> BufMap_;
//    BlockingQueue<uint32_t> freeBufferId_;
};
} // namespace CodecAdapter
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PLUGIN_CODEC_BUFFER_POOL_H
#endif