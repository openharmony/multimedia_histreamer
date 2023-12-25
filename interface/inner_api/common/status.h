/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef STATUS_H
#define STATUS_H

#include <cstdint> // NOLINT: using int32_t in this file

namespace OHOS {
namespace Media {
// 需要参考OH的错误码编码规范来写，模块+错误码,常见具体错误是有固定定义
// 后续再定义北向接口 MfErrCode，那么它应该是Status的子集。
/**
* @enum Api Return Status.
*
* @since 1.0
* @version 1.0
*/
enum struct Status : int32_t {
    END_OF_STREAM = 1,         ///< Read source when end of stream
    OK = 0,                    ///< The execution result is correct.
    NO_ERROR = OK,             ///< Same as Status::OK
    ERROR_UNKNOWN = -1,        ///< An unknown error occurred.
    ERROR_PLUGIN_ALREADY_EXISTS = -2, ///< The plugin already exists, usually occurs when in plugin registered.
    ERROR_INCOMPATIBLE_VERSION =
        -3, ///< Incompatible version, may occur during plugin registration or function calling.
    ERROR_NO_MEMORY = -4,           ///< The system memory is insufficient.
    ERROR_WRONG_STATE = -5,         ///< The function is called in an invalid state.
    ERROR_UNIMPLEMENTED = -6,       ///< This method or interface is not implemented.
    ERROR_INVALID_PARAMETER = -7,   ///< The plugin does not support this parameter.
    ERROR_INVALID_DATA = -8,        ///< The value is not in the valid range.
    ERROR_MISMATCHED_TYPE = -9,     ///< Mismatched data type
    ERROR_TIMED_OUT = -10,          ///< Operation timeout.
    ERROR_UNSUPPORTED_FORMAT = -11, ///< The plugin not support this format/name.
    ERROR_NOT_ENOUGH_DATA = -12,    ///< Not enough data when read from source.
    ERROR_NOT_EXISTED = -13,        ///< Source is not existed.
    ERROR_AGAIN = -14,              ///< Operation is not available right now, should try again later.
    ERROR_PERMISSION_DENIED = -15,  ///< Permission denied.
    ERROR_NULL_POINTER = -16,       ///< Null pointer.
    ERROR_INVALID_OPERATION = -17,  ///< Invalid operation.
    ERROR_CLIENT = -18,             ///< Http client error
    ERROR_SERVER = -19,             ///< Http server error
    ERROR_DELAY_READY = -20,        ///< Delay ready event
    ERROR_INVALID_STATE = -21,
    ERROR_AUDIO_INTERRUPT = -22,
    ERROR_INVALID_BUFFER_SIZE = 0xF001,
    ERROR_UNEXPECTED_MEMORY_TYPE = 0xF002,
    ERROR_CREATE_BUFFER = 0xF003,
    ERROR_NULL_POINT_BUFFER = 0xF004,
    ERROR_INVALID_BUFFER_ID = 0xF005,
    ERROR_INVALID_BUFFER_STATE = 0xF006,
    ERROR_NO_FREE_BUFFER = 0xF007,
    ERROR_NO_DIRTY_BUFFER = 0xF008,
    ERROR_NO_CONSUMER_LISTENER = 0xF009,
    ERROR_NULL_BUFFER_QUEUE = 0xF00A,
    ERROR_WAIT_TIMEOUT = 0xF00B,
    ERROR_OUT_OF_RANGE = 0xF00C,
    ERROR_NULL_SURFACE = 0xF00D,
    ERROR_SURFACE_INNER = 0xF00E,
    ERROR_NULL_SURFACE_BUFFER = 0xF00F,

    ERROR_IPC_WRITE_INTERFACE_TOKEN = 0xF101,
    ERROR_IPC_SEND_REQUEST = 0xF102,
};
} // namespace Media
} // namespace OHOS
#endif // STATUS_H
