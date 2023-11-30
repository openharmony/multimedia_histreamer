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
#include "native_averrors.h"
#include "native_avformat.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct OH_AVBuffer OH_AVBuffer;

/**
 * @brief Define the Buffer description information of OH_AVBuffer
 * @syscap SystemCapability.Multimedia.Media.Core
 * @since 11
 */
typedef struct OH_AVBufferAttr {
    /* Presentation timestamp of this Buffer in microseconds */
    int64_t pts;
    /* The size of the data contained in the Buffer in bytes */
    int32_t size;
    /* The starting offset of valid data in this Buffer */
    int32_t offset;
    /* The flags this Buffer has, which is also a combination of multiple {@link OH_AVCodecBufferFlags} */
    uint32_t flags;
} OH_AVBufferAttr;

/**
 * @brief Create an OH_AVBuffer instance
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param capacity the buffer's capacity, bytes
 * @return Returns a pointer to an OH_AVBuffer instance, needs to be freed by OH_AVBuffer_Destroy
 * @since 11
 */
OH_AVBuffer *OH_AVBuffer_Create(int32_t capacity);

/**
 * @brief Clear the internal resources of the buffer and destroy the buffer instance
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param buffer Encapsulate OH_AVBuffer structure instance pointer
 * @return Returns AV_ERR_OK if the execution is successful, otherwise returns a specific error code, refer to {@link
 * OH_AVErrCode}
 * @since 11
 */
OH_AVErrCode OH_AVBuffer_Destroy(struct OH_AVBuffer *buffer);

/**
 * @brief Get the buffer's attribute
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param buffer Encapsulate OH_AVBuffer structure instance pointer
 * @return Returns the description of the buffer, please refer to {@link OH_AVBufferAttr}
 * @since 11
 */
OH_AVBufferAttr OH_AVBuffer_GetBufferAttr(OH_AVBuffer *buffer);

/**
 * @brief Set the buffer's attribute
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param buffer Encapsulate OH_AVBuffer structure instance pointer
 * @param attr The description of the buffer, please refer to {@link OH_AVBufferAttr}
 * @return Returns AV_ERR_OK if the execution is successful, otherwise returns a specific error code, refer to {@link
 * OH_AVErrCode}
 * @since 11
 */
OH_AVErrCode OH_AVBuffer_SetBufferAttr(OH_AVBuffer *buffer, OH_AVBufferAttr *attr);

/**
 * @brief Get the buffer's parameter. It should be noted that the life cycle of the OH_AVFormat instance pointed to
 * by the return value * needs to be manually released by the caller.
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param buffer Encapsulate OH_AVBuffer structure instance pointer
 * @return Returns Encapsulate OH_AVFormat structure instance pointer, refer to {@link OH_AVFormat}
 * @since 11
 */
OH_AVFormat *OH_AVBuffer_GetParameter(OH_AVBuffer *buffer);

/**
 * @brief Set the buffer's parameter
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param buffer Encapsulate OH_AVBuffer structure instance pointer
 * @param format Encapsulate OH_AVFormat structure instance pointer, refer to {@link OH_AVFormat}
 * @return Returns AV_ERR_OK if the execution is successful, otherwise returns a specific error code, refer to {@link
 * OH_AVErrCode}
 * @since 11
 */
OH_AVErrCode OH_AVBuffer_SetParameter(OH_AVBuffer *buffer, OH_AVFormat *format);

/**
 * @brief Get the buffer's virtual address
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param buffer Encapsulate OH_AVBuffer structure instance pointer
 * @return the buffer's virtual address if the buffer is valid, otherwise nullptr
 * @since 11
 */
uint8_t *OH_AVBuffer_GetAddr(OH_AVBuffer *buffer);

/**
 * @brief Get the buffer's capacity
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param buffer Encapsulate OH_AVBuffer structure instance pointer
 * @return the buffer's capacity if the buffer is valid, otherwise -1
 * @since 11
 */
int32_t OH_AVBuffer_GetCapacity(OH_AVBuffer *buffer);

/**
 * @brief Get the OH_NativeBuffer instance pointer
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param buffer Encapsulate OH_AVBuffer structure instance pointer
 * @return Returns Encapsulate OH_NativeBuffer structure instance pointer is successful,
 * otherwise returns nullptr, refer to {@link OH_NativeBuffer}
 * @since 11
 */
OH_NativeBuffer *OH_AVBuffer_GetNativeBuffer(OH_AVBuffer *buffer);

#ifdef __cplusplus
}
#endif

#endif // NATIVE_AVBUFFER_H
