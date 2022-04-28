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

#ifndef HISTREAMER_STREAM_SOURCE_PLUGIN_H
#define HISTREAMER_STREAM_SOURCE_PLUGIN_H

#include "source.h"
#include "foundation/osal/thread/task.h"
#include "utils/blocking_queue.h"
#include "utils/buffer_pool.h"
#include "utils/constants.h"
#include "pipeline/core/error_code.h"
#include "plugin/common/plugin_types.h"
#include "plugin/interface/source_plugin.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace StreamSource {
using StreamCallback = OHOS::Media::StreamCallback;
using StreamSource = OHOS::Media::StreamSource;

class StreamSourcePlugin;

class StreamSourceAllocator : public Allocator {
public:
    StreamSourceAllocator() = default;
    ~StreamSourceAllocator() = default;

    void* Alloc(size_t size) override;
    void Free(void* ptr) override; // NOLINT: void*
};

class StreamSourceCallback : public StreamCallback {
public:
    StreamSourceCallback(std::shared_ptr<StreamSourcePlugin> dataSource, std::shared_ptr<StreamSource>& stream);
    virtual ~StreamSourceCallback() = default;

    uint8_t* GetBuffer(size_t index) override;
    void QueueBuffer(size_t index, size_t offset, size_t size, int64_t timestampUs, uint32_t flags) override;
    void SetParameters(const Media::Format& params) override
    {
    }

private:
    std::shared_ptr<StreamSourcePlugin> dataSource_;
    std::weak_ptr<StreamSource> streamSource_;
};

class StreamSourcePlugin : public SourcePlugin, std::enable_shared_from_this<StreamSourcePlugin> {
public:
    explicit StreamSourcePlugin(std::string name);
    ~StreamSourcePlugin();

    Status Init() override;
    Status Deinit() override;
    Status Prepare() override;
    Status Reset() override;
    Status Start() override;
    Status Stop() override;
    Status GetParameter(Tag tag, ValueType& value) override;
    Status SetParameter(Tag tag, const ValueType& value) override;
    std::shared_ptr<Allocator> GetAllocator() override;
    Status SetCallback(Callback* cb) override;
    Status SetSource(std::shared_ptr<MediaSource> source) override;
    Status Read(std::shared_ptr<Buffer>& buffer, size_t expectedLen) override;
    Status GetSize(size_t& size) override;
    bool IsSeekable() override;
    Status SeekTo(uint64_t offset) override;

    std::shared_ptr<Plugin::Buffer> AllocateBuffer();
    std::shared_ptr<Plugin::Buffer> FindBuffer(size_t idx);
    void EraseBuffer(size_t idx);
    void EnqueBuffer(std::shared_ptr<Plugin::Buffer>& bufferPtr);

protected:
    BufferPool<Plugin::Buffer> bufferPool_;

private:
    State state_;
    bool isSeekable_;
    OSAL::Mutex mutex_ {};
    std::map<size_t, std::shared_ptr<Plugin::Buffer>> waitBuffers_;
    std::weak_ptr<StreamSource> streamSource_ {};
    std::shared_ptr<StreamSourceCallback> streamCallback_ {nullptr};
    size_t idx_ {0};
    BlockingQueue<std::shared_ptr<Plugin::Buffer>> bufferQueue_;
    std::shared_ptr<OSAL::Task> taskPtr_ {nullptr};
    std::shared_ptr<StreamSourceAllocator> mAllocator_ {nullptr};

    void NotifyAvilableBufferLoop();
    size_t GetUniqueIdx()
    {
        return ++idx_;
    }
};
} // namespace StreamSource
} // namespace Plugin
} // namespace Media
} // namespace OHOS

#endif // HISTREAMER_STREAM_SOURCE_PLUGIN_H
