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
enum ErrorCode {
    SUCCESS = 0,
    UNKNOWN_ERROR,
    UNIMPLEMENT,
    BUSY,
    NOT_FOUND,
    INVALID_PARAM_TYPE,
    INVALID_PARAM_VALUE,
    INVALID_OPERATION,
    INVALID_SOURCE,
    INVALID_STREAM_INDEX,
    INITIALIZATION_FAILURE,
    NULL_POINTER_ERROR,
    ACTIVATE_ERROR,
    NEGOTIATE_ERROR,
    LOAD_URI_FAILED,
    END_OF_STREAM,
    PLUGIN_NOT_FOUND,
    PORT_NOT_FOUND,
    PORT_UNEXPECTED,
    ASYNC,
    ERROR_STATE,
    PARSE_META_FAILED,
    SEEK_FAILURE,
    ALREADY_EXISTS,
    ERROR_TIMEOUT
};
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_FOUNDATION_ERROR_CODE_H
