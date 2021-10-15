/*
 * Copyright (c) 2021-2021 Huawei Device Co., Ltd.
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

#ifndef HISTREAMER_HDI_ADAPTER_RING_BUFFER_H
#define HISTREAMER_HDI_ADAPTER_RING_BUFFER_H

#include <cstdint>
#include <memory>

#include "foundation/osal/thread/condition_variable.h"
#include "foundation/osal/thread/mutex.h"

namespace OHOS {
namespace Media {
namespace Plugin {
class Buffer;
}
namespace HosLitePlugin {
class RingBuffer {
public:
    explicit RingBuffer(size_t bufferSize) : bufferSize_(bufferSize) {}

    ~RingBuffer() = default;

    bool Init();

    std::shared_ptr<uint8_t> ReadBufferWithoutAdvance(size_t readSize, size_t& outSize);

    void Advance(size_t size);

    void WriteBuffer(const std::shared_ptr<OHOS::Media::Plugin::Buffer>& inputInfo);

    void SetActive(bool active);

    void Clear();

private:
    const size_t bufferSize_;
    std::unique_ptr<uint8_t[]> buffer_ {};
    size_t head_ {0}; // head
    size_t tail_ {0}; // tail
    OHOS::Media::OSAL::Mutex writeMutex_ {};
    OHOS::Media::OSAL::ConditionVariable writeCondition_ {};
    bool isActive_ {true};
};
} // namespace HosLitePlugin
} // namespace Media
} // namespace OHOS

#endif // MEDIA_PIPELINE_RING_BUFFER_H
