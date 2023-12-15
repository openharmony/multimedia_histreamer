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

#include "avbuffer_queue_consumer_impl.h"
#include "avbuffer_queue_producer_impl.h"
#include "common/log.h"


namespace OHOS {
namespace Media {

AVBufferQueueSurfaceWrapper::AVBufferQueueSurfaceWrapper(
        sptr<Surface>& surface, const std::string& name, uint8_t wrapperType):
        AVBufferQueueImpl(surface->GetQueueSize(), MemoryType::UNKNOWN_MEMORY, name),
        surface_(surface), wrapperType_(wrapperType)
{
    for (uint32_t i = 0; i < surface->GetQueueSize(); i++) {
        auto buffer = AVBuffer::CreateAVBuffer();
        cachedBufferMap_[buffer->GetUniqueId()] = buffer;
        freeBufferList_.emplace_back(buffer->GetUniqueId());
    }
}

sptr<AVBufferQueueProducer> AVBufferQueueSurfaceWrapper::GetProducer()
{
    return AVBufferQueueImpl::GetProducer();
}

sptr<AVBufferQueueConsumer> AVBufferQueueSurfaceWrapper::GetConsumer()
{
    if (wrapperType_ == PRODUCER_WRAPPER) {
        return nullptr;
    }
    return AVBufferQueueImpl::GetConsumer();
}

sptr<Surface> AVBufferQueueSurfaceWrapper::GetSurfaceAsProducer()
{
    if (wrapperType_ == PRODUCER_WRAPPER || wrapperType_ == BOTH_WRAPPER) {
        return nullptr;
    }
    return surface_;
}

sptr<Surface> AVBufferQueueSurfaceWrapper::GetSurfaceAsConsumer()
{
    if (wrapperType_ == CONSUMER_WRAPPER || wrapperType_ == BOTH_WRAPPER) {
        return nullptr;
    }
    return surface_;
}

uint32_t AVBufferQueueSurfaceWrapper::GetQueueSize()
{
    return surface_->GetQueueSize();
}

Status AVBufferQueueSurfaceWrapper::SetQueueSize(uint32_t size)
{
    NZERO_RETURN_V(surface_->SetQueueSize(size), Status::ERROR_SURFACE_INNER);

    std::lock_guard<std::mutex> lockGuard(queueMutex_);

    auto curSize = cachedBufferMap_.size();
    if (size > curSize) {
        for (auto i = curSize; i < size; ++i) {
            auto buffer = AVBuffer::CreateAVBuffer();
            cachedBufferMap_[buffer->GetUniqueId()] = buffer;
            freeBufferList_.emplace_back(buffer->GetUniqueId());
        }
    }

    return Status::OK;
}

Status AVBufferQueueSurfaceWrapper::BindSurface(
        std::shared_ptr<AVBuffer>& buffer, sptr<SurfaceBuffer>& surfaceBuffer, int32_t fence)
{
    std::lock_guard<std::mutex> lockGuard(queueMutex_);
    if (freeBufferList_.empty()) {
        MEDIA_LOG_W("cannot find free buffer, alloc new one");
        buffer = AVBuffer::CreateAVBuffer();
        cachedBufferMap_[buffer->GetUniqueId()] = buffer;
    } else {
        buffer = cachedBufferMap_[freeBufferList_.front()];
        freeBufferList_.pop_front();

    }

    // todo:需要AVBuffer实现对SurfaceBuffer的封装
//    buffer->surfaceBuffer_ = surfaceBuffer;
//    buffer->fence_ = fence;

    return Status::OK;
}

Status AVBufferQueueSurfaceWrapper::UnbindSurface(uint64_t uniqueId, sptr<SurfaceBuffer>& surfaceBuffer, int32_t& fence,
                                                  BufferFlushConfig& config)
{
    std::lock_guard<std::mutex> lockGuard(queueMutex_);
    FALSE_RETURN_V(cachedBufferMap_.find(uniqueId) != cachedBufferMap_.end(), Status::ERROR_INVALID_BUFFER_ID);

    // todo:需要AVBuffer实现对SurfaceBuffer的封装
//    surfaceBuffer = cachedBufferMap_[uniqueId]->surfaceBuffer_;
//    cachedBufferMap_[uniqueId]->surfaceBuffer_ = nullptr;
    freeBufferList_.emplace_back(uniqueId);

    return Status::OK;
}

Status AVBufferQueueSurfaceWrapper::RequestBuffer(
        std::shared_ptr<AVBuffer>& buffer, const AVBufferConfig& config, int32_t timeoutMs)
{
    sptr<SurfaceBuffer> surfaceBuffer = nullptr;
    int32_t fence;
    FALSE_RETURN_V(config.surfaceBufferConfig != nullptr, Status::ERROR_INVALID_PARAMETER);
    BufferRequestConfig surfaceBufferConfig = *(config.surfaceBufferConfig);
    NZERO_RETURN_V(surface_->RequestBuffer(surfaceBuffer, fence, surfaceBufferConfig),
                   Status::ERROR_SURFACE_INNER);

    return BindSurface(buffer, surfaceBuffer, fence);
}

Status AVBufferQueueSurfaceWrapper::CancelBuffer(uint64_t uniqueId)
{
    // todo:需要AVBuffer实现对SurfaceBuffer的封装
//    NZERO_RETURN_V(surface_->CancelBuffer(cachedBufferMap_[uniqueId]->surfaceBuffer_),
//                   Status::ERROR_SURFACE_INNER);

    sptr<SurfaceBuffer> surfaceBuffer = nullptr;
    int32_t fence;
    BufferFlushConfig config;
    return UnbindSurface(uniqueId, surfaceBuffer, fence, config);
}

Status AVBufferQueueSurfaceWrapper::PushBuffer(uint64_t uniqueId, bool available)
{
    if (brokerListener_ != nullptr) {
        std::lock_guard<std::mutex> lockGuard(brokerListenerMutex_);
        if (brokerListener_ != nullptr) {
            brokerListener_->OnBufferFilled(cachedBufferMap_[uniqueId]);
            return Status::OK;
        }
    }

    return ReturnBuffer(uniqueId, available);
}

Status AVBufferQueueSurfaceWrapper::PushBuffer(const std::shared_ptr<AVBuffer>& buffer, bool available)
{
    FALSE_RETURN_V(buffer != nullptr, Status::ERROR_NULL_POINT_BUFFER);

    return PushBuffer(buffer->GetUniqueId(), available);
}

Status AVBufferQueueSurfaceWrapper::ReturnBuffer(uint64_t uniqueId, bool available)
{
    if (!available) {
        return CancelBuffer(uniqueId);
    }

    int32_t fence;
    sptr<SurfaceBuffer> surfaceBuffer = nullptr;
    BufferFlushConfig config;
    NOK_RETURN(UnbindSurface(uniqueId, surfaceBuffer, fence, config));

    NZERO_RETURN_V(surface_->FlushBuffer(surfaceBuffer, fence, config), Status::ERROR_SURFACE_INNER);

    return Status::OK;
}

Status AVBufferQueueSurfaceWrapper::ReturnBuffer(const std::shared_ptr<AVBuffer>& buffer, bool available)
{
    FALSE_RETURN_V(buffer != nullptr, Status::ERROR_NULL_POINT_BUFFER);

    return ReturnBuffer(buffer->GetUniqueId(), available);
}

Status AVBufferQueueSurfaceWrapper::AttachBuffer(std::shared_ptr<AVBuffer>& buffer, bool isFilled)
{
    FALSE_RETURN_V(buffer != nullptr, Status::ERROR_NULL_POINT_BUFFER);

    FALSE_RETURN_V(!isFilled, Status::ERROR_INVALID_PARAMETER);

    // todo:需要AVBuffer实现对SurfaceBuffer的封装
//    FALSE_RETURN_V(buffer->surfaceBuffer_ != nullptr, Status::ERROR_NULL_SURFACE_BUFFER);

//    NZERO_RETURN_V(surface_->AttachBuffer(buffer->surfaceBuffer_), Status::ERROR_SURFACE_INNER);

    return Status::OK;
}

Status AVBufferQueueSurfaceWrapper::DetachBuffer(uint64_t uniqueId)
{
    sptr<SurfaceBuffer> surfaceBuffer = nullptr;
    int32_t fence;
    BufferFlushConfig config;
    NOK_RETURN(UnbindSurface(uniqueId, surfaceBuffer, fence, config));

    NZERO_RETURN_V(surface_->DetachBuffer(surfaceBuffer), Status::ERROR_SURFACE_INNER);

    return Status::OK;
}

Status AVBufferQueueSurfaceWrapper::DetachBuffer(const std::shared_ptr<AVBuffer>& buffer)
{
    FALSE_RETURN_V(buffer != nullptr, Status::ERROR_NULL_POINT_BUFFER);

    NOK_RETURN(DetachBuffer(buffer->GetUniqueId()));

    return Status::OK;
}

Status AVBufferQueueSurfaceWrapper::AcquireBuffer(std::shared_ptr<AVBuffer>& buffer)
{
    sptr<SurfaceBuffer> surfaceBuffer = nullptr;
    int32_t fence;
    int64_t timestamp; // todo:怎么赋值
    Rect range;
    NZERO_RETURN_V(surface_->AcquireBuffer(surfaceBuffer, fence, timestamp, range),
                   Status::ERROR_SURFACE_INNER);

    NOK_RETURN(BindSurface(buffer, surfaceBuffer, fence));

    return Status::OK;
}

Status AVBufferQueueSurfaceWrapper::ReleaseBuffer(uint64_t uniqueId)
{
    sptr<SurfaceBuffer> surfaceBuffer = nullptr;
    int32_t fence;
    BufferFlushConfig config;
    NOK_RETURN(UnbindSurface(uniqueId, surfaceBuffer, fence, config));

    NZERO_RETURN_V(surface_->ReleaseBuffer(surfaceBuffer, fence), Status::ERROR_SURFACE_INNER);

    return Status::OK;
}

Status AVBufferQueueSurfaceWrapper::ReleaseBuffer(const std::shared_ptr<AVBuffer>& buffer)
{
    FALSE_RETURN_V(buffer != nullptr, Status::ERROR_NULL_POINT_BUFFER);

    return ReleaseBuffer(buffer->GetUniqueId());
}

Status AVBufferQueueSurfaceWrapper::SetBrokerListener(sptr<IBrokerListener>& listener)
{
    std::lock_guard<std::mutex> lockGuard(producerListenerMutex_);
    brokerListener_ = listener;
    return Status::OK;
}

Status AVBufferQueueSurfaceWrapper::SetProducerListener(sptr<IProducerListener>& listener)
{
    std::lock_guard<std::mutex> lockGuard(producerListenerMutex_);
    producerListener_ = listener;

    return Status::OK;
}

class SurfaceConsumerListener: public IBufferConsumerListener
{
public:
    explicit SurfaceConsumerListener(std::function<void()> releaseBufferFunc):
            onReleaseBufferFunc_(std::move(releaseBufferFunc)) { }

    void OnBufferAvailable() override {
        onReleaseBufferFunc_();
    }

private:
    std::function<void(void)> onReleaseBufferFunc_;
};

Status AVBufferQueueSurfaceWrapper::SetConsumerListener(sptr<IConsumerListener>& listener)
{
    auto releaseBufferFunc = [this]()->void {
        std::lock_guard<std::mutex> lockGuard(consumerListenerMutex_);
        if (consumerListener_ != nullptr) {
            consumerListener_->OnBufferAvailable();
        }
    };

    std::lock_guard<std::mutex> lockGuard(consumerListenerMutex_);
    if (listener == nullptr) {
        surfaceConsumerListener_ = nullptr;
        surface_->RegisterConsumerListener(nullptr);
    } else {
        surfaceConsumerListener_ = new SurfaceConsumerListener(releaseBufferFunc);
        surface_->RegisterConsumerListener(surfaceConsumerListener_);
    }

    consumerListener_ = listener;

    return Status::OK;
}

}
}