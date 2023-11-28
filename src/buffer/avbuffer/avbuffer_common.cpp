/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "buffer/avbuffer_common.h"
#include "surface_type.h" // foundation/graphic/graphic_2d/interfaces/surface/surface_type.h

namespace OHOS {
namespace Media {
AVBufferConfig::AVBufferConfig()
{
    this->surfaceBufferConfig = std::make_unique<BufferRequestConfig>();
}

AVBufferConfig::AVBufferConfig(const AVBufferConfig &rhs)
{
    if (&rhs == this) {
        return;
    }
    this->surfaceBufferConfig = std::make_unique<BufferRequestConfig>();
    *(this->surfaceBufferConfig) = *(rhs.surfaceBufferConfig);
    this->size = rhs.size;
    this->align = rhs.align;
    this->dmaFd = rhs.dmaFd;
    this->capacity = rhs.capacity;
    this->memoryFlag = rhs.memoryFlag;
    this->memoryType = rhs.memoryType;
}

AVBufferConfig::AVBufferConfig(AVBufferConfig &&rhs) noexcept
{
    this->surfaceBufferConfig = std::move(rhs.surfaceBufferConfig);
    this->size = rhs.size;
    this->align = rhs.align;
    this->dmaFd = rhs.dmaFd;
    this->capacity = rhs.capacity;
    this->memoryFlag = rhs.memoryFlag;
    this->memoryType = rhs.memoryType;
}

AVBufferConfig &AVBufferConfig::operator=(const AVBufferConfig &rhs)
{
    if (&rhs == this) {
        return *this;
    }
    *(this->surfaceBufferConfig) = *(rhs.surfaceBufferConfig);
    this->size = rhs.size;
    this->align = rhs.align;
    this->dmaFd = rhs.dmaFd;
    this->capacity = rhs.capacity;
    this->memoryFlag = rhs.memoryFlag;
    this->memoryType = rhs.memoryType;
    return *this;
}

AVBufferConfig &AVBufferConfig::operator=(AVBufferConfig &&rhs) noexcept
{
    if (&rhs == this) {
        return *this;
    }
    this->surfaceBufferConfig = std::move(rhs.surfaceBufferConfig);
    this->size = rhs.size;
    this->align = rhs.align;
    this->dmaFd = rhs.dmaFd;
    this->capacity = rhs.capacity;
    this->memoryFlag = rhs.memoryFlag;
    this->memoryType = rhs.memoryType;
    return *this;
}

bool AVBufferConfig::operator<=(const struct AVBufferConfig &rhs) const
{
    if (memoryType != rhs.memoryType) {
        return false;
    }
    int32_t configAllocSize = rhs.align ? (rhs.capacity + rhs.align - 1) : rhs.capacity;
    switch (memoryType) {
        case MemoryType::VIRTUAL_MEMORY:
            return size <= configAllocSize;
        case MemoryType::SHARED_MEMORY:
            return size <= configAllocSize &&
                   (memoryFlag == rhs.memoryFlag || rhs.memoryFlag == MemoryFlag::MEMORY_READ_WRITE);
        case MemoryType::HARDWARE_MEMORY:
            return size <= configAllocSize &&
                   (memoryFlag == rhs.memoryFlag || rhs.memoryFlag == MemoryFlag::MEMORY_READ_WRITE);
        case MemoryType::SURFACE_MEMORY:
            return (surfaceBufferConfig->width == rhs.surfaceBufferConfig->width) &&
                   (surfaceBufferConfig->height == rhs.surfaceBufferConfig->height) &&
                   (surfaceBufferConfig->strideAlignment == rhs.surfaceBufferConfig->strideAlignment) &&
                   (surfaceBufferConfig->format == rhs.surfaceBufferConfig->format) &&
                   (surfaceBufferConfig->usage == rhs.surfaceBufferConfig->usage) &&
                   (surfaceBufferConfig->transform == rhs.surfaceBufferConfig->transform) &&
                   (surfaceBufferConfig->colorGamut == rhs.surfaceBufferConfig->colorGamut); // ignore timeout
        default:
            return false;
    }
}

} // namespace Media
} // namespace OHOS