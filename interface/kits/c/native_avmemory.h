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

#ifndef NATIVE_AVMEMORY_H
#define NATIVE_AVMEMORY_H

#include <stdint.h>
#include "native_averrors.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OH_AVMemory OH_AVMemory;

/**
 * @brief Create an OH_AVMemory instance
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param size the memory's size, bytes.
 * @return Returns a pointer to an OH_AVMemory instance, needs to be freed by OH_AVMemory_Destroy.
 * @deprecated since 11
 * @useinstead OH_AVBuffer_Create
 * @since 10
 */
OH_AVMemory *OH_AVMemory_Create(int32_t size);

/**
 * @brief Get the memory's virtual address
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param mem Encapsulate OH_AVMemory structure instance pointer
 * @return the memory's virtual address if the memory is valid, otherwise nullptr.
 * @deprecated since 11
 * @useinstead OH_AVBuffer_GetAddr
 * @since 9
 * @version 1.0
 */
uint8_t *OH_AVMemory_GetAddr(struct OH_AVMemory *mem);

/**
 * @brief Get the memory's size
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param mem Encapsulate OH_AVMemory structure instance pointer
 * @return the memory's size if the memory is valid, otherwise -1.
 * @deprecated since 11
 * @useinstead OH_AVBuffer_GetCapacity
 * @since 9
 * @version 1.0
 */
int32_t OH_AVMemory_GetSize(struct OH_AVMemory *mem);

/**
 * @brief Clear the internal resources of the memory and destroy the memory
 * instance
 * @syscap SystemCapability.Multimedia.Media.Core
 * @param mem Encapsulate OH_AVMemory structure instance pointer
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @deprecated since 11
 * @useinstead OH_AVBuffer_Destroy
 * @since 10
 */
OH_AVErrCode OH_AVMemory_Destroy(struct OH_AVMemory *mem);

#ifdef __cplusplus
}
#endif

#endif // NATIVE_AVMEMORY_H
