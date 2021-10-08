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

#ifndef HISTREAMER_PLUGIN_COMMON_BUFFER_H
#define HISTREAMER_PLUGIN_COMMON_BUFFER_H

#include <memory>
#include <map>

#include "plugin_tags.h"
#include "plugin_types.h"
#include "plugin_audio_tags.h"
#include "plugin_video_tags.h"

namespace OHOS {
namespace Media {
namespace Plugin {
/// End of Stream Buffer Flag
#define BUFFER_FLAG_EOS 0x00000001

// Align value template
template <typename T>
using MakeUnsigned = typename std::make_unsigned<T>::type;

template <typename T, typename U>
constexpr T AlignUp(T num, U alignment)
{
    return (alignment > 0) ? (static_cast<uint64_t>((num + static_cast<MakeUnsigned<T>>(alignment) - 1)) &
        static_cast<uint64_t>((~(static_cast<MakeUnsigned<T>>(alignment) - 1)))) :
        num;
}

/**
 * @brief Memory allocator, which is provided by the plugin implementer.
 *
 * @since 1.0
 * @version 1.0
 */
struct Allocator {
    virtual ~Allocator() = default;
    /**
     * @brief Allocates a buffer using the specified size
     * .
     * @param size  Allocation parameters.
     * @return      Pointer of the allocated buffer.
     */
    virtual void* Alloc(size_t size) = 0;

    /**
     * @brief Frees a buffer.
     * Buffer handles returned by Alloc() must be freed with this function when no longer needed.
     *
     * @param ptr   Pointer of the allocated buffer.
     */
    virtual void Free(void* ptr) = 0;
};

/**
 * @enum Buffer Meta Type
 *
 * @since 1.0
 * @version 1.0
 */
enum struct BufferMetaType : uint32_t {
    AUDIO,      ///< Meta used to describe audio data
    VIDEO,      ///< Meta used to describe video data
};

/**
* @brief Memory description. Only manager the basic memory information.
*
*  here is the memory layout.
*            |            capacity                      |
*            |------------------------------------------|
*            |       buffer size       |
*            |-------------------------|
*  addr   offset                      buffer end
*  +---------+-------------------------+----------------+
*  |*********|        used buffer      |  unused buffer |
*  +---------+-------------------------+----------------+
*
*  operate position:
*                         position
*            +----------------+
*
* @since 1.0
* @version 1.0
*/
class Memory {
public:
    /// Destructor
    ~Memory() = default;

    // Todo: Add the documentation description.
    size_t GetCapacity();

    size_t GetSize();

    const uint8_t* GetReadOnlyData(size_t position = 0);

    uint8_t *GetWritableData(size_t size, size_t position = 0);

    size_t Write(const uint8_t* in, size_t size, size_t position = std::string::npos);

    size_t Read(uint8_t* out, size_t size, size_t position = std::string::npos);

    void Reset();

private:
    /**
     * Create objects based on the external memory, use shared pointers,
     * the allocation and release of memory are managed externally.
     *
     * @param capacity Allocated memory size.
     * @param bufData External memory.
     * @param align The alignment of the memory.
     */
    Memory(size_t capacity, std::shared_ptr<uint8_t> bufData, size_t align = 1);

    /**
     * Allocates memory by the specified allocator.
     * Allocation and release are the responsibility of the external allocator.
     *
     * @param capacity Allocated memory size.
     * @param allocator External allocator.
     * @param align The alignment of the memory.
     */
    explicit Memory(size_t capacity, std::shared_ptr<Allocator> allocator = nullptr, size_t align = 1);

    /**
     * Get real memory address, it is addr + offset, the offset is calculated according to alignment.
     */
    uint8_t *GetRealAddr() const;

private:
    /// Allocated memory size.
    size_t capacity;

    /// The alignment of the memory.
    size_t alignment;

    /// Offset of the buffer address to make sure access acording to alignment.
    size_t offset {0};

    /// Valid data size
    size_t size;

    /// Externally specified allocator, optional.
    std::shared_ptr<Allocator> allocator;

    /// Allocated memory address.
    std::shared_ptr<uint8_t> addr;

    friend class Buffer;
};

/**
 * @brief Buffer Meta.
 * Base class that describes various media metadata.
 *
 * @since 1.0
 * @version 1.0
 */
class BufferMeta {
public:
    /// Destructor
    virtual ~BufferMeta() = default;

    ValueType GetMeta(Tag tag);

    void SetMeta(Tag tag, ValueType value);

    BufferMetaType GetType() const;

protected:
    /// Constructor
    explicit BufferMeta(BufferMetaType type);

private:
    BufferMetaType type;

    /// Buffer metadata information of the buffer, which is represented by the key-value pair of the tag.
    std::shared_ptr<TagMap> tags {};
};

/**
 * @brief Audio buffer metadata.
 *
 * Buffer metadata describing how data is laid out inside the buffer.
 *
 * @since 1.0
 * @version 1.0
 */
class AudioBufferMeta : public BufferMeta {
public:
    /// Destructor
    ~AudioBufferMeta() = default;

    /// the number of valid samples in the buffer
    size_t samples {0};

    /// Audio sample formats
    AudioSampleFormat sampleFormat {AudioSampleFormat::S8};

    /// the audio sample rate
    uint32_t sampleRate {0};

    /// the number of channels
    uint32_t channels {0};

    /// the number bytes for one frame, this is the size of one sample * channels
    uint32_t bytesPreFrame {0};

    /// Indicates that the channel order.
    AudioChannelLayout channelLayout {AudioChannelLayout::MONO};

    /// the offsets (in bytes) where each channel plane starts in the buffer.
    std::vector<size_t> offsets {};

private:
    /// Constructor
    AudioBufferMeta() : BufferMeta(BufferMetaType::AUDIO) {}

    friend class Buffer;
};

/**
 * @brief Video buffer metadata.
 *
 *  Extra buffer metadata describing video properties.
 *
 *  @since 1.0
 *  @version 1.0
 */
class VideoBufferMeta : public BufferMeta {
public:
    /// Destructor
    ~VideoBufferMeta() = default;

    /// describing video formats.
    VideoPixelFormat  videoPixelFormat {VideoPixelFormat::UNKNOWN};

    /// identifier of the frame。
    uint32_t id {0};

    /// the video width.
    uint32_t width {0};

    /// the video height.
    uint32_t height {0};

    /// the number of planes in the image.
    uint32_t planes {0};

    /// array of strides for the planes.
    std::vector<uint32_t> stride {};

    /// array of offsets for the planes.
    std::vector<uint32_t> offset {};

private:
    /// Constructor
    VideoBufferMeta() : BufferMeta(BufferMetaType::VIDEO) {}

    friend class Buffer;
};

/**
* @brief Buffer base class.
* Contains the data storage and metadata information of the buffer (buffer description information).
*
* @since 1.0
* @version 1.0
*/
class Buffer {
public:
    /// Construct an empty buffer.
    explicit Buffer(BufferMetaType type = BufferMetaType::AUDIO);

    /// Destructor
    ~Buffer() = default;

    static std::shared_ptr<Buffer> CreateDefaultBuffer(BufferMetaType type, size_t capacity,
                                                       std::shared_ptr<Allocator> allocator = nullptr,
                                                       size_t align = 1);

    std::shared_ptr<Memory> WrapMemory(uint8_t* data, size_t capacity, size_t size);

    std::shared_ptr<Memory> WrapMemoryPtr(std::shared_ptr<uint8_t> data, size_t capacity, size_t size);

    std::shared_ptr<Memory> AllocMemory(std::shared_ptr<Allocator> allocator, size_t capacity, size_t align = 1);

    uint32_t GetMemoryCount();

    std::shared_ptr<Memory> GetMemory(uint32_t index = 0);

    std::shared_ptr<BufferMeta> GetBufferMeta();

    void Reset();

    /// no memory in the buffer.
    bool IsEmpty();

    /// stream index.
    uint32_t streamID;

    /// presentation timestamp of the buffer.
    uint64_t pts;

    /// decoding timestamp of the buffer.
    uint64_t dts;

    /// duration in time of the buffer data.
    uint64_t duration;

    /// flag of the buffer, which is used to record extra information.
    /// @see BUFFER_FLAG_EOS
    uint64_t flag;

private:
    /// Data described by this buffer.
    std::vector<std::shared_ptr<Memory>> data {};

    /// The audio buffer meta information.
    std::shared_ptr<BufferMeta> meta;
};
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PLUGIN_COMMON_BUFFER_H
