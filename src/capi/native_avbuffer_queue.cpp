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

#include "kits/c/native_avbuffer_queue.h"
#include "native_avbuffer_queue_define.h"
#include "buffer/avbuffer_queue.h"
#include "common/native_mfmagic.h"
#include "iremote_stub.h"
#include "common/log.h"

using namespace OHOS::Media;

#define NATIVE_AV_BUFFER_QUEUE_CHECK(nativeBufferQueue, ret)                                       \
    FALSE_RETURN_V((nativeBufferQueue) != nullptr, ret);                                             \
    FALSE_RETURN_V((nativeBufferQueue)->magic_ == NATIVE_ABQ_OBJECT_MAGIC_QUEUE,  ret);              \
    FALSE_RETURN_V((nativeBufferQueue)->bufferQueue_ != nullptr, ret)


#define NATIVE_AV_BUFFER_QUEUE_PRODUCER_CHECK(bufferQueueProducer, ret)                             \
    FALSE_RETURN_V((bufferQueueProducer) != nullptr, ret);                                            \
    FALSE_RETURN_V((bufferQueueProducer)->magic_ == NATIVE_ABQ_OBJECT_MAGIC_PRODUCER,  ret);          \
    NATIVE_AV_BUFFER_QUEUE_CHECK((bufferQueueProducer)->nativeBufferQueue_, ret);                     \
    FALSE_RETURN_V((bufferQueueProducer)->producer_ != nullptr, ret)

#define NATIVE_AV_BUFFER_QUEUE_CONSUMER_CHECK(bufferQueueConsumer, ret)                             \
    FALSE_RETURN_V((bufferQueueConsumer) != nullptr, ret);                                            \
    FALSE_RETURN_V((bufferQueueConsumer)->magic_ == NATIVE_ABQ_OBJECT_MAGIC_CONSUMER, ret);           \
    NATIVE_AV_BUFFER_QUEUE_CHECK((bufferQueueConsumer)->nativeBufferQueue_, ret);                     \
    FALSE_RETURN_V((bufferQueueConsumer)->consumer_ != nullptr, ret)

class ProducerBufferFilledListener : public OHOS::IRemoteStub<IBrokerListener> {
public:
    explicit ProducerBufferFilledListener(
        OH_AVBufferQueueProducer *nativeProducer,
        OH_AVBufferQueueProducer_OnBufferFilled *listener, void *userData)
        : nativeProducer_(nativeProducer), listener_(listener), userData_(userData) { }
    ProducerBufferFilledListener(const ProducerBufferFilledListener &) = delete;
    ProducerBufferFilledListener operator=(const ProducerBufferFilledListener &) = delete;
    ~ProducerBufferFilledListener() override = default;

    void OnBufferFilled(std::shared_ptr<AVBuffer> &buffer) override {
        FALSE_RETURN_W(nativeProducer_ != nullptr);
        FALSE_RETURN_W(nativeProducer_->nativeBufferQueue_ != nullptr);
        FALSE_RETURN_W(listener_ != nullptr && *listener_ != nullptr);
        auto nativeBuffer =
            nativeProducer_->nativeBufferQueue_->FindAndBindNativeBuffer(buffer);
        FALSE_RETURN_W(nativeBuffer != nullptr);
        (*listener_)(nativeProducer_, nativeBuffer, userData_);
    }

private:
    OH_AVBufferQueueProducer* nativeProducer_;
    OH_AVBufferQueueProducer_OnBufferFilled* listener_;
    void* userData_;
};

class ConsumerBufferAvailableListener : public IConsumerListener {
public:
    ConsumerBufferAvailableListener(
        OH_AVBufferQueueConsumer* nativeConsumer,
        OH_AVBufferQueueConsumer_OnBufferAvailable* listener,
        void* userData):
        nativeConsumer_(nativeConsumer), listener_(listener), userData_(userData) { }
    ConsumerBufferAvailableListener(const ConsumerBufferAvailableListener&) = delete;
    ConsumerBufferAvailableListener operator=(const ConsumerBufferAvailableListener&) = delete;

    void OnBufferAvailable() override
    {
        FALSE_RETURN_W(nativeConsumer_ != nullptr);
        FALSE_RETURN_W(listener_ != nullptr && *listener_ != nullptr);
        (*listener_)(nativeConsumer_, userData_);
    }

private:
    OH_AVBufferQueueConsumer* nativeConsumer_;
    OH_AVBufferQueueConsumer_OnBufferAvailable* listener_;
    void* userData_;
};

static OH_AVErrCode NativeObjectAddRef(NativeAVBufferQueueMagic* magicObject)
{
    FALSE_RETURN_V(magicObject != nullptr, OH_AVErrCode::AV_ERR_INVALID_VAL);

    auto it = std::find(NativeAVBufferQueueObjectMagicList.begin(),
                        NativeAVBufferQueueObjectMagicList.end(), magicObject->magic_);
    FALSE_RETURN_V(it != NativeAVBufferQueueObjectMagicList.end(), OH_AVErrCode::AV_ERR_UNSUPPORT);

    magicObject->IncStrongRef(magicObject);

    return OH_AVErrCode::AV_ERR_OK;
}

static OH_AVErrCode NativeObjectDelRef(NativeAVBufferQueueMagic* magicObject)
{
    FALSE_RETURN_V(magicObject != nullptr, OH_AVErrCode::AV_ERR_INVALID_VAL);

    auto it = std::find(NativeAVBufferQueueObjectMagicList.begin(),
                        NativeAVBufferQueueObjectMagicList.end(), magicObject->magic_);
    FALSE_RETURN_V(it != NativeAVBufferQueueObjectMagicList.end(), OH_AVErrCode::AV_ERR_UNSUPPORT);

    magicObject->DecStrongRef(magicObject);

    return OH_AVErrCode::AV_ERR_OK;
}

OH_AVBufferQueue::OH_AVBufferQueue(
    std::shared_ptr<OHOS::Media::AVBufferQueue>& bufferQueue, uint32_t maxBufferCount):
        NativeAVBufferQueueMagic(NATIVE_ABQ_OBJECT_MAGIC_QUEUE), bufferQueue_(bufferQueue)
{
    for (int32_t i = 0; i < maxBufferCount; i++) {
        nativeBufferVector_.push_back(std::make_shared<OH_AVBuffer>(nullptr));
        nativeBufferFree_.push_back(true);
    }
}

OH_AVBuffer* OH_AVBufferQueue::FindAndBindNativeBuffer(std::shared_ptr<AVBuffer>& buffer)
{
    FALSE_RETURN_V(buffer != nullptr, nullptr);

    std::lock_guard<std::mutex> lockGuard(lock_);

    for (uint32_t i = 0; i < nativeBufferFree_.size(); i++) {
        if (nativeBufferFree_[i]) {
            nativeBufferFree_[i] = false;
            nativeBufferVector_[i]->buffer_ = buffer;
            return nativeBufferVector_[i].get();
        }
    }

    return nullptr;
}

OH_AVErrCode OH_AVBufferQueue::UnbindNativeBuffer(const OH_AVBuffer* nativeBuffer)
{
    FALSE_RETURN_V(nativeBuffer != nullptr, OH_AVErrCode::AV_ERR_INVALID_VAL);

    std::lock_guard<std::mutex> lockGuard(lock_);

    for (uint32_t i = 0; i < nativeBufferFree_.size(); i++) {
        if (nativeBufferVector_[i].get() == nativeBuffer) {
            nativeBufferVector_[i]->buffer_ = nullptr;
            nativeBufferFree_[i] = true;
            return OH_AVErrCode::AV_ERR_OK;
        }
    }

    return OH_AVErrCode::AV_ERR_INVALID_VAL;
}

OH_AVBufferQueueConsumer* OH_AVBufferQueueConsumer_Create(const char* name, uint32_t maxBufferCount)
{
    auto bufferQueue = AVBufferQueue::Create(maxBufferCount, MemoryType::VIRTUAL_MEMORY, name);
    FALSE_RETURN_V(bufferQueue != nullptr, nullptr);

    OHOS::sptr<OH_AVBufferQueue> nativeBufferQueue = new OH_AVBufferQueue(bufferQueue, maxBufferCount);
    auto consumer = bufferQueue->GetConsumer();
    auto nativeConsumer = new OH_AVBufferQueueConsumer(nativeBufferQueue, consumer);

    FALSE_RETURN_V(NativeObjectAddRef(nativeConsumer) == OH_AVErrCode::AV_ERR_OK, nullptr);

    return nativeConsumer;
}

OH_AVErrCode OH_AVBufferQueueConsumer_Destroy(OH_AVBufferQueueConsumer* bufferQueueConsumer)
{
    NATIVE_AV_BUFFER_QUEUE_CONSUMER_CHECK(bufferQueueConsumer, OH_AVErrCode::AV_ERR_INVALID_VAL);

    return NativeObjectDelRef(bufferQueueConsumer);
}

OH_AVBufferQueueProducer* OH_AVBufferQueueProducer_CreateFromConsumer(OH_AVBufferQueueConsumer* bufferQueueConsumer)
{
    NATIVE_AV_BUFFER_QUEUE_CONSUMER_CHECK(bufferQueueConsumer, nullptr);

    auto producer = bufferQueueConsumer->nativeBufferQueue_->bufferQueue_->GetProducer();
    auto nativeProducer = new OH_AVBufferQueueProducer(bufferQueueConsumer->nativeBufferQueue_, producer);

    auto ret = NativeObjectAddRef(nativeProducer);
    if (ret != OH_AVErrCode::AV_ERR_OK) {
        delete nativeProducer;
        nativeProducer = nullptr;
    }
    FALSE_RETURN_V(ret == OH_AVErrCode::AV_ERR_OK, nullptr);

    return nativeProducer;
}

OH_AVErrCode OH_AVBufferQueueProducer_Destroy(OH_AVBufferQueueProducer* bufferQueueProducer)
{
    NATIVE_AV_BUFFER_QUEUE_PRODUCER_CHECK(bufferQueueProducer, OH_AVErrCode::AV_ERR_INVALID_VAL);

    return NativeObjectDelRef(bufferQueueProducer);
}

OH_AVErrCode OH_AVBufferQueueProducer_RequestBuffer(
    OH_AVBufferQueueProducer* bufferQueueProducer, OH_AVBuffer** buffer, int32_t bufferSize, int32_t timeoutMs)
{
    NATIVE_AV_BUFFER_QUEUE_PRODUCER_CHECK(bufferQueueProducer, OH_AVErrCode::AV_ERR_INVALID_VAL);

    std::shared_ptr<AVBuffer> localBuffer = nullptr;
    AVBufferConfig config;
    config.size = bufferSize;
    config.memoryType = MemoryType::UNKNOWN_MEMORY;

    auto ret = bufferQueueProducer->producer_->RequestBuffer(localBuffer, config, timeoutMs);
    FALSE_RETURN_V(ret != Status::ERROR_TIMED_OUT, OH_AVErrCode::AV_ERR_TIMEOUT);
    FALSE_RETURN_V_MSG(ret == Status::OK, OH_AVErrCode::AV_ERR_UNKNOWN,
                       "RequestBuffer failed ret = " PUBLIC_LOG_D32, ret);
    FALSE_RETURN_V(localBuffer != nullptr, OH_AVErrCode::AV_ERR_UNKNOWN);

    *buffer = bufferQueueProducer->nativeBufferQueue_->FindAndBindNativeBuffer(localBuffer);
    if (*buffer == nullptr) {
        bufferQueueProducer->producer_->PushBuffer(localBuffer, false);
    }
    FALSE_RETURN_V(*buffer != nullptr, OH_AVErrCode::AV_ERR_UNKNOWN);

    return OH_AVErrCode::AV_ERR_OK;
}

OH_AVErrCode OH_AVBufferQueueProducer_PushBuffer(
    OH_AVBufferQueueProducer* bufferQueueProducer, const OH_AVBuffer* buffer, bool available)
{
    FALSE_RETURN_V(buffer != nullptr, OH_AVErrCode::AV_ERR_INVALID_VAL);
    FALSE_RETURN_V(buffer->buffer_ != nullptr, OH_AVErrCode::AV_ERR_INVALID_VAL);
    NATIVE_AV_BUFFER_QUEUE_PRODUCER_CHECK(bufferQueueProducer, OH_AVErrCode::AV_ERR_INVALID_VAL);

    auto ret = bufferQueueProducer->producer_->PushBuffer(buffer->buffer_, available);
    auto unbind = bufferQueueProducer->nativeBufferQueue_->UnbindNativeBuffer(buffer);
    FALSE_LOG(unbind == OH_AVErrCode::AV_ERR_OK);
    FALSE_RETURN_V_MSG(ret == Status::OK, OH_AVErrCode::AV_ERR_UNKNOWN,
                       "PushBuffer failed ret = " PUBLIC_LOG_D32, ret);

    return OH_AVErrCode::AV_ERR_OK;
}

OH_AVErrCode OH_AVBufferQueueProducer_ReturnBuffer(
    OH_AVBufferQueueProducer* bufferQueueProducer, const OH_AVBuffer* buffer, bool available)
{
    FALSE_RETURN_V(buffer != nullptr, OH_AVErrCode::AV_ERR_INVALID_VAL);
    FALSE_RETURN_V(buffer->buffer_ != nullptr, OH_AVErrCode::AV_ERR_INVALID_VAL);
    NATIVE_AV_BUFFER_QUEUE_PRODUCER_CHECK(bufferQueueProducer, OH_AVErrCode::AV_ERR_INVALID_VAL);

    auto ret = bufferQueueProducer->producer_->ReturnBuffer(buffer->buffer_, available);
    auto unbind = bufferQueueProducer->nativeBufferQueue_->UnbindNativeBuffer(buffer);
    FALSE_LOG(unbind == OH_AVErrCode::AV_ERR_OK);
    FALSE_RETURN_V_MSG(ret == Status::OK, OH_AVErrCode::AV_ERR_UNKNOWN,
                       "ReturnBuffer failed ret = " PUBLIC_LOG_D32, ret);

    return OH_AVErrCode::AV_ERR_OK;
}

OH_AVErrCode OH_AVBufferQueueProducer_SetBufferFilledListener(
    OH_AVBufferQueueProducer* bufferQueueProducer,
    OH_AVBufferQueueProducer_OnBufferFilled* listener,
    void* userData)
{
    NATIVE_AV_BUFFER_QUEUE_PRODUCER_CHECK(bufferQueueProducer, OH_AVErrCode::AV_ERR_INVALID_VAL);
    FALSE_RETURN_V(listener != nullptr, OH_AVErrCode::AV_ERR_INVALID_VAL);

    OHOS::sptr<IBrokerListener> localListener = nullptr;
    if (listener != nullptr) {
        localListener = new ProducerBufferFilledListener(bufferQueueProducer, listener, userData);
    }

    auto ret = bufferQueueProducer->producer_->SetBufferFilledListener(localListener);
    FALSE_RETURN_V(ret == Status::OK, OH_AVErrCode::AV_ERR_INVALID_VAL);

    return OH_AVErrCode::AV_ERR_OK;
}

OH_AVErrCode OH_AVBufferQueueConsumer_AcquireBuffer(
    OH_AVBufferQueueConsumer* bufferQueueConsumer, OH_AVBuffer** buffer)
{
    NATIVE_AV_BUFFER_QUEUE_CONSUMER_CHECK(bufferQueueConsumer, OH_AVErrCode::AV_ERR_INVALID_VAL);
    std::shared_ptr<AVBuffer> localBuffer = nullptr;
    auto ret = bufferQueueConsumer->consumer_->AcquireBuffer(localBuffer);
    FALSE_RETURN_V(ret == Status::OK, OH_AVErrCode::AV_ERR_INVALID_STATE);
    *buffer = bufferQueueConsumer->nativeBufferQueue_->FindAndBindNativeBuffer(localBuffer);
    if (*buffer == nullptr) {
        bufferQueueConsumer->consumer_->ReleaseBuffer(localBuffer);
    }
    FALSE_RETURN_V(*buffer != nullptr, OH_AVErrCode::AV_ERR_INVALID_STATE);

    return OH_AVErrCode::AV_ERR_OK;
}

OH_AVErrCode OH_AVBufferQueueConsumer_ReleaseBuffer(
    OH_AVBufferQueueConsumer* bufferQueueConsumer, const OH_AVBuffer* buffer)
{
    FALSE_RETURN_V(buffer != nullptr, OH_AVErrCode::AV_ERR_INVALID_VAL);
    FALSE_RETURN_V(buffer->buffer_ != nullptr, OH_AVErrCode::AV_ERR_INVALID_VAL);
    NATIVE_AV_BUFFER_QUEUE_CONSUMER_CHECK(bufferQueueConsumer, OH_AVErrCode::AV_ERR_INVALID_VAL);
    auto ret = bufferQueueConsumer->consumer_->ReleaseBuffer(buffer->buffer_);
    auto unbind = bufferQueueConsumer->nativeBufferQueue_->UnbindNativeBuffer(buffer);
    FALSE_LOG(unbind == OH_AVErrCode::AV_ERR_OK);
    FALSE_RETURN_V(ret == Status::OK, OH_AVErrCode::AV_ERR_INVALID_STATE);

    return OH_AVErrCode::AV_ERR_OK;
}

OH_AVErrCode OH_AVBufferQueueConsumer_SetBufferAvailableListener(
    OH_AVBufferQueueConsumer* bufferQueueConsumer,
    OH_AVBufferQueueConsumer_OnBufferAvailable* listener,
    void* userData)
{
    NATIVE_AV_BUFFER_QUEUE_CONSUMER_CHECK(bufferQueueConsumer, OH_AVErrCode::AV_ERR_INVALID_VAL);
    OHOS::sptr<IConsumerListener> localListener = nullptr;
    if (listener != nullptr) {
        localListener = new ConsumerBufferAvailableListener(bufferQueueConsumer, listener, userData);
    }
    auto ret = bufferQueueConsumer->consumer_->SetBufferAvailableListener(localListener);
    FALSE_RETURN_V(ret == Status::OK, OH_AVErrCode::AV_ERR_INVALID_VAL);

    return OH_AVErrCode::AV_ERR_OK;
}