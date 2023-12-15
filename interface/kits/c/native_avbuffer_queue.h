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

#ifndef NATIVE_AVBUFFER_QUEUE_H
#define NATIVE_AVBUFFER_QUEUE_H

#include "native_averrors.h"
#include "native_avbuffer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OH_AVBufferQueueConsumer OH_AVBufferQueueConsumer;
typedef struct OH_AVBufferQueueProducer OH_AVBufferQueueProducer;

typedef void (*OH_AVBufferQueueProducer_OnBufferFilled)(
        OH_AVBufferQueueProducer* bufferQueue, OH_AVBuffer* buffer, void* userData);

typedef void (*OH_AVBufferQueueConsumer_OnBufferAvailable)(
        OH_AVBufferQueueConsumer* bufferQueue, void* userData);

/**
 * @brief Create an OH_AVBufferQueueConsumer instance.
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param name Name for tracing.
 * @param maxBufferCount Max buffer count of queue.
 * @return Return a pointer to an OH_AVBufferQueueConsumer instance,
 *         which need to be destroyed by OH_AVBufferQueueConsumer_Destroy after using.
 * @since 10
 */
OH_AVBufferQueueConsumer* OH_AVBufferQueueConsumer_Create(const char* name, uint32_t maxBufferCount);

/**
 * @brief Destroy an OH_AVBufferQueueConsumer instance.
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param bufferQueue An OH_AVBufferQueueConsumer instance created by OH_AVBufferQueueConsumer_Create.
 * @return AV_ERR_OK if success, otherwise return error code.
 * @since 10
 */
OH_AVErrCode OH_AVBufferQueueConsumer_Destroy(OH_AVBufferQueueConsumer* bufferQueue);

/**
 * @brief Create an OH_AVBufferQueueProducer instance.
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param bufferQueue An OH_AVBufferQueueConsumer instance created by OH_AVBufferQueueConsumer_Create.
 * @return Return a pointer to an OH_AVBufferQueueProducer instance,
 *         needs to be destroyed by OH_AVBufferQueueProducer_Destroy after using.
 * @since 10
 */
OH_AVBufferQueueProducer* OH_AVBufferQueueProducer_CreateFromConsumer(OH_AVBufferQueueConsumer* bufferQueue);

/**
 * @brief Destroy an AVBufferQueueProducer instance.
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param bufferQueue An OH_AVBufferQueueProducer instance created by OH_AVBufferQueueProducer_CreateFromConsumer.
 * @return AV_ERR_OK if success, otherwise return error code.
 * @since 10
 */
OH_AVErrCode OH_AVBufferQueueProducer_Destroy(OH_AVBufferQueueProducer* bufferQueue);

/**
 * @brief Request a free buffer from buffer queue.
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param bufferQueue An OH_AVBufferQueueProducer instance created by OH_AVBufferQueueProducer_CreateFromConsumer.
 * @param buffer Point of output free buffer.
 * @param bufferSize The size of request buffer.
 * @param timeoutMs Timeout time in ms.
 * @return AV_ERR_OK if success, otherwise return error code.
 * @since 10
 */
OH_AVErrCode OH_AVBufferQueueProducer_RequestBuffer(
        OH_AVBufferQueueProducer* bufferQueue, OH_AVBuffer** buffer, int32_t bufferSize, int32_t timeoutMs);

/**
 * @brief Push a buffer to buffer queue, which can modified by buffer filled listener.
 *         If buffer is available and buffer filled listener was not registered,
 *         it will be returned to buffer queue immediately.
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param bufferQueue input an OH_AVBufferQueueProducer instance created by OH_AVBufferQueueProducer_CreateFromConsumer.
 * @param buffer The buffer requested earlier.
 * @param available If the buffer was written or not.
 * @return AV_ERR_OK if success, otherwise return error code.
 * @since 10
 */
OH_AVErrCode OH_AVBufferQueueProducer_PushBuffer(
        OH_AVBufferQueueProducer* bufferQueue, const OH_AVBuffer* buffer, bool available);

/**
 * @brief Return a buffer to buffer queue immediately, which cannot be modified,
 *         if the buffer is available, it will be consumed.
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param bufferQueue An OH_AVBufferQueueProducer instance created by OH_AVBufferQueueProducer_CreateFromConsumer.
 * @param buffer The buffer requested earlier.
 * @param available If the buffer was written or not.
 * @return AV_ERR_OK if success, otherwise return error code.
 * @since 10
 */
OH_AVErrCode OH_AVBufferQueueProducer_ReturnBuffer(
        OH_AVBufferQueueProducer* bufferQueue, const OH_AVBuffer* buffer, bool available);

/**
 * @brief Set buffer filled listener, when a buffer was produced, the listener will be called,
 *         and you can holed and modify it. After that, you return it to the buffer queue.
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param bufferQueue An OH_AVBufferQueueProducer instance created by OH_AVBufferQueueProducer_CreateFromConsumer.
 * @param listener The listener of buffer was filled by producer.
 *         If it was null point, the listener will be unregistered.
 * @param userData Which will be transported to user when listener was called.
 * @return AV_ERR_OK if success, otherwise return error code.
 * @since 10
 */
OH_AVErrCode OH_AVBufferQueueProducer_SetBufferFilledListener(
        OH_AVBufferQueueProducer* bufferQueue, OH_AVBufferQueueProducer_OnBufferFilled* listener, void* userData);

/**
 * @brief Acquire an available buffer from queue. Recommend to call it after
 *         consumer buffer available listener was triggered.
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param bufferQueue An OH_AVBufferQueueConsumer instance created by OH_AVBufferQueueConsumer_Create.
 * @param buffer The output buffer point, which is filled with available data by producer.
 * @return AV_ERR_OK if success, otherwise return error code.
 * @since 10
 */
OH_AVErrCode OH_AVBufferQueueConsumer_AcquireBuffer(OH_AVBufferQueueConsumer* bufferQueue, OH_AVBuffer** buffer);

/**
 * @brief Release an available buffer to queue when the data was consumed.
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param bufferQueue An OH_AVBufferQueueConsumer instance created by OH_AVBufferQueueConsumer_Create.
 * @param buffer The buffer, which data was consumed.
 * @return AV_ERR_OK if success, otherwise return error code.
 * @since 10
 */
OH_AVErrCode OH_AVBufferQueueConsumer_ReleaseBuffer(OH_AVBufferQueueConsumer* bufferQueue, const OH_AVBuffer* buffer);

/**
 * @brief Set the listener of there is an available buffer for consuming.
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param bufferQueue An OH_AVBufferQueueConsumer instance created by OH_AVBufferQueueConsumer_Create.
 * @param listener The listener. If it was null point, the listener will be unregistered.
 * @return AV_ERR_OK if success, otherwise return error code.
 * @since 10
 */
OH_AVErrCode OH_AVBufferQueueConsumer_SetBufferAvailableListener(
        OH_AVBufferQueueConsumer* bufferQueue, OH_AVBufferQueueConsumer_OnBufferAvailable* listener, void* userData);

#ifdef __cplusplus
}
#endif

#endif // NATIVE_AVBUFFER_QUEUE_H
