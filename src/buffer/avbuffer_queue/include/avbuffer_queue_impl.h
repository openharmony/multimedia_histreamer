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

#ifndef HISTREAMER_FOUNDATION_AVBUFFER_QUEUE_IMPL_H
#define HISTREAMER_FOUNDATION_AVBUFFER_QUEUE_IMPL_H

#include <list>
#include <map>
#include <string>
#include <mutex>
#include <condition_variable>
#include "buffer/avbuffer_queue.h"

namespace OHOS {
namespace Media {

using AVBufferState = enum AVBufferState {
    AVBUFFER_STATE_RELEASED,
    AVBUFFER_STATE_REQUESTED,
    AVBUFFER_STATE_PUSHED,
    AVBUFFER_STATE_RETURNED,
    AVBUFFER_STATE_ACQUIRED,
    AVBUFFER_STATE_ATTACHED,
};

using AVBufferElement = struct AVBufferElement {
    AVBufferConfig config;
    AVBufferState state;
    bool isDeleting;
    std::shared_ptr<AVBuffer> buffer;
};

class AVBufferQueueProducerImpl;
class AVBufferQueueConsumerImpl;

    // 当前调试版本，错误码统一用int32_t表示，0表示返回正确，非0表示返回错误。
class AVBufferQueueImpl : public AVBufferQueue, public std::enable_shared_from_this<AVBufferQueueImpl> {
public:
    explicit AVBufferQueueImpl(const std::string &name);
    AVBufferQueueImpl(uint32_t size, MemoryType type, const std::string &name);
    ~AVBufferQueueImpl() override = default;
    AVBufferQueueImpl(const AVBufferQueueImpl&) = delete;
    AVBufferQueueImpl operator=(const AVBufferQueueImpl&) = delete;

    std::shared_ptr<AVBufferQueueProducer> GetLocalProducer() override;
    std::shared_ptr<AVBufferQueueConsumer> GetLocalConsumer() override;

    sptr<AVBufferQueueProducer> GetProducer() override;
    sptr<AVBufferQueueConsumer> GetConsumer() override;

    inline sptr<Surface> GetSurfaceAsProducer() override { return nullptr; }
    inline sptr<Surface> GetSurfaceAsConsumer() override { return nullptr; }

    uint32_t GetQueueSize() override;
    Status SetQueueSize(uint32_t size) override;

    virtual Status RequestBuffer(std::shared_ptr<AVBuffer>& buffer,
                          const AVBufferConfig& config, int32_t timeoutMs);
    virtual Status PushBuffer(uint64_t uniqueId, bool available);
    virtual Status PushBuffer(const std::shared_ptr<AVBuffer>& buffer, bool available);
    virtual Status ReturnBuffer(uint64_t uniqueId, bool available);
    virtual Status ReturnBuffer(const std::shared_ptr<AVBuffer>& buffer, bool available);

    virtual Status AttachBuffer(std::shared_ptr<AVBuffer>& buffer, bool isFilled);
    virtual Status DetachBuffer(uint64_t uniqueId);
    virtual Status DetachBuffer(const std::shared_ptr<AVBuffer>& buffer);

    virtual Status AcquireBuffer(std::shared_ptr<AVBuffer>& buffer);
    virtual Status ReleaseBuffer(const std::shared_ptr<AVBuffer>& buffer);

    virtual Status SetBrokerListener(sptr<IBrokerListener>& listener);
    virtual Status SetProducerListener(sptr<IProducerListener>& listener);
    virtual Status SetConsumerListener(sptr<IConsumerListener>& listener);

protected:
    std::string name_;

    std::mutex producerCreatorMutex_;
    std::mutex consumerCreatorMutex_;

    sptr<IBrokerListener> brokerListener_;
    sptr<IProducerListener> producerListener_;
    sptr<IConsumerListener> consumerListener_;

    std::mutex producerListenerMutex_;
    std::mutex consumerListenerMutex_;
    std::mutex brokerListenerMutex_;

    std::mutex queueMutex_;

    std::weak_ptr<AVBufferQueueProducerImpl> localProducer_;
    std::weak_ptr<AVBufferQueueConsumerImpl> localConsumer_;

    wptr<AVBufferQueueProducerImpl> producer_;
    wptr<AVBufferQueueConsumerImpl> consumer_;

private:
    uint32_t size_;
    MemoryType memoryType_;

    std::map<uint64_t, AVBufferElement> cachedBufferMap_;

    std::list<uint64_t> freeBufferList_;  // 记录已分配的且处于空闲状态的buffer uniqueId，按bufferSize升序排列
    std::list<uint64_t> dirtyBufferList_;

    std::condition_variable requestCondition;

    Status CheckConfig(const AVBufferConfig& config);

    bool wait_for(std::unique_lock<std::mutex>& lock, int32_t timeoutMs);

    uint32_t GetCachedBufferCount() const;
    Status RequestReuseBuffer(std::shared_ptr<AVBuffer>& buffer, const AVBufferConfig& config);
    void InsertFreeBufferInOrder(uint64_t uniqueId);
    Status CancelBuffer(uint64_t uniqueId);
    Status DetachBuffer(uint64_t uniqueId, bool force);
    Status ReleaseBuffer(uint64_t uniqueId);
    Status PopFromFreeBufferList(std::shared_ptr<AVBuffer>& buffer, const AVBufferConfig& config);
    Status PopFromDirtyBufferList(std::shared_ptr<AVBuffer>& buffer);
    Status AllocBuffer(std::shared_ptr<AVBuffer>& buffer, const AVBufferConfig& config);

    void DeleteBuffers(uint32_t count);
    void DeleteCachedBufferById(uint64_t uniqueId_);
};

class AVBufferQueueSurfaceWrapper : public AVBufferQueueImpl {
public:
    enum: uint8_t {
        PRODUCER_WRAPPER,
        CONSUMER_WRAPPER,
        BOTH_WRAPPER
    };

    AVBufferQueueSurfaceWrapper(sptr<Surface>& surface, const std::string& name, uint8_t wrapperType);

    sptr<AVBufferQueueProducer> GetProducer() override;
    sptr<AVBufferQueueConsumer> GetConsumer() override;

    sptr<Surface> GetSurfaceAsProducer() override;
    sptr<Surface> GetSurfaceAsConsumer() override;

    uint32_t GetQueueSize() override;
    Status SetQueueSize(uint32_t size) override;

    Status RequestBuffer(std::shared_ptr<AVBuffer>& buffer,
                         const AVBufferConfig& config, int32_t timeoutMs) override;
    Status PushBuffer(uint64_t uniqueId, bool available) override;
    Status PushBuffer(const std::shared_ptr<AVBuffer>& buffer, bool available) override;
    Status ReturnBuffer(uint64_t uniqueId, bool available) override;
    Status ReturnBuffer(const std::shared_ptr<AVBuffer>& buffer, bool available) override;

    Status AttachBuffer(std::shared_ptr<AVBuffer>& buffer, bool isFilled) override;
    Status DetachBuffer(uint64_t uniqueId) override;
    Status DetachBuffer(const std::shared_ptr<AVBuffer>& buffer) override;

    Status AcquireBuffer(std::shared_ptr<AVBuffer>& buffer) override;
    Status ReleaseBuffer(const std::shared_ptr<AVBuffer>& buffer) override;

    Status SetBrokerListener(sptr<IBrokerListener>& listener) override;
    Status SetProducerListener(sptr<IProducerListener>& listener) override;
    Status SetConsumerListener(sptr<IConsumerListener>& listener) override;

private:
    sptr<Surface> surface_;
    uint8_t wrapperType_;

    std::list<uint64_t> freeBufferList_;
    std::map<uint64_t, std::shared_ptr<AVBuffer>> cachedBufferMap_;

    sptr<IBufferConsumerListener> surfaceConsumerListener_;

    Status BindSurface(std::shared_ptr<AVBuffer>& buffer, sptr<SurfaceBuffer>& surfaceBuffer, int32_t fence);
    Status UnbindSurface(uint64_t uniqueId, sptr<SurfaceBuffer>& surfaceBuffer, int32_t& fence,
                         BufferFlushConfig& config);
    Status CancelBuffer(uint64_t uniqueId);
    Status ReleaseBuffer(uint64_t uniqueId);
};

} // namespace Media
} // namespace OHOS

#endif // HISTREAMER_FOUNDATION_AVBUFFER_QUEUE_IMPL_H
