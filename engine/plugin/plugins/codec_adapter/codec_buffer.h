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

#ifndef HISTREAMER_PLUGIN_CODEC_BUFFER_H
#define HISTREAMER_PLUGIN_CODEC_BUFFER_H

#include "codec_component_type.h"
#include "codec_omx_ext.h"
#include "plugin/common/plugin_buffer.h"
#include "plugin/common/share_memory.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace CodecAdapter {
class CodecBuffer {
public:
    CodecBuffer(std::shared_ptr<Buffer>& buffer, CompVerInfo& verInfo,
        bool isInput, uint32_t bufferSize, MemoryType bufMemType);

    ~CodecBuffer() = default;

    std::shared_ptr<OmxCodecBuffer> GetOmxBuffer();

    uint32_t GetBufferId() const;

    Status Copy(const std::shared_ptr<Plugin::Buffer>& pluginBuffer);

    Status Rebind(const std::shared_ptr<Plugin::Buffer>& pluginBuffer); // 重新申请内存时， CodecBuffer都要重新创建

    Status Unbind(std::shared_ptr<Plugin::Buffer>& buffer, const OmxCodecBuffer* omxBuffer);

private:
    void Init(bool isInput, uint32_t bufferSize, MemoryType bufMemType);

    std::shared_ptr<Buffer> buffer_;
    CompVerInfo verInfo_ {};
    std::shared_ptr<Memory> memory_;
    std::shared_ptr<OmxCodecBuffer> omxBuffer_;
};
} // namespace CodecAdapter
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PLUGIN_CODEC_BUFFER_H
#endif