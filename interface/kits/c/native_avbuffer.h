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

#ifndef NATIVE_AVBUFFER_H
#define NATIVE_AVBUFFER_H

#include <stdint.h>
#include <stdio.h>
#include "native_avcodec_base.h"
#include "native_averrors.h"
#include "native_avformat.h"
#include "native_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct OH_AVBuffer OH_AVBuffer;
// typedef struct OH_AVBufferQueue OH_AVBufferQueue;

/**
 * @brief Create an OH_AVBuffer instance
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param capacity the buffer's capacity, bytes.
 * @return Returns a pointer to an OH_AVBuffer instance, needs to be freed by OH_AVBuffer_Destroy.
 * @since 10
 */
OH_AVBuffer *OH_AVBuffer_Create(int32_t capacity);

// /**
//  * @brief Create an OH_AVBuffer instance
//  * @syscap SystemCapability.Multimedia.Media.Core
//  * @param bufferQueue Encapsulate OH_AVBufferQueue structure instance pointer
//  * @return Returns a pointer to an OH_AVBuffer instance, needs to be freed by OH_AVBuffer_Destroy.
//  * @since 10
//  */
// OH_AVBuffer *OH_AVBuffer_CreateFromBufferQueue(OH_AVBufferQueue *bufferQueue);

/**
 * @brief Get the buffer's virtual address
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param buf Encapsulate OH_AVBuffer structure instance pointer
 * @return the buffer's virtual address if the buffer is valid, otherwise nullptr.
 * @since 10
 * @version 1.0
 */
uint8_t *OH_AVBuffer_GetAddr(OH_AVBuffer *buf);

/**
 * @brief Get the buffer's size
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param buf Encapsulate OH_AVBuffer structure instance pointer
 * @return the buffer's capacity if the buffer is valid, otherwise 0, bytes.
 * @since 10
 * @version 1.0
 */
int32_t OH_AVBuffer_GetCapacity(OH_AVBuffer *buf);

/**
 * @brief Get the buffer's flag
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param buf Encapsulate OH_AVBuffer structure instance pointer
 * @return Returns The description of the Buffer, please refer to {@link OH_AVCodecBufferAttr}
 * @since 10
 * @version 1.0
 */
// OH_AVCodecBufferAttr OH_AVBuffer_GetBufferAttr(OH_AVBuffer *buf);

/**
 * @brief Set the buffer's flag
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param buf Encapsulate OH_AVBuffer structure instance pointer
 * @param attr The description of the Buffer, please refer to {@link OH_AVCodecBufferAttr}
 * @return Returns Status::OK if the execution is successful, otherwise returns a specific error code, refer to {@link
 * OH_AVErrCode}
 * @since 10
 * @version 1.0
 */
// OH_AVErrCode OH_AVBuffer_SetBufferAttr(OH_AVBuffer *buf, OH_AVCodecBufferAttr *attr);

/**
 * @brief Get the buffer's meta data
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param buf Encapsulate OH_AVBuffer structure instance pointer
 * @return Returns Encapsulate OH_AVFormat structure instance pointer, refer to {@link OH_AVFormat}
 * @since 10
 * @version 1.0
 */
OH_AVFormat *OH_AVBuffer_GetParameter(OH_AVBuffer *buf);

/**
 * @brief Set the buffer's meta data
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param buf Encapsulate OH_AVBuffer structure instance pointer
 * @param format Encapsulate OH_AVFormat structure instance pointer, refer to {@link OH_AVFormat}
 * @return Returns Status::OK if the execution is successful, otherwise returns a specific error code, refer to {@link
 * OH_AVErrCode}
 * @since 10
 * @version 1.0
 */
OH_AVErrCode OH_AVBuffer_SetParameter(OH_AVBuffer *buf, OH_AVFormat *format);

/**
 * @brief Get the nativeBuffer
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param buf Encapsulate OH_AVBuffer structure instance pointer
 * @return Returns Encapsulate OH_NativeBuffer structure instance pointer is successful, otherwise returns nullptr,
 * refer to {@link OH_NativeBuffer}
 * @since 10
 * @version 1.0
 */
OH_NativeBuffer *OH_AVBuffer_GetNativeBuffer(OH_AVBuffer *buf);

/**
 * @brief Clear the internal resources of the buffer and destroy the buffer instance
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param mem Encapsulate OH_AVBuffer structure instance pointer
 * @return Returns Status::OK if the execution is successful, otherwise returns a specific error code, refer to {@link
 * OH_AVErrCode}
 * @since 10
 * @version 1.0
 */
OH_AVErrCode OH_AVBuffer_Destroy(struct OH_AVBuffer *buf);

// /**
//  * @brief Get the nativeBuffer
//  * @syscap SystemCapability.Multimedia.Media.Core
//  * @param bufferQueue Encapsulate OH_AVBufferQueue structure instance pointer
//  * @param callback  A collection of all callback functions, see {@link OH_AVBufferQueueCallback}
//  * @param userData specified data
//  * @return Returns Status::OK if the execution is successful, otherwise returns a specific error code, refer to
//  {@link
//  * OH_AVErrCode}
//  * @since 10
//  * @version 1.0
//  */
// OH_AVErrCode OH_AVBufferQueue_SetProducerCallback(OH_AVBufferQueue *bufferQueue, OH_AVBufferQueueCallback callback,
//                                                   void *userData);

// /**
//  * @brief Request the buffer from buffer queue.
//  * @syscap SystemCapability.Multimedia.Media.Core
//  * @param bufferQueue Encapsulate OH_AVBufferQueue structure instance pointer
//  * @param capacity the buffer's capacity, bytes.
//  * @return Returns a pointer to an OH_AVBuffer instance, needs to be freed by OH_AVBufferQueue_PushBuffer.
//  * @since 10
//  * @version 1.0
//  */
// OH_AVBuffer *OH_AVBufferQueue_RequestBuffer(OH_AVBufferQueue *bufferQueue, int32_t capacity);

// /**
//  * @brief Push and clear the internal resources of the buffer that request from buffer queue.
//  * @syscap SystemCapability.Multimedia.Media.Core
//  * @param bufferQueue Encapsulate OH_AVBufferQueue structure instance pointer
//  * @param buf Encapsulate OH_AVBuffer structure instance pointer
//  * @return Returns Status::OK if the execution is successful, otherwise returns a specific error code, refer to
//  {@link
//  * OH_AVErrCode}
//  * @since 10
//  * @version 1.0
//  */
// OH_AVErrCode OH_AVBufferQueue_PushBuffer(OH_AVBufferQueue *bufferQueue, OH_AVBuffer *buf);

#ifdef __cplusplus
}
#endif

#endif // NATIVE_AVBUFFER_H
