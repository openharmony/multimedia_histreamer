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
#ifndef NATIVE_AVERRORS_H
#define NATIVE_AVERRORS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief AV error code
 * @syscap SystemCapability.Multimedia.Media.Core
 * @since 9
 * @version 1.0
 */
typedef enum OH_AVErrCode {
    /**
     * the operation completed successfully.
     */
    AV_ERR_OK = 0,
    /**
     * no memory.
     */
    AV_ERR_NO_MEMORY = 1,
    /**
     * opertation not be permitted.
     */
    AV_ERR_OPERATE_NOT_PERMIT = 2,
    /**
     * invalid argument.
     */
    AV_ERR_INVALID_VAL = 3,
    /**
     * IO error.
     */
    AV_ERR_IO = 4,
    /**
     * network timeout.
     */
    AV_ERR_TIMEOUT = 5,
    /**
     * unknown error.
     */
    AV_ERR_UNKNOWN = 6,
    /**
     * media service died.
     */
    AV_ERR_SERVICE_DIED = 7,
    /**
     * the state is not support this operation.
     */
    AV_ERR_INVALID_STATE = 8,
    /**
     * unsupport interface.
     */
    AV_ERR_UNSUPPORT = 9,
    /**
     * extend err start.
     */
    AV_ERR_EXTEND_START = 100,
} OH_AVErrCode;

#ifdef __cplusplus
}
#endif

#endif // NATIVE_AVERRORS_H
