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

#ifndef HISTREAMER_FOUNDATION_ERROR_CODE_H
#define HISTREAMER_FOUNDATION_ERROR_CODE_H

namespace OHOS {
namespace Media {
enum struct ErrorCode : int32_t {
    END_OF_STREAM = 1,
    SUCCESS = 0,
    ERROR_UNKNOWN = -1,
    ERROR_UNIMPLEMENTED = -2,
    ERROR_NOT_FOUND = -3,
    ERROR_INVALID_PARAM_TYPE = -4,
    ERROR_INVALID_PARAM_VALUE = -5,
    ERROR_INVALID_OPERATION = -6,
    ERROR_INVALID_SOURCE = -7,
    ERROR_INVALID_STREAM_INDEX = -8,
    ERROR_INITIALIZATION_FAILURE = -9,
    ERROR_NULL_POINTER = -10,
    ERROR_ACTIVATE = -11,
    ERROR_NEGOTIATE_FAILED = -12,
    ERROR_LOAD_URI_FAILED = -13,
    ERROR_PLUGIN_NOT_FOUND = -14,
    ERROR_PORT_UNEXPECTED = -15,
    ERROR_STATE = -16,
    ERROR_PARSE_META_FAILED = -17,
    ERROR_SEEK_FAILURE = -18,
    ERROR_ALREADY_EXISTS = -19,
    ERROR_TIMEOUT = -20
};
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_FOUNDATION_ERROR_CODE_H
