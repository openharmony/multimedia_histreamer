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

#include "plugin/common/plugin_buffer.h"
#include "plugin/common/share_memory.h"
#include "plugin/common/surface_memory.h"

namespace OHOS {
namespace Media {
namespace Plugin {
Memory::Memory(size_t capacity, std::shared_ptr<uint8_t> bufData, size_t align, MemoryType type)
    : memoryType(type), capacity(capacity), alignment(align),
      offset(0), size(0), allocator(nullptr), addr(std::move(bufData))
{
}

Memory::Memory(size_t capacity, std::shared_ptr<Allocator> allocator, size_t align, MemoryType type, bool allocMem)
    : memoryType(type), capacity(capacity), alignment(align), offset(0),
      size(0), allocator(std::move(allocator)), addr(nullptr)
{
    if (!allocMem) { // SurfaceMemory alloc mem in subclass
        return;
    }
    size_t allocSize = align ? (capacity + align - 1) : capacity;
    if (this->allocator) {
        addr = std::shared_ptr<uint8_t>(static_cast<uint8_t*>(this->allocator->Alloc(allocSize)),
                                        [this](uint8_t* ptr) { this->allocator->Free(static_cast<void*>(ptr)); });
    } else {
        addr = std::shared_ptr<uint8_t>(new uint8_t[allocSize], std::default_delete<uint8_t[]>());
    }
    offset = static_cast<size_t>(AlignUp(reinterpret_cast<uintptr_t>(addr.get()), static_cast<uintptr_t>(align)) -
        reinterpret_cast<uintptr_t>(addr.get()));
}

size_t Memory::GetCapacity()
{
    return capacity;
}

void Memory::Reset()
{
    this->size = 0;
}

size_t Memory::Write(const uint8_t* in, size_t writeSize, size_t position)
{
    size_t start = 0;
    if (position == INVALID_POSITION) {
        start = size;
    } else {
        start = std::min(position, capacity);
    }
    size_t length = std::min(writeSize, capacity - start);
    if (memcpy_s(GetRealAddr() + start, length, in, length) != EOK) {
        return 0;
    }
    size = start + length;
    return length;
}

size_t Memory::Read(uint8_t* out, size_t readSize, size_t position)
{
    size_t start = 0;
    size_t maxLength = size;
    if (position != INVALID_POSITION) {
        start = std::min(position, size);
        maxLength = size - start;
    }
    size_t length = std::min(readSize, maxLength);
    if (memcpy_s(out, length, GetRealAddr() + start, length) != EOK) {
        return 0;
    }
    return length;
}

const uint8_t* Memory::GetReadOnlyData(size_t position)
{
    if (position > capacity) {
        return nullptr;
    }
    return GetRealAddr() + position;
}

uint8_t* Memory::GetWritableAddr(size_t estimatedWriteSize, size_t position)
{
    if (position + estimatedWriteSize > capacity) {
        return nullptr;
    }
    uint8_t* ptr = GetRealAddr() + position;
    size = (estimatedWriteSize + position);
    return ptr;
}

void Memory::UpdateDataSize(size_t realWriteSize, size_t position)
{
    if (position + realWriteSize > capacity) {
        return;
    }
    size = (realWriteSize + position);
}

size_t Memory::GetSize()
{
    return size;
}

uint8_t* Memory::GetRealAddr() const
{
    return addr.get() + offset;
}

MemoryType Memory::GetMemoryType()
{
    return memoryType;
}

BufferMeta::BufferMeta(BufferMetaType type) : type_(type), tags_(std::make_shared<Meta>())
{
}

ValueType BufferMeta::GetMeta(Tag tag)
{
    if (tags_) {
        return (*tags_)[tag];
    }
    return {};
}

void BufferMeta::SetMeta(Tag tag, ValueType value)
{
    (*tags_)[tag] = value;
}

BufferMetaType BufferMeta::GetType() const
{
    return type_;
}

bool BufferMeta::IsExist(Tag tag)
{
    return tags_->Find(tag) != tags_->end();
}

void BufferMeta::Update(const BufferMeta& bufferMeta)
{
    type_ = bufferMeta.GetType();
    *tags_ = *bufferMeta.tags_;
}

std::shared_ptr<BufferMeta> AudioBufferMeta::Clone()
{
    auto bufferMeta = std::shared_ptr<AudioBufferMeta>(new AudioBufferMeta());
    bufferMeta->samples = samples;
    bufferMeta->sampleFormat = sampleFormat;
    bufferMeta->sampleRate = sampleRate;
    bufferMeta->channels = channels;
    bufferMeta->bytesPreFrame = bytesPreFrame;
    bufferMeta->channelLayout = channelLayout;
    bufferMeta->offsets = offsets;
    bufferMeta->Update(*this);
    return bufferMeta;
}

std::shared_ptr<BufferMeta> VideoBufferMeta::Clone()
{
    auto bufferMeta = std::shared_ptr<VideoBufferMeta>(new VideoBufferMeta());
    bufferMeta->videoPixelFormat = videoPixelFormat;
    bufferMeta->id = id;
    bufferMeta->width = width;
    bufferMeta->height = height;
    bufferMeta->planes = planes;
    bufferMeta->stride = stride;
    bufferMeta->offset = offset;
    bufferMeta->Update(*this);
    return bufferMeta;
}

Buffer::Buffer(BufferMetaType type) : trackID(0), pts(0), dts(0), duration(0), flag (0), meta()
{
    if (type == BufferMetaType::AUDIO) {
        meta = std::shared_ptr<AudioBufferMeta>(new AudioBufferMeta());
    } else if (type == BufferMetaType::VIDEO) {
        meta = std::shared_ptr<VideoBufferMeta>(new VideoBufferMeta());
    }
}

std::shared_ptr<Buffer> Buffer::CreateDefaultBuffer(BufferMetaType type, size_t capacity,
                                                    std::shared_ptr<Allocator> allocator, size_t align)
{
    auto buffer = std::make_shared<Buffer>(type);
    std::shared_ptr<Memory> memory = std::shared_ptr<Memory>(new Memory(capacity, allocator, align));
    buffer->data.push_back(memory);
    return buffer;
}

std::shared_ptr<Memory> Buffer::WrapMemory(uint8_t* data, size_t capacity, size_t size)
{
    auto memory = std::shared_ptr<Memory>(new Memory(capacity, std::shared_ptr<uint8_t>(data, [](void* ptr) {})));
    memory->size = size;
    this->data.push_back(memory);
    return memory;
}

std::shared_ptr<Memory> Buffer::WrapMemoryPtr(std::shared_ptr<uint8_t> data, size_t capacity, size_t size)
{
    auto memory = std::shared_ptr<Memory>(new Memory(capacity, data));
    memory->size = size;
    this->data.push_back(memory);
    return memory;
}

#if !defined(OHOS_LITE) && defined(VIDEO_SUPPORT)
std::shared_ptr<Memory> Buffer::WrapSurfaceMemory(sptr<SurfaceBuffer> surfaceBuffer)
{
    int32_t bufferSize;
    auto ret = surfaceBuffer->GetExtraData()->ExtraGet("dataSize", bufferSize);
    if (ret != OHOS::SurfaceError::SURFACE_ERROR_OK || bufferSize <= 0) {
        return nullptr;
    }
    auto memory = std::shared_ptr<SurfaceMemory>(new SurfaceMemory(surfaceBuffer, bufferSize));
    this->data.push_back(memory);
    return memory;
}
#endif

std::shared_ptr<Memory> Buffer::AllocMemory(std::shared_ptr<Allocator> allocator, size_t capacity, size_t align)
{
    auto type = (allocator != nullptr) ? allocator->GetMemoryType() : MemoryType::VIRTUAL_ADDR;
    std::shared_ptr<Memory> memory = nullptr;
    switch (type) {
        case MemoryType::VIRTUAL_ADDR: {
            memory = std::shared_ptr<Memory>(new Memory(capacity, allocator, align));
            break;
        }
#if !defined(OHOS_LITE) && defined(VIDEO_SUPPORT)
        case MemoryType::SURFACE_BUFFER: {
            memory = std::shared_ptr<Memory>(new SurfaceMemory(capacity, allocator, align));
            break;
        }
        case MemoryType::SHARE_MEMORY:
            memory = std::shared_ptr<Memory>(new ShareMemory(capacity, allocator, align));
            break;
#endif
        default:
            break;
    }
    if (memory == nullptr) {
        return nullptr;
    }
    data.push_back(memory);
    return memory;
}

uint32_t Buffer::GetMemoryCount()
{
    return data.size();
}

std::shared_ptr<Memory> Buffer::GetMemory(uint32_t index)
{
    if (data.size() <= index) {
        return nullptr;
    }
    return data[index];
}

std::shared_ptr<BufferMeta> Buffer::GetBufferMeta()
{
    return meta;
}

void Buffer::UpdateBufferMeta(const BufferMeta& bufferMeta)
{
    meta->Update(bufferMeta);
}

bool Buffer::IsEmpty()
{
    return data.empty();
}

void Buffer::Reset()
{
    data[0]->Reset();
    trackID = 0;
    pts = 0;
    dts = 0;
    duration = 0;
    flag = 0;
    BufferMetaType type = meta->GetType();
    meta.reset();
    if (type == BufferMetaType::AUDIO) {
        meta = std::shared_ptr<AudioBufferMeta>(new AudioBufferMeta());
    } else if (type == BufferMetaType::VIDEO) {
        meta = std::shared_ptr<VideoBufferMeta>(new VideoBufferMeta());
    }
}

void Buffer::ChangeBufferMetaType(BufferMetaType type)
{
    meta.reset();
    if (type == BufferMetaType::AUDIO) {
        meta = std::shared_ptr<AudioBufferMeta>(new AudioBufferMeta());
    } else if (type == BufferMetaType::VIDEO) {
        meta = std::shared_ptr<VideoBufferMeta>(new VideoBufferMeta());
    }
}

} // namespace Plugin
} // namespace Media
} // namespace OHOS
