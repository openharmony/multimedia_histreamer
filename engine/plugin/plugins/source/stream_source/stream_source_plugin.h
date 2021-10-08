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
#include "foundation/blocking_queue.h"
#include "foundation/buffer_pool.h"
#include "foundation/constants.h"
#include "foundation/error_code.h"
#include "foundation/type_define.h"
#include "osal/thread/task.h"
#include "plugin/common/plugin_types.h"
#include "plugin/interface/source_plugin.h"

namespace OHOS {
namespace Media {
namespace Plugin {
using SourceType = OHOS::Media::SourceType;
using MediaSource = OHOS::Media::Source;
using StreamCallback = OHOS::Media::StreamCallback;
using StreamSource = OHOS::Media::StreamSource;

class StreamSourcePlugin;

class StreamSourceAllocator : public Allocator {
public:
    StreamSourceAllocator() = default;
    ~StreamSourceAllocator() = default;

    void* Alloc(size_t size) override;
    void Free(void* ptr) override;
};

class StreamSourceCallback : public StreamCallback {
public:
    StreamSourceCallback(std::shared_ptr<StreamSourcePlugin> dataSource, std::shared_ptr<StreamSource>& stream);
    ~StreamSourceCallback() = default;

    uint8_t* GetBuffer(size_t index);
    void QueueBuffer(size_t index, size_t offset, size_t size, int64_t timestampUs, uint32_t flags);
    void SetParameters(const Media::Format& params)
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
    bool IsParameterSupported(Tag tag) override;
    Status GetParameter(Tag tag, ValueType& value) override;
    Status SetParameter(Tag tag, const ValueType& value) override;
    std::shared_ptr<Allocator> GetAllocator() override;
    Status SetCallback(const std::shared_ptr<Callback>& cb) override;
    Status SetSource(std::string& uri, std::shared_ptr<std::map<std::string, ValueType>> params = nullptr) override;
    Status Read(std::shared_ptr<Buffer>& buffer, size_t expectedLen) override;
    Status GetSize(size_t& size) override;
    bool IsSeekable() override;
    Status SeekTo(uint64_t offset) override;

    AVBufferPtr AllocateBuffer();
    AVBufferPtr FindBuffer(size_t idx);
    void EraseBuffer(size_t idx);
    void EnqueBuffer(AVBufferPtr& bufferPtr);

protected:
    AVBufferPool bufferPool_;

private:
    State state_;
    bool isSeekable_;
    OSAL::Mutex mutex_ {};
    std::map<size_t, AVBufferPtr> waitBuffers_;
    std::weak_ptr<StreamSource> streamSource_ {};
    std::shared_ptr<StreamSourceCallback> streamCallback_ {nullptr};
    size_t idx_ {0};
    BlockingQueue<AVBufferPtr> bufferQueue_;
    std::shared_ptr<OSAL::Task> taskPtr_ {nullptr};
    std::shared_ptr<StreamSourceAllocator> mAllocator_ {nullptr};

    void NotifyAvilableBufferLoop();
    size_t GetUniqueIdx()
    {
        return ++idx_;
    }
};
} // namespace Plugin
} // namespace Media
} // namespace OHOS

#endif // HISTREAMER_STREAM_SOURCE_PLUGIN_H
