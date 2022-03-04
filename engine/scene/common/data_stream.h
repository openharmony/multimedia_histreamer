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

#ifndef MEDIA_DATA_STREAM
#define MEDIA_DATA_STREAM

#include <cstdint>
#include <memory>

namespace OHOS {
namespace Media {

enum class MemoryType {
    MEMORY_VIRTUAL,
    MEMORY_PHY
};

class DataBuffer {
public:
    virtual ~DataBuffer() = default;
    virtual bool IsEos() = 0;
    virtual void SetEos(bool isEos) = 0;
    virtual uint8_t* GetAddress() = 0;
    virtual size_t GetCapacity() = 0;
    virtual size_t GetSize() = 0;
    virtual void SetSize(size_t size) = 0;
};

class DataProducer {
public:
    virtual ~DataProducer() = default;
    virtual bool GetEmptyBuffer(std::shared_ptr<DataBuffer>& buffer, int timeout = -1) = 0; // timeout - millisecond
    virtual bool QueueDataBuffer(const std::shared_ptr<DataBuffer>& buffer) = 0;
};

class DataConsumer {
public:
    virtual ~DataConsumer() = default;
    virtual bool GetDataBuffer(std::shared_ptr<DataBuffer>& buffer, int timeout = -1) = 0; // timeout - millisecond
    virtual bool QueueEmptyBuffer(const std::shared_ptr<DataBuffer>& buffer) = 0;
    virtual bool QueueEmptyBuffer(uint8_t* address) = 0;
};

class DataStream : public DataConsumer, DataProducer{
};

std::shared_ptr<DataStream> CreateDataStream(size_t size, size_t count, MemoryType type = MemoryType::MEMORY_VIRTUAL);

} // Media
} // OHOS
#endif // MEDIA_DATA_STREAM
